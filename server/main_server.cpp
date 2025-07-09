#include "GameLogic.hpp"
#include "SocketServer.hpp"
#include "GameUI.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include "UdpDiscoveryServer.hpp"

int main() {
    int port;
    std::cout << "Puerto para el servidor: ";
    std::cin >> port;

    SocketServer server(port);
    server.start();
    std::string nombreHost;
    std::cout << "Tu nombre de jugador (host): ";
    std::cin >> nombreHost;
    UdpDiscoveryServer udpDiscovery(nombreHost, port);
    udpDiscovery.start();
    std::cout << "Servidor Buscaminas iniciado en el puerto " << port << std::endl;

    int ancho = 10, alto = 10, minas = 10;
    auto logic = std::make_shared<GameLogic>(ancho, alto, minas);
    bool primerMovimiento = true;

    // Bucle principal del servidor con turnos y sincronización básica
    std::cout << "Esperando conexiones de clientes... (Ctrl+C para salir)" << std::endl;

    // --- NUEVO: El servidor también es jugador local ---
    std::vector<std::string> jugadores;
    jugadores.push_back(nombreHost);
    int turno = 0;
    bool partidaIniciada = false;
    GameUI ui(logic);

    // --- NUEVO FLUJO DE TURNO Y ENTRADA ---
    bool esperandoInputHost = false;
    bool clienteUnido = false;
    while (true) {
        // Recibe mensajes de todos los clientes conectados
        auto mensajes = server.receiveAll();
        for (const auto& msg : mensajes) {
            // Protocolo simple: JOIN nombre | MOVE x y flag nombre | TURN nombre
            // Manejo de desconexión simple: si un cliente se desconecta, terminar partida
            if (server.getNumClients() < 1 && partidaIniciada) {
                server.broadcast("MSG El cliente se ha desconectado. La partida ha terminado.");
                partidaIniciada = false;
                break;
            }
            if (msg.rfind("JOIN ", 0) == 0) {
                std::string nombre = msg.substr(5);
                // Solo permitir UN cliente además del host
                if (!clienteUnido && nombre != nombreHost) {
                    jugadores.push_back(nombre);
                    clienteUnido = true;
                    std::cout << "Jugador unido: " << nombre << std::endl;
                    server.broadcast("MSG " + nombre + " se ha unido al juego.");
                }
                // Ignorar conexiones adicionales
                if (jugadores.size() == 2 && !partidaIniciada) {
                    partidaIniciada = true;
                    // El turno inicial es siempre del host
                    turno = 0;
                    esperandoInputHost = true;
                    // No mostrar el tablero aquí: solo mostrarlo junto con el input del host
                    server.broadcast("START " + jugadores[turno]);
                    server.broadcast("TURN " + jugadores[turno]);
                }
            } else if (msg.rfind("TURN ", 0) == 0) {
                std::string turnoMsg = msg.substr(5);
                // Sincronizar el índice de turno con el nombre recibido
                for (size_t i = 0; i < jugadores.size(); ++i) {
                    if (jugadores[i] == turnoMsg) {
                        turno = i;
                        break;
                    }
                }
                if (turnoMsg == nombreHost) esperandoInputHost = true;
            } else if (msg.rfind("MOVE ", 0) == 0 && partidaIniciada) {
                // MOVE x y flag nombre
                int fila = 0, columna = 0, flag = 0;
                std::string nombre;
                // Extraer los argumentos de forma segura
                size_t pos1 = msg.find(' ', 5);
                size_t pos2 = msg.find(' ', pos1 + 1);
                size_t pos3 = msg.find(' ', pos2 + 1);
                // Validación robusta de parsing
                if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) {
                    server.broadcast("MSG Movimiento malformado ignorado");
                    continue;
                }
                try {
                    fila = std::stoi(msg.substr(5, pos1 - 5));
                    columna = std::stoi(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    flag = std::stoi(msg.substr(pos2 + 1, pos3 - pos2 - 1));
                    nombre = msg.substr(pos3 + 1);
                } catch (...) {
                    server.broadcast("MSG Movimiento inválido ignorado");
                    continue;
                }
                // Validación de coordenadas usando dimensiones dinámicas
                if (fila < 0 || fila >= logic->getHeight() || columna < 0 || columna >= logic->getWidth()) {
                    server.broadcast("MSG Coordenadas fuera de rango");
                    continue;
                }
                if (jugadores[turno] == nombre && nombre != nombreHost) { // Solo procesar jugadas del cliente
                    if (flag) {
                        logic->toggleFlag(fila, columna);
                        std::cout << "[Cliente] Colocó/quitó bandera en (" << fila << ", " << columna << ")" << std::endl;
                        ui.render();
                    } else {
                        if (primerMovimiento) {
                            unsigned seed = fila * 100 + columna;
                            srand(seed);
                            logic = std::make_shared<GameLogic>(ancho, alto, minas);
                            ui = GameUI(logic);
                            logic->reveal(fila, columna);
                            server.broadcast("SEED " + std::to_string(seed));
                            primerMovimiento = false;
                        } else {
                            logic->reveal(fila, columna);
                        }
                        std::cout << "[Cliente] Descubrió (" << fila << ", " << columna << ")" << std::endl;
                        ui.render();
                        std::string moveMsg = "REVEAL " + std::to_string(fila) + " " + std::to_string(columna);
                        server.broadcast(moveMsg);
                        if (logic->isGameOver()) {
                            server.broadcast("LOSE " + nombre);
                            partidaIniciada = false;
                        } else if (logic->isWin()) {
                            server.broadcast("WIN " + nombre);
                            partidaIniciada = false;
                        } else {
                            turno = (turno + 1) % jugadores.size();
                            server.broadcast("TURN " + jugadores[turno]);
                        }
                    }
                }
            }
        }

        // --- Entrada de movimiento del host SOLO tras recibir TURN ---
        while (partidaIniciada && jugadores.size() == 2 && jugadores[turno] == nombreHost && esperandoInputHost) {
            ui.render();
            std::cout << "fila columna (flag=1, descubrir=0): ";
            int fila, columna, f;
            std::cin >> fila >> columna >> f;
            if (fila < 0 || fila >= logic->getHeight() || columna < 0 || columna >= logic->getWidth()) {
                std::cout << "Coordenadas fuera de rango. Intenta de nuevo." << std::endl;
                continue;
            }
            if (f) {
                logic->toggleFlag(fila, columna);
                ui.render();
                continue;
            } else {
                if (primerMovimiento) {
                    unsigned seed = fila * 100 + columna;
                    srand(seed);
                    logic = std::make_shared<GameLogic>(ancho, alto, minas);
                    ui = GameUI(logic);
                    logic->reveal(fila, columna);
                    ui.render();
                    server.broadcast("SEED " + std::to_string(seed));
                    std::string moveMsg = "REVEAL " + std::to_string(fila) + " " + std::to_string(columna);
                    server.broadcast(moveMsg);
                    primerMovimiento = false;
                    turno = (turno + 1) % jugadores.size();
                    server.broadcast("TURN " + jugadores[turno]);
                } else {
                    logic->reveal(fila, columna);
                    ui.render();
                    std::string moveMsg = "REVEAL " + std::to_string(fila) + " " + std::to_string(columna);
                    server.broadcast(moveMsg);
                    if (logic->isGameOver()) {
                        server.broadcast("LOSE " + nombreHost);
                        partidaIniciada = false;
                    } else if (logic->isWin()) {
                        server.broadcast("WIN " + nombreHost);
                        partidaIniciada = false;
                    } else {
                        turno = (turno + 1) % jugadores.size();
                        server.broadcast("TURN " + jugadores[turno]);
                    }
                }
                esperandoInputHost = false;
                break; // Sal del ciclo de input tras un movimiento válido
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    udpDiscovery.stop();
    server.stop();
    return 0;
}
