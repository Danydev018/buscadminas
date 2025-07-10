#ifndef NETWORKCOMMON_H
#define NETWORKCOMMON_H

#include <cstdint>

// Puerto de discovery y base para TCP
constexpr uint16_t DISCOVERY_PORT = 30000;
constexpr uint16_t GAME_PORT_BASE = 40000;

// Mensajes UDP de descubrimiento
struct DiscoveryRequest  { char magic[4] = {'M','S','?','?'}; };
struct DiscoveryReply {
    char magic[4] = {'M','S','!','!'};
    uint16_t tcpPort;      // en network order
    char     name[32];
};

// Movimiento en la partida
struct Move {
    uint8_t row;
    uint8_t col;
    uint8_t isFlag;        // 0=reveal, 1=flag
};

// Al iniciar la partida, enviamos esto por TCP
struct GameInit {
    uint32_t seed;         // en network order (htonl)
    uint8_t  rows;
    uint8_t  cols;
    uint8_t  mines;
};

#endif // NETWORKCOMMON_H
