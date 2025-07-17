#include "common/ConsoleUtils.h"
#include <termios.h>
#include <unistd.h>
#include <chrono>
#include <thread>

void gotoxy(int x, int y) {
    std::cout << "\033[" << y << ";" << x << "H";
}

void drawFrameAroundBoard(int startX, int startY, int width, int height) {  
    gotoxy(startX - 2, startY - 1);  
    std::cout << "╔";  
    for (int i = 0; i < width * 3; ++i) std::cout << "═";   
    std::cout << "╗";  
  
    for (int j = 0; j < height; ++j) {  
        gotoxy(startX - 2, startY + j);  
        std::cout << "║";  
        gotoxy(startX + (width - 1) * 3 + 2, startY + j);   
        std::cout << "║";  
    }  
  
    gotoxy(startX - 2, startY + height);  
    std::cout << "╚";  
    for (int i = 0; i < width * 3; ++i) std::cout << "═";  
    std::cout << "╝";  
}

// void drawFrameAroundBoard(int startX, int startY, int width, int height) {  
//     gotoxy(startX - 2, startY - 1);  
//     std::cout << "╔";  
//     // Cambiar de width * 3 a (width - 1) * 3 + 1  
//     for (int i = 0; i < (width - 1) * 3 + 1; ++i) std::cout << "═";  
//     std::cout << "╗";  
  
//     for (int j = 0; j < height; ++j) {  
//         gotoxy(startX - 2, startY + j);  
//         std::cout << "║";  
//         // Cambiar de width * 3 a (width - 1) * 3 + 1  
//         gotoxy(startX + (width - 1) * 3 + 1, startY + j);  
//         std::cout << "║";  
//     }  
  
//     gotoxy(startX - 2, startY + height);  
//     std::cout << "╚";  
//     // Cambiar de width * 3 a (width - 1) * 3 + 1  
//     for (int i = 0; i < (width - 1) * 3 + 1; ++i) std::cout << "═";  
//     std::cout << "╝";  
// }

KeyCode getKey() {
    termios oldt{}, newt{};
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);  // modo raw
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    char ch1 = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // restaurar

    if (ch1 == 27) {
        char ch2 = getchar();
        if (ch2 == '[') {
            char ch3 = getchar();
            if (ch3 == 'A') return KEY_UP;
            if (ch3 == 'B') return KEY_DOWN;
            if (ch3 == 'C') return KEY_RIGHT;
            if (ch3 == 'D') return KEY_LEFT;
        }
        return KEY_ESC;
    }
    if (ch1 == 'f' || ch1 == 'F') return KEY_FLAG;
    if (ch1 == 'q' || ch1 == 'Q') return KEY_QUIT;
    if (ch1 == '\n' || ch1 == ' ' || ch1 == 'r' || ch1 == 'R') return KEY_ENTER;

    return KEY_NONE;
}

void updateBoardDisplay(int startX, int startY, const Board& board) {
    clearScreen();
    gotoxy(1, 1);
    drawFrameAroundBoard(startX, startY, board.cols(), board.rows());
    board.drawGotoxy(startX, startY);
}

void drawStatusBar(const std::string& msg, int rowY) {
    gotoxy(2, rowY);
    std::cout << "\033[36m" << msg << "\033[0m";  // Cyan
}

void showAllMines(const Board& board, const std::string& gameResult) {  
    // Limpiar y redibujar el tablero  
    clearScreen();  
    gotoxy(1, 1);  
    drawFrameAroundBoard(4, 2, board.cols(), board.rows());  
    board.drawGotoxy(4, 2);  
      
    // Mostrar todas las minas  
    for (int r = 0; r < board.rows(); r++) {  
        for (int c = 0; c < board.cols(); c++) {  
            if (board.isMine(r, c)) {  
                highlightCell(r, c, "💣");  
            }  
        }  
    }  
      
    // Mostrar el resultado del juego DESPUÉS de las minas  
    gotoxy(2, board.rows() + 5);  
    std::cout << gameResult << std::endl;  
      
    gotoxy(2, board.rows() + 6);  
    std::cout << "Presiona cualquier tecla para continuar...";  
    getKey();  
}