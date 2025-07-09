#pragma once
#include "GameLogic.hpp"
#include <string>
#include <memory>

class GameUI {
public:
    GameUI(std::shared_ptr<GameLogic> logic);
    void render();
    void handleInput(int x, int y, bool flag);
    void showMessage(const std::string& msg);
private:
    std::shared_ptr<GameLogic> logic;
    std::string message;
};
