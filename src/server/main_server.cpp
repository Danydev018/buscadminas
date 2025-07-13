#include "server/Server.h"

int main() {
    Server srv("Sala1", 10, 10, 15);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    srv.run();
    return 0;
}
