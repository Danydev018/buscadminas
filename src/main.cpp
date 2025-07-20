#include <iostream>
#include <string>


#include "entrypoints.h"
#include "common/Board.h"
#include "common/ConsoleUtils.h"

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

void runSinglePlayer() {
    std::cout << "Iniciando modo jugador individual..." << std::endl;
    int filas = 9, columnas = 9, minas = 10, dificultad = 1;
    std::cout << "Tamaño por defecto: 9x9\n";
    std::cout << "¿Desea personalizar el tablero? (s/n): ";
    char custom;
    std::cin >> custom;
    if (custom == 's' || custom == 'S') {
        std::cout << "Filas (5-20): ";
        while (!(std::cin >> filas) || filas < 5 || filas > 20) {
            std::cout << "Valor inválido. Filas (5-20): ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cout << "Columnas (5-15): ";
        while (!(std::cin >> columnas) || columnas < 5 || columnas > 15) {
            std::cout << "Valor inválido. Columnas (5-15): ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cout << "Seleccione dificultad (1=Fácil 20%, 2=Medio 50%, 3=Difícil 70%): ";
        while (!(std::cin >> dificultad) || dificultad < 1 || dificultad > 3) {
            std::cout << "Valor inválido. Seleccione dificultad (1=Fácil 20%, 2=Medio 50%, 3=Difícil 70%): ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } else {
        std::cout << "Seleccione dificultad (1=Fácil 20%, 2=Medio 50%, 3=Difícil 70%): ";
        while (!(std::cin >> dificultad) || dificultad < 1 || dificultad > 3) {
            std::cout << "Valor inválido. Seleccione dificultad (1=Fácil 20%, 2=Medio 50%, 3=Difícil 70%): ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    double porcentaje = (dificultad == 1) ? 0.2 : (dificultad == 2) ? 0.5 : 0.7;
    int totalCasillas = filas * columnas;
    minas = static_cast<int>(totalCasillas * porcentaje);
    if (minas == 0) minas = 1;
    if (minas >= totalCasillas) minas = totalCasillas - 1;
    unsigned seed = static_cast<unsigned>(time(nullptr));
    Board board(filas, columnas, minas, seed);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    int cursorRow = 0, cursorCol = 0;
    int lastRow = -1, lastCol = -1;
    bool gameOver = false;
    clearScreen();
    gotoxy(1, 1);
    drawFrameAroundBoard(4, 2, board.cols(), board.rows());
    board.drawGotoxy(4, 2);
    while (!gameOver) {
        KeyCode key = getKey();
        if (key == KEY_UP && cursorRow > 0) cursorRow--;
        else if (key == KEY_DOWN && cursorRow < board.rows() - 1) cursorRow++;
        else if (key == KEY_LEFT && cursorCol > 0) cursorCol--;
        else if (key == KEY_RIGHT && cursorCol < board.cols() - 1) cursorCol++;
        else if (key == KEY_FLAG) {
            board.toggleFlag(cursorRow, cursorCol);
        } else if (key == KEY_ENTER) {
            board.reveal(cursorRow, cursorCol);
            if (board.isMine(cursorRow, cursorCol)) {
                showAllMines(board, "Has perdido💣");
                gameOver = true;
                break;
            }
            if (board.allSafeRevealed()) {
                showAllMines(board, "¡Has ganado! 🏁");
                gameOver = true;
                break;
            }
        } else if (key == KEY_QUIT) {
            gotoxy(2, board.rows() + 5);
            std::cout << "🔚 Saliendo del juego..." << std::endl;
            break;
        }
        // Redibuja solo si el cursor cambió o hubo acción
        if (cursorRow != lastRow || cursorCol != lastCol || key == KEY_FLAG || key == KEY_ENTER) {
            lastRow = cursorRow;
            lastCol = cursorCol;
            clearScreen();
            gotoxy(1, 1);
            drawFrameAroundBoard(4, 2, board.cols(), board.rows());
            board.drawGotoxy(4, 2);
            // Solo dibujar highlight si la celda NO está revelada
            if (board.at(cursorRow, cursorCol).state != Revealed) {
                if (board.rows() >= 10) gotoxy(4 + cursorCol * 3, 1 + cursorRow);    
                else gotoxy(4 + cursorCol * 3, 2 + cursorRow); 
                std::cout << "\033[35m◉\033[0m";
            }
            gotoxy(2, board.rows() + 6);
            std::cout << "\033[92m┌─ CONTROLES ─────────────────────────────────────┐\033[0m\n";
            gotoxy(2, board.rows() + 7);
            std::cout << "\033[92m│ \033[97m⬆️⬇️⬅️➡️ Mover cursor  \033[93mR\033[97m Revelar  \033[93mF\033[97m Bandera \033[93mQ\033[97m Salir \033[92m│\033[0m\n";
            gotoxy(2, board.rows() + 8);
            std::cout << "\033[92m└─────────────────────────────────────────────────┘\033[0m\n";
        }
    }
    gotoxy(2, board.rows() + 10);
    std::cout << "Presione Enter para volver al menú principal..." << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int main() {
    while (true) {
        std::cout << "Buscaminas - Unificado" << std::endl;
        std::cout << "Seleccione el modo:\n1. Jugador individual\n2. Multijugador\n3. Salir\nOpción: ";
        int opcion;
        std::cin >> opcion;
        if (opcion == 1) {
            runSinglePlayer();
        } else if (opcion == 2) {
            std::cout << "\nMultijugador:\n1. Host (Servidor)\n2. Join (Cliente)\n3. Volver\nOpción: ";
            int subopcion;
            std::cin >> subopcion;
            if (subopcion == 1) {
                runServer();
            } else if (subopcion == 2) {
                runClient();
            } else if (subopcion == 3) {
                continue;
            } else {
                std::cout << "Opción no válida en multijugador." << std::endl;
            }
        } else if (opcion == 3) {
            std::cout << "¡Hasta luego!" << std::endl;
            break;
        } else {
            std::cout << "Opción no válida." << std::endl;
        }
    }
    return 0;
}
