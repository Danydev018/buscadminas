#include "GameUI.hpp"
#include <iostream>

GameUI::GameUI(std::shared_ptr<GameLogic> logic) : logic(logic) {}

void GameUI::render() {
    const auto& board = logic->getBoard();
    std::cout << "   ";
    for (int x = 0; x < logic->getWidth(); ++x) std::cout << x << ' ';
    std::cout << '\n';
    for (int y = 0; y < logic->getHeight(); ++y) {
        std::cout << y << " |";
        for (int x = 0; x < logic->getWidth(); ++x) {
            const auto& cell = board[y][x];
            if (cell.state == GameLogic::Hidden) std::cout << ". ";
            else if (cell.state == GameLogic::Flagged) std::cout << "F ";
            else if (cell.hasMine) std::cout << "* ";
            else if (cell.adjacentMines > 0) std::cout << cell.adjacentMines << ' ';
            else std::cout << "  ";
        }
        std::cout << '\n';
    }
    if (!message.empty()) std::cout << message << std::endl;
}

void GameUI::handleInput(int x, int y, bool flag) {
    if (flag) logic->toggleFlag(x, y);
    else logic->reveal(x, y);
}

void GameUI::showMessage(const std::string& msg) {
    message = msg;
}
