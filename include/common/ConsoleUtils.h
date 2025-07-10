#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H

#include <iostream>

// Limpia toda la pantalla y posiciona el cursor arriba a la izquierda
inline void clearScreen() {
    std::cout << "\033[2J\033[H";
}

#endif // CONSOLE_UTILS_H
