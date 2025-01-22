#include "ModbusTCPClient.h"
#include <iostream>

int main() {
    int A;
    try {
        // Initialisation du client Modbus
        ModbusTCPClient client("192.168.5.1", 502);
        client.connect();
        std::cout << "Connexion reussie au serveur Modbus\n";

        // Écriture d'un bit (coil) à l'adresse 10
        client.writeCoil(90, true);
        std::cout << "Ecriture réussie : Coil 10 mis a ON.\n";

        // Lecture du bit (coil) à l'adresse 10
        bool coilState = client.readCoil(900);
        std::cout << "Lecture reussie : Coil 10 est " << (coilState ? "ON" : "OFF") << ".\n";

        std::cin >> A;

    } catch (const std::exception& ex) {
        std::cerr << "Erreur : " << ex.what() << "\n";
    }

    return 0;
}
