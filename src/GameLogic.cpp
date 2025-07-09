
#include <vector>
#include <algorithm>
#include "GameLogic.hpp"
#include <random>

std::vector<std::vector<GameLogic::Cell>>& GameLogic::getMutableBoard() {
    return board;
}

GameLogic::GameLogic(int w, int h, int mines)
    : width(w), height(h), totalMines(mines), board(h, std::vector<Cell>(w)) {}

void GameLogic::placeMines(int firstX, int firstY) {
    std::vector<std::pair<int, int>> positions;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            if (!(x == firstX && y == firstY))
                positions.emplace_back(x, y);
    std::shuffle(positions.begin(), positions.end(), std::mt19937{std::random_device{}()});
    for (int i = 0; i < totalMines && i < (int)positions.size(); ++i)
        board[positions[i].second][positions[i].first].hasMine = true;
    calculateAdjacents();
    minesPlaced = true;
}

void GameLogic::calculateAdjacents() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int count = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                        if (board[ny][nx].hasMine)
                            ++count;
                }
            board[y][x].adjacentMines = count;
        }
    }
}

void GameLogic::reveal(int x, int y) {
    if (!minesPlaced) placeMines(x, y);
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    if (board[y][x].state != Hidden) return;
    if (board[y][x].hasMine) {
        gameOver = true;
        board[y][x].state = Revealed;
        return;
    }
    revealRecursive(x, y);
    // Check win
    int hidden = 0;
    for (const auto& row : board)
        for (const auto& cell : row)
            if (cell.state == Hidden && !cell.hasMine)
                ++hidden;
    if (hidden == 0) win = true;
}

void GameLogic::revealRecursive(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    // Si hay bandera y no es mina, quitar bandera antes de revelar
    if (board[y][x].state == Flagged && !board[y][x].hasMine)
        board[y][x].state = Hidden;
    if (board[y][x].state != Hidden) return;
    board[y][x].state = Revealed;
    if (board[y][x].adjacentMines == 0 && !board[y][x].hasMine) {
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (dx != 0 || dy != 0)
                    revealRecursive(x + dx, y + dy);
    }
}

void GameLogic::toggleFlag(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    if (board[y][x].state == Hidden)
        board[y][x].state = Flagged;
    else if (board[y][x].state == Flagged)
        board[y][x].state = Hidden;
}

bool GameLogic::isGameOver() const { return gameOver; }
bool GameLogic::isWin() const { return win; }
const std::vector<std::vector<GameLogic::Cell>>& GameLogic::getBoard() const { return board; }
int GameLogic::getWidth() const { return width; }
int GameLogic::getHeight() const { return height; }
