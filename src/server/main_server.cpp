
#include "server/Server.h"
#include "entrypoints.h"


void main_server() {
    int filas, columnas, dificultad;
    std::cout << "Ingrese cantidad de filas (5-20): ";
    while (!(std::cin >> filas) || filas < 5 || filas > 20) {
        std::cout << "Valor inválido. Ingrese cantidad de filas (5-20): ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Ingrese cantidad de columnas (5-15): ";
    while (!(std::cin >> columnas) || columnas < 5 || columnas > 15) {
        std::cout << "Valor inválido. Ingrese cantidad de columnas (5-15): ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Seleccione dificultad (1=Fácil 20%, 2=Medio 50%, 3=Difícil 70%): ";
    while (!(std::cin >> dificultad) || dificultad < 1 || dificultad > 3) {
        std::cout << "Valor inválido. Seleccione dificultad (1=Fácil 20%, 2=Medio 50%, 3=Difícil 70%): ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    double porcentaje = (dificultad == 1) ? 0.2 : (dificultad == 2) ? 0.5 : 0.7;
    int totalCasillas = filas * columnas;
    int minas = static_cast<int>(totalCasillas * porcentaje);
    if (minas == 0) minas = 1;
    if (minas >= totalCasillas) minas = totalCasillas - 1;

    Server srv("Sala1", filas, columnas, minas);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    srv.run();
    // No return, función void
}
