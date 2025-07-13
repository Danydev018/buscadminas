#include "client/Client.h"
#include <iostream>

int main() {
    Client cli;
    cli.discover();
    std::cout<<"Selecciona servidor: ";
    int idx; std::cin>>idx;
    cli.connectTo(idx);
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    cli.play();
    return 0;
}
