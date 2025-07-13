#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H

#include <iostream>

// Limpia toda la pantalla y posiciona el cursor arriba a la izquierda
inline void clearScreen() {
    std::cout << "\033[2J\033[H";
}

// Posiciona el cursor en una coordenada específica
inline void gotoxy(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

inline void highlightCell(int row, int col, const std::string& symbol) {
    // Ajusta estas coordenadas si tu tablero tiene encabezado o márgenes
    int screenRow = 6 + row; // por ejemplo, si el tablero empieza en fila 6
    int screenCol = 3 + col * 2; // cada celda toma 2 espacios: " ·"
    gotoxy(screenRow, screenCol);
    std::cout << "\033[7m" << symbol << "\033[0m"; // invertido temporal
}


#endif // CONSOLE_UTILS_H
