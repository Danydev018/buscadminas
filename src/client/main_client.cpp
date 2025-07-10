#include "client/Client.h"
#include <iostream>

int main() {
    Client cli;
    cli.discover();
    std::cout<<"Selecciona servidor: ";
    int idx; std::cin>>idx;
    cli.connectTo(idx);
    cli.play();
    return 0;
}
