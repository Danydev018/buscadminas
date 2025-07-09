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

    int ancho = 10, alto = 10, minas = 10;
    auto logic = std::make_shared<GameLogic>(ancho, alto, minas);
    GameUI ui(logic);
    bool miTurno = false;
    bool partidaIniciada = false;
    bool primerMovimiento = true;
    bool juegoTerminado = false;
    while (!juegoTerminado) {
        std::string msg = client.receive();
        if (msg.empty()) continue;

        if (msg.rfind("SEED ", 0) == 0) {
            unsigned seed = 0;
            try {
                seed = std::stoul(msg.substr(5));
            } catch (...) {
                std::cout << "[ERROR] Semilla inválida recibida: " << msg << std::endl;
                continue;
            }
            if (primerMovimiento) {
                srand(seed);
                primerMovimiento = false;
            }
            continue;
        }

        if (msg.rfind("START ", 0) == 0) {
            std::string turno = msg.substr(6);
            partidaIniciada = true;
            miTurno = false;
            ui.render();
            std::cout << "La partida ha comenzado. Esperando turno..." << std::endl;
            continue;
        }

        if (msg.rfind("TURN ", 0) == 0) {
            std::string turno = msg.substr(5);
            miTurno = (turno == nombre);
            if (miTurno) system("clear");
            ui.render();
            if (miTurno) {
                std::cout << "¡Es tu turno!" << std::endl;
                int fila = -1, columna = -1, f = -1;
                while (true) {
                    std::cout << "fila columna (flag=1, descubrir=0): ";
                    std::cin >> fila >> columna >> f;
                    if (std::cin.fail()) {
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                        std::cout << "Entrada inválida. Intenta de nuevo." << std::endl;
                        continue;
                    }
                    if (fila < 0 || fila >= logic->getHeight() || columna < 0 || columna >= logic->getWidth()) {
                        std::cout << "Coordenadas fuera de rango. Intenta de nuevo." << std::endl;
                        continue;
                    }
                    if (f != 0 && f != 1) {
                        std::cout << "Flag debe ser 0 o 1." << std::endl;
                        continue;
                    }
                    break;
                }
                client.send("MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " " + std::to_string(f) + " " + nombre);
                miTurno = false;
            } else {
                std::cout << "Turno de: " << turno << std::endl;
                std::cout << "Esperando tu turno..." << std::endl;
            }
            continue;
        }

        if (msg.rfind("REVEAL ", 0) == 0) {
            int fila = 0, columna = 0;
            if (sscanf(msg.c_str() + 7, "%d %d", &fila, &columna) != 2) {
                std::cout << "[ERROR] Mensaje REVEAL malformado: " << msg << std::endl;
                continue;
            }
            if (fila < 0 || fila >= logic->getHeight() || columna < 0 || columna >= logic->getWidth()) {
                std::cout << "[ERROR] Coordenadas fuera de rango en REVEAL: " << fila << ", " << columna << std::endl;
                continue;
            }
            logic->reveal(fila, columna);
            ui.render();
            std::cout << "Esperando tu turno..." << std::endl;
            continue;
        }

        if (msg.rfind("WIN ", 0) == 0) {
            std::string ganador = msg.substr(4);
            std::cout << "[ESTADO] ¡Ganador: " << ganador << "!" << std::endl;
            juegoTerminado = true;
            continue;
        }
        if (msg.rfind("LOSE ", 0) == 0) {
            std::string perdedor = msg.substr(5);
            std::cout << "[ESTADO] ¡Perdedor: " << perdedor << "!" << std::endl;
            juegoTerminado = true;
            continue;
        }
        if (msg.rfind("MSG ", 0) == 0) {
            std::cout << "[INFO] " << msg.substr(4) << std::endl;
            continue;
        }
        // Ignorar mensajes no reconocidos
    }
    client.disconnect();
    return 0;
}
