#include "common/NetworkUtils.h"  
#include <iostream>  
#include <cstring>  
  
int NetworkUtils::safeRecv(int sockfd, void* buffer, size_t size, int timeoutSec) {  
    fd_set readfds;  
    struct timeval timeout;  
      
    FD_ZERO(&readfds);  
    FD_SET(sockfd, &readfds);  
      
    timeout.tv_sec = timeoutSec;  
    timeout.tv_usec = 0;  
      
    int selectResult = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);  
      
    if (selectResult == -1) {  
        std::cerr << "Error en select(): " << strerror(errno) << std::endl;  
        return -1;  
    } else if (selectResult == 0) {  
        std::cerr << "Timeout: No se recibieron datos en " << timeoutSec << " segundos" << std::endl;  
        return -2; // Timeout  
    }  
      
    int bytesReceived = recv(sockfd, buffer, size, 0);  
    if (bytesReceived == 0) {  
        std::cerr << "Conexión cerrada por el peer" << std::endl;  
        return 0;  
    } else if (bytesReceived == -1) {  
        std::cerr << "Error en recv(): " << strerror(errno) << std::endl;  
        return -1;  
    } else if (bytesReceived != static_cast<int>(size)) {  
        std::cerr << "Datos incompletos: esperados " << size << ", recibidos " << bytesReceived << std::endl;  
        return -3; // Datos incompletos  
    }  
      
    return bytesReceived;  
}  
  
int NetworkUtils::safeSend(int sockfd, const void* buffer, size_t size) {  
    int bytesSent = send(sockfd, buffer, size, 0);  
    if (bytesSent == -1) {  
        std::cerr << "Error en send(): " << strerror(errno) << std::endl;  
        return -1;  
    } else if (bytesSent != static_cast<int>(size)) {  
        std::cerr << "Envío incompleto: esperados " << size << ", enviados " << bytesSent << std::endl;  
        return -2;  
    }  
    return bytesSent;  
}  
  
bool NetworkUtils::isSocketConnected(int sockfd) {  
    int error = 0;  
    socklen_t len = sizeof(error);  
    int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);  
    return (retval == 0 && error == 0);  
}