#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ModbusTCPClient.h"
#include <iostream>
#include <stdexcept>

int inet_pton(int af, const char* src, void* dst) {
    if (af == AF_INET) {
        unsigned long result = inet_addr(src);
        if (result == INADDR_NONE) return 0;
        ((struct in_addr*)dst)->s_addr = result;
        return 1;
    }
    return -1;
}

ModbusTCPClient::ModbusTCPClient(const std::string& ip, int port)
    : server_ip(ip), server_port(port), sock_fd(INVALID_SOCKET), transaction_id(0) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("Erreur lors de l'initialisation de Winsock.");
    }
}

ModbusTCPClient::~ModbusTCPClient() {
    if (sock_fd != INVALID_SOCKET) {
        closesocket(sock_fd);
    }
    WSACleanup();
}

void ModbusTCPClient::connect() {
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == INVALID_SOCKET) {
        throw std::runtime_error("Erreur lors de la création du socket.");
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        throw std::runtime_error("Adresse IP invalide.");
    }

    if (::connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        throw std::runtime_error("Erreur de connexion au serveur.");
    }
}

std::vector<uint16_t> ModbusTCPClient::readRegisters(int address, int numRegisters) {
    std::vector<uint8_t> request = buildRequest(0x03, address, numRegisters);
    sendRequest(request);
    return parseResponse(receiveResponse(), numRegisters);
}

void ModbusTCPClient::writeCoil(int address, bool value) {
    std::vector<uint8_t> request(12);

    // MBAP Header
    request[0] = transaction_id >> 8;    // Transaction ID High
    request[1] = transaction_id & 0xFF; // Transaction ID Low
    request[2] = 0x00;                  // Protocol ID High
    request[3] = 0x00;                  // Protocol ID Low
    request[4] = 0x00;                  // Length High
    request[5] = 0x06;                  // Length Low (6 bytes following)
    request[6] = 0x01;                  // Unit ID

    // Modbus PDU
    request[7] = 0x05;                  // Function Code (Write Single Coil)
    request[8] = address >> 8;          // Address High
    request[9] = address & 0xFF;        // Address Low
    request[10] = value ? 0xFF : 0x00;  // Value High (0xFF for ON, 0x00 for OFF)
    request[11] = 0x00;                 // Value Low (Always 0)

    transaction_id++;
    sendRequest(request);
    receiveResponse(); // Vérification des erreurs
}

bool ModbusTCPClient::readCoil(int address) {
    std::vector<uint8_t> request(12);

    // MBAP Header
    request[0] = transaction_id >> 8;    // Transaction ID High
    request[1] = transaction_id & 0xFF; // Transaction ID Low
    request[2] = 0x00;                  // Protocol ID High
    request[3] = 0x00;                  // Protocol ID Low
    request[4] = 0x00;                  // Length High
    request[5] = 0x06;                  // Length Low (6 bytes following)
    request[6] = 0x01;                  // Unit ID

    // Modbus PDU
    request[7] = 0x01;                  // Function Code (Read Coils)
    request[8] = address >> 8;          // Address High
    request[9] = address & 0xFF;        // Address Low
    request[10] = 0x00;                 // Number of Coils High
    request[11] = 0x01;                 // Number of Coils Low (Read 1 bit)

    transaction_id++;
    sendRequest(request);

    auto response = receiveResponse();
    if (response.size() < 10 || response[7] != 0x01) {
        throw std::runtime_error("Réponse Modbus invalide pour la lecture de coil.");
    }

    return response[9] & 0x01; // Retourne le premier bit
}


void ModbusTCPClient::writeRegisters(int address, const std::vector<uint16_t>& values) {
    int numRegisters = values.size();
    std::vector<uint8_t> request(13 + numRegisters * 2);

    request[0] = transaction_id >> 8;
    request[1] = transaction_id & 0xFF;
    request[2] = 0x00;
    request[3] = 0x00;
    request[4] = (7 + numRegisters * 2) >> 8;
    request[5] = (7 + numRegisters * 2) & 0xFF;
    request[6] = 0x01;

    request[7] = 0x10;
    request[8] = address >> 8;
    request[9] = address & 0xFF;
    request[10] = numRegisters >> 8;
    request[11] = numRegisters & 0xFF;
    request[12] = numRegisters * 2;

    for (int i = 0; i < numRegisters; ++i) {
        request[13 + i * 2] = values[i] >> 8;
        request[14 + i * 2] = values[i] & 0xFF;
    }

    transaction_id++;
    sendRequest(request);
    receiveResponse();
}

void ModbusTCPClient::writeInt16(int address, int16_t value) {
    uint16_t data = static_cast<uint16_t>(value);
    writeRegisters(address, {data});
}



int16_t ModbusTCPClient::readInt16(int address) {
    auto registers = readRegisters(address, 1);
    return static_cast<int16_t>(registers[0]);
}

void ModbusTCPClient::writeFloat32(int address, float value) {
    uint32_t intValue = *reinterpret_cast<uint32_t*>(&value);
    uint16_t data[2];
    data[0] = static_cast<uint16_t>(intValue >> 16); // Parte alta
    data[1] = static_cast<uint16_t>(intValue & 0xFFFF); // Parte baja

    // Verifica si se necesita cambiar el orden de los bytes
    bool swapEndian = true; // Cambiar a 'false' si no es necesario
    if (swapEndian) {
        std::swap(data[0], data[1]);
    }

    writeRegisters(address, {data[0], data[1]});
}


float ModbusTCPClient::readFloat32(int address) {
    auto registers = readRegisters(address, 2);
    uint16_t data[2] = {registers[0], registers[1]};

    // Verifica si se necesita cambiar el orden de los bytes
    bool swapEndian = true; // Cambiar a 'false' si no es necesario
    if (swapEndian) {
        std::swap(data[0], data[1]);
    }

    uint32_t intValue = (static_cast<uint32_t>(data[0]) << 16) | data[1];
    return *reinterpret_cast<float*>(&intValue);
}


std::vector<uint8_t> ModbusTCPClient::buildRequest(uint8_t function_code, int address, int value_or_count) {
    std::vector<uint8_t> request(12);

    request[0] = transaction_id >> 8;
    request[1] = transaction_id & 0xFF;
    request[2] = 0x00;
    request[3] = 0x00;
    request[4] = 0x00;
    request[5] = 0x06;
    request[6] = 0x01;

    request[7] = function_code;
    request[8] = address >> 8;
    request[9] = address & 0xFF;

    if (function_code == 0x03 || function_code == 0x10) {
        request[10] = value_or_count >> 8;
        request[11] = value_or_count & 0xFF;
    }

    transaction_id++;
    return request;
}

void ModbusTCPClient::sendRequest(const std::vector<uint8_t>& request) {
    if (send(sock_fd, reinterpret_cast<const char*>(request.data()), request.size(), 0) == SOCKET_ERROR) {
        throw std::runtime_error("Erreur lors de l'envoi de la requête.");
    }
}

std::vector<uint8_t> ModbusTCPClient::receiveResponse() {
    std::vector<uint8_t> response(256);
    int bytes_received = recv(sock_fd, reinterpret_cast<char*>(response.data()), response.size(), 0);
    if (bytes_received == SOCKET_ERROR) {
        throw std::runtime_error("Erreur lors de la réception de la réponse.");
    }
    response.resize(bytes_received);
    return response;
}

std::vector<uint16_t> ModbusTCPClient::parseResponse(const std::vector<uint8_t>& response, int numRegisters) {
    if (response.size() < 9 || response[7] != 0x03) {
        throw std::runtime_error("Réponse Modbus invalide.");
    }

    int byteCount = response[8];
    if (byteCount != numRegisters * 2) {
        throw std::runtime_error("Nombre d'octets inattendu dans la réponse.");
    }

    std::vector<uint16_t> registers;
    for (int i = 0; i < numRegisters; ++i) {
        uint16_t value = (response[9 + i * 2] << 8) | response[10 + i * 2];
        registers.push_back(value);
    }
    return registers;
}