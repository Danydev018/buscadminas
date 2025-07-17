#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H

#include <iostream>
#include "common/Board.h"
#include <chrono>
#include <thread>
#include <stdlib.h>


enum KeyCode {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_ENTER, KEY_ESC, KEY_FLAG,
    KEY_QUIT,
    KEY_NONE
};

KeyCode getKey();


// Limpia toda la pantalla y posiciona el cursor arriba a la izquierda
inline void clearScreen() {
    system("clear");
    std::cout << "\033[2J\033[H";
}

// Posiciona el cursor en una coordenada específica
// En Board.cpp (arriba del archivo o justo antes de drawGotoxy)
void gotoxy(int x, int y);
void drawFrameAroundBoard(int startX, int startY, int width, int height);
void updateBoardDisplay(int startX, int startY, const Board& board);

inline void highlightCell(int row, int col, const std::string& symbol) {    
    int screenX = 4 + col * 3;  // Cambiar de col * 4 a col * 3  
    int screenY = 2 + row;    
    gotoxy(screenX, screenY);    
    std::cout << "\033[35m" << symbol << "\033[0m";  
}

void showAllMines(const Board& board, const std::string& gameResult);

void drawStatusBar(const std::string& msg, int rowY);


#endif // CONSOLE_UTILS_H
