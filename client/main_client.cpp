#include "GameLogic.hpp"
#include "GameUI.hpp"
#include "SocketClient.hpp"
#include <iostream>
#include <memory>
#include "UdpDiscoveryClient.hpp"


int main() {
    int port = 5000;
    UdpDiscoveryClient discovery;
    std::cout << "Buscando servidores activos en la red...\n";
    auto servers = discovery.discover();
    if (servers.empty()) {
        std::cout << "No se encontraron servidores activos.\n";
        return 1;
    }
    std::cout << "Servidores encontrados:\n";
    for (size_t i = 0; i < servers.size(); ++i) {
        std::cout << i << ": " << servers[i].hostName << " (" << servers[i].ip << ":" << servers[i].port << ")\n";
    }
    int idx;
    std::cout << "Ingrese el número del servidor al que desea conectarse: ";
    std::cin >> idx;
    if (idx < 0 || idx >= (int)servers.size()) {
        std::cout << "Índice inválido.\n";
        return 1;
    }
    std::string host = servers[idx].ip;
    port = servers[idx].port;

    SocketClient client(host, port);
    try {
        client.connectToServer();
    } catch (...) {
        std::cout << "No se pudo conectar al servidor." << std::endl;
        return 1;
    }

    std::string nombre;
    std::cout << "Tu nombre de jugador: ";
    std::cin >> nombre;
    client.send("JOIN " + nombre);

    auto logic = std::make_shared<GameLogic>(10, 10, 10);
    GameUI ui(logic);
    bool miTurno = false;
    bool partidaIniciada = false;
    bool primerMovimiento = true;
    while (true) {
        // Recibir mensajes del servidor
        std::string msg = client.receive();
        if (msg.empty()) continue;

        // Mostrar tablero inicial al recibir START
        if (msg.rfind("START ", 0) == 0) {
            std::string turno = msg.substr(6);
            partidaIniciada = true;
            miTurno = false;
            system("clear");
            ui.render();
            std::cout << "La partida ha comenzado. Esperando turno..." << std::endl;
            continue;
        }

        // Cambiar turno solo con mensaje TURN
        if (msg.rfind("TURN ", 0) == 0) {
            std::string turno = msg.substr(5);
            miTurno = (turno == nombre);
            system("clear");
            ui.render();
            if (miTurno) {
                std::cout << "¡Es tu turno!" << std::endl;
                std::cout << "fila columna (flag=1, descubrir=0): ";
                int fila, columna, f;
                std::cin >> fila >> columna >> f;
                if (f) {
                    logic->toggleFlag(fila, columna);
                    system("clear");
                    ui.render();
                    std::cout << "fila columna (flag=1, descubrir=0): ";
                    // Permitir banderas múltiples en el mismo turno
                    std::cin >> fila >> columna >> f;
                }
                if (primerMovimiento) {
                    client.send("MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " " + std::to_string(f) + " " + nombre);
                    miTurno = false;
                } else {
                    logic->reveal(fila, columna);
                    system("clear");
                    ui.render();
                    std::cout << "Esperando tu turno..." << std::endl;
                    client.send("MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " " + std::to_string(f) + " " + nombre);
                    miTurno = false;
                }
            } else {
                std::cout << "Turno de: " << turno << std::endl;
                std::cout << "Esperando tu turno..." << std::endl;
            }
            continue;
        }

        // Ya no se usa BOARD, ignorar
        if (msg.rfind("BOARD ", 0) == 0) {
            continue;
        }

        // Ejecutar la jugada recibida del rival
        if (msg.rfind("REVEAL ", 0) == 0) {
            int fila = 0, columna = 0;
            sscanf(msg.c_str() + 7, "%d %d", &fila, &columna);
            logic->reveal(fila, columna);
            system("clear");
            ui.render();
            if (miTurno) {
                std::cout << "fila columna (flag=1, descubrir=0): ";
                int fila2, columna2, f2;
                std::cin >> fila2 >> columna2 >> f2;
                if (f2) {
                    logic->toggleFlag(fila2, columna2);
                    system("clear");
                    ui.render();
                    std::cout << "fila columna (flag=1, descubrir=0): ";
                    std::cin >> fila2 >> columna2 >> f2;
                }
                if (primerMovimiento) {
                    client.send("MOVE " + std::to_string(fila2) + " " + std::to_string(columna2) + " " + std::to_string(f2) + " " + nombre);
                    miTurno = false;
                } else {
                    logic->reveal(fila2, columna2);
                    system("clear");
                    ui.render();
                    std::cout << "Esperando tu turno..." << std::endl;
                    client.send("MOVE " + std::to_string(fila2) + " " + std::to_string(columna2) + " " + std::to_string(f2) + " " + nombre);
                    miTurno = false;
                }
            } else {
                std::cout << "Esperando tu turno..." << std::endl;
            }
            continue;
        }

        if (msg.rfind("WIN ", 0) == 0) {
            std::string ganador = msg.substr(4);
            std::cout << "¡Ganador: " << ganador << "!" << std::endl;
            break;
        }
        if (msg.rfind("LOSE ", 0) == 0) {
            std::string perdedor = msg.substr(5);
            std::cout << "¡Perdedor: " << perdedor << "!" << std::endl;
            break;
        }
        if (msg.rfind("MSG ", 0) == 0) {
            std::cout << msg.substr(4) << std::endl;
            continue;
        }

        // Sincronizar la semilla si el servidor la envía
        if (msg.rfind("SEED ", 0) == 0) {
            unsigned seed = std::stoul(msg.substr(5));
            srand(seed);
            logic = std::make_shared<GameLogic>(10, 10, 10);
            ui = GameUI(logic);
            primerMovimiento = false;
            continue;
        }

        // Si es mi turno y la partida está iniciada, pedir jugada SOLO tras recibir TURN
        if (miTurno && partidaIniciada) {
            int fila, columna, f;
            std::cin >> fila >> columna >> f;
            if (f) {
                logic->toggleFlag(fila, columna);
                system("clear");
                ui.render();
                std::cout << "fila columna (flag=1, descubrir=0): ";
            } else {
                if (primerMovimiento) {
                    client.send("MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " " + std::to_string(f) + " " + nombre);
                    miTurno = false;
                } else {
                    logic->reveal(fila, columna);
                    system("clear");
                    ui.render();
                    std::cout << "Esperando tu turno..." << std::endl;
                    client.send("MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " " + std::to_string(f) + " " + nombre);
                    miTurno = false;
                }
            }
        }
    }
    client.disconnect();
    return 0;
}
