#pragma once
#include <vector>
#include <utility>

class GameLogic {
public:
    enum CellState { Hidden, Revealed, Flagged };
    struct Cell {
        bool hasMine = false;
        int adjacentMines = 0;
        CellState state = Hidden;
    };

    GameLogic(int width, int height, int mines);
    void reveal(int x, int y);
    void toggleFlag(int x, int y);
    bool isGameOver() const;
    bool isWin() const;
    const std::vector<std::vector<Cell>>& getBoard() const;
    std::vector<std::vector<Cell>>& getMutableBoard();
    int getWidth() const;
    int getHeight() const;
private:
    int width, height, totalMines;
    bool gameOver = false;
    bool win = false;
    std::vector<std::vector<Cell>> board;
    void placeMines(int firstX, int firstY);
    void calculateAdjacents();
    void revealRecursive(int x, int y);
    bool minesPlaced = false;
};
