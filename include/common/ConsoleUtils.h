#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H

#include <iostream>
#include "common/Board.h"

enum KeyCode {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_ENTER, KEY_ESC, KEY_FLAG,
    KEY_QUIT,
    KEY_NONE
};

KeyCode getKey();


// Limpia toda la pantalla y posiciona el cursor arriba a la izquierda
inline void clearScreen() {
    std::cout << "\033[2J\033[H";
}

// Posiciona el cursor en una coordenada específica
// En Board.cpp (arriba del archivo o justo antes de drawGotoxy)
void gotoxy(int x, int y);
void drawFrameAroundBoard(int startX, int startY, int width, int height);
void updateBoardDisplay(int startX, int startY, const Board& board);

inline void highlightCell(int row, int col, const std::string& symbol) {
    // Ajusta estas coordenadas si tu tablero tiene encabezado o márgenes
    int screenRow = 6 + row; // por ejemplo, si el tablero empieza en fila 6
    int screenCol = 3 + col * 2; // cada celda toma 2 espacios: " ·"
    gotoxy(screenRow, screenCol);
    std::cout << "\033[7m" << symbol << "\033[0m"; // invertido temporal
}

void drawStatusBar(const std::string& msg, int rowY);


#endif // CONSOLE_UTILS_H
