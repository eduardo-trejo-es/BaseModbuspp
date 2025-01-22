#include "ModbusTCPClient.h"
#include <iostream>

int main() {
    try {
        // Initialisation du client Modbus
        ModbusTCPClient client("192.168.5.13", 502);
        client.connect();
        std::cout << "Connexion réussie au serveur Modbus\n";

        // Écriture d'un bit (coil) à l'adresse 10
        client.writeCoil(10, true);
        std::cout << "Écriture réussie : Coil 10 mis à ON.\n";

        // Lecture du bit (coil) à l'adresse 10
        bool coilState = client.readCoil(10);
        std::cout << "Lecture réussie : Coil 10 est " << (coilState ? "ON" : "OFF") << ".\n";

    } catch (const std::exception& ex) {
        std::cerr << "Erreur : " << ex.what() << "\n";
    }

    return 0;
}
