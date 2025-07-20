#include <iostream>
#include <string>

#include "entrypoints.h"

// Prototipos directos para asegurar visibilidad
void main_server();
void main_client();

void runServer() {
    std::cout << "Iniciando en modo servidor..." << std::endl;
    main_server();
}

void runClient() {
    std::cout << "Iniciando en modo cliente..." << std::endl;
    main_client();
}

int main() {
    std::cout << "Buscaminas - Unificado (Servidor/Cliente)" << std::endl;
    std::cout << "Seleccione el modo:\n1. Host (Servidor)\n2. Join (Cliente)\nOpción: ";
    int opcion;
    std::cin >> opcion;
    if (opcion == 1) {
        runServer();
    } else if (opcion == 2) {
        runClient();
    } else {
        std::cout << "Opción no válida." << std::endl;
    }
    return 0;
}
