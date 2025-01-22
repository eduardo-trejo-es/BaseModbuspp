#include "ModbusTCPClient.h"
#include <iostream>

int main() {
    int membool_1=90,membool_2=900,memInt_1=500,memInt_2=550,memInt_3=700,memInt_4=750;
    int A;
    try {
        // Initialisation du client Modbus
        ModbusTCPClient client("192.168.5.1", 502);
        client.connect();
        std::cout << "Connexion reussie au serveur Modbus\n";

        // Écriture d'un bit (coil) à l'adresse 10
        client.writeCoil(membool_1, true);
        std::cout << "Ecriture réussie : Coil 10 mis a ON.\n";

        // Lecture du bit (coil) à l'adresse 10
        bool coilState = client.readCoil(membool_2);
        std::cout << "Lecture reussie : Coil 10 est " << (coilState ? "ON" : "OFF") << ".\n";

        // Écriture d'un entier 16 bits
        int16_t intToWrite = 12345;
        client.writeInt16(memInt_1, intToWrite);
        std::cout << "Écriture réussie : " << intToWrite << " à l'adresse 10.\n";

        // Lecture d'un entier 32 bits
        int16_t intRead = client.readInt16(memInt_2);
        std::cout << "Lecture réussie : " << intRead << " à l'adresse 10.\n";

        // Écriture d'un flottant 32 bits
        float floatToWrite = 123.45f;
        client.writeFloat32(memInt_3, floatToWrite);
        std::cout << "Écriture réussie : " << floatToWrite << " à l'adresse 20.\n";

        // Lecture d'un flottant 32 bits
        float floatRead = client.readFloat32(memInt_4);
        std::cout << "Lecture réussie : " << floatRead << " à l'adresse 20.\n";

        std::cin >> A;

    } catch (const std::exception& ex) {
        std::cerr << "Erreur : " << ex.what() << "\n";
    }

    return 0;
}
