#include "server/Server.h"

int main() {
    Server srv("Sala1", 10, 10, 15);
    srv.run();
    return 0;
}
