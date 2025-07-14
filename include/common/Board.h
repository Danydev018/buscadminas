#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <random>
#include <iostream>

enum CellState { Hidden, Revealed, Flagged };

struct Cell {
    bool hasMine{};
    CellState state{Hidden};
    int adjacentMines{0};
};

class Board {
public:
    Board(int rows, int cols, int mines, unsigned seed);

    // Revela una casilla (y expande si no hay adyacentes)
    void reveal(int r, int c);

    // Coloca o quita bandera
    void toggleFlag(int r, int c);

    bool isMine(int r, int c) const;
    bool allSafeRevealed() const;
    const Cell& at(int r, int c) const;
    int rows() const;
    int cols() const;

    // Dibuja el tablero en pantalla según el estado de cada celda
    void print() const;
    void drawGotoxy(int startX, int startY) const;


private:
    int R, C, M;
    std::vector<Cell> grid;
    void placeMines(unsigned seed);
    void computeAdjacents();
    void floodFill(int r, int c);
    int index(int r, int c) const;
};

#endif // BOARD_H
