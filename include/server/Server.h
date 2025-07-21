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
    Server(const std::string& name, int rows, int cols, int mines, int difficulty);  
    ~Server();  
    void run();  
    void stop();  
  
private:  
    std::string roomName;  
    int         R, C, M;  
    int         difficulty;  
    uint32_t    seed;  
    Board       board;  
    bool        turnHost{true};  
      
    // Control del hilo de descubrimiento  
    std::atomic<bool> shouldStop{false};  
    std::thread discoveryThread;  
    int discoverySocket{-1};  
  
    void discoveryListener();  
    void gameLoop();  
    bool validateMove(const Move& mv) const; 
     
};  

struct MultiplayerScore {  
    int hostScore;  
    int clientScore;  
    int hostWon;  
    int clientWon;  
    double gameTime;  
    int hostClicks, clientClicks, hostFlags, clientFlags;  
}; 
  
#endif