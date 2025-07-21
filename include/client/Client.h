#ifndef CLIENT_H  
#define CLIENT_H  
  
#include "common/Board.h"  
#include "common/NetworkCommon.h"  
#include "common/NetworkUtils.h"  // Nueva inclusión  
#include <vector>  
#include <string>  
  
struct ServerInfo {  
    std::string ip;  
    uint16_t    port;  
    std::string name;  
};  
  
class Client {  
public:  
    Client();  
    ~Client();  // Agregar destructor  
    void discover();  
    void connectTo(int index);  
    void play();  
  
private:
    Board*                     board{nullptr};  
    uint32_t                   seed;  
    bool                       turnHost{true};  
    int                        R, C, M;  
    int                        difficulty = 2; // Por defecto medio, se sincroniza en connectTo
    int                        sockfd{-1};  
    std::vector<ServerInfo>    servers;  
  
    void listServers() const;  
    bool validateMove(const Move& mv) const;  // Nueva función de validación  
};  
  
#endif // CLIENT_H