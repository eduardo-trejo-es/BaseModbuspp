#ifndef MODBUS_TCP_CLIENT_H
#define MODBUS_TCP_CLIENT_H

#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>


class ModbusTCPClient {
public:
    ModbusTCPClient(const std::string& ip, int port);
    ~ModbusTCPClient();

    void connect();
    std::vector<uint16_t> readRegisters(int address, int numRegisters);
    void writeRegister(int address, uint16_t value);

    void writeCoil(int address, bool value); // Écriture d'un bit (coil)
    bool readCoil(int address); // Lecture d'un bit (coil)


private:
    std::string server_ip;
    int server_port;
    SOCKET sock_fd;
    uint16_t transaction_id;

    std::vector<uint8_t> buildRequest(uint8_t function_code, int address, int value_or_count);
    void sendRequest(const std::vector<uint8_t>& request);
    std::vector<uint8_t> receiveResponse();
    std::vector<uint16_t> parseResponse(const std::vector<uint8_t>& response, int numRegisters);
};

#endif // MODBUS_TCP_CLIENT_H