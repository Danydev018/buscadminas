#ifndef SERVER_H  
#define SERVER_H  
  
#include "common/Board.h"  
#include "common/NetworkCommon.h"  
#include "common/NetworkUtils.h"  
#include <thread>  
#include <string>  
#include <atomic>  
  
class Server {
public:
    Server(const std::string& name, int rows, int cols, int mines);
    ~Server();
    void run();
    void stop();

private:
    std::string roomName;
    int         R, C, M;
    uint32_t    seed;
    Board       board;
    bool        turnHost{true};
    int         scoreHost{0};
    int         scoreClient{0};

    // Control del hilo de descubrimiento
    std::atomic<bool> shouldStop{false};
    std::thread discoveryThread;
    int discoverySocket{-1};

    void discoveryListener();
    void gameLoop();
    bool validateMove(const Move& mv) const;
    void printScores() const;
    void saveWinnerToCSV(const std::string& winnerName, int score) const;
};
  
#endif