
#include "server/Server.h"
#include "entrypoints.h"


void main_server() {
    int filas, columnas, dificultad;
    std::cout << "Ingrese cantidad de filas (5-20): ";
    while (!(std::cin >> filas) || filas < 5 || filas > 20) {
        std::cout << "\033[31mValor inválido.\033[0m Ingrese cantidad de filas (5-20): ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Ingrese cantidad de columnas (5-15): ";
    while (!(std::cin >> columnas) || columnas < 5 || columnas > 15) {
        std::cout << "\033[31mValor inválido.\033[0m Ingrese cantidad de columnas (5-15): ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Seleccione dificultad | 1. Fácil | 2. Medio | 3. Difícil |: ";
    while (!(std::cin >> dificultad) || dificultad < 1 || dificultad > 3) {
        std::cout << "\033[31mValor inválido.\033[0m Seleccione dificultad | 1. Fácil | 2. Medio | 3. Difícil |: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    double porcentaje = (dificultad == 1) ? 0.2 : (dificultad == 2) ? 0.5 : 0.7;
    int totalCasillas = filas * columnas;
    int minas = static_cast<int>(totalCasillas * porcentaje);
    if (minas == 0) minas = 1;
    if (minas >= totalCasillas) minas = totalCasillas - 1;

    Server srv("Sala 1", filas, columnas, minas, dificultad);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    srv.run();
    // No return, función void
}
