#ifndef NETWORKUTILS_H  
#define NETWORKUTILS_H  
  
#include <sys/socket.h>  
#include <sys/select.h>  
#include <errno.h>  
#include <unistd.h>  
  
class NetworkUtils {  
public:  
    // Recv con timeout y verificación de errores  
    static int safeRecv(int sockfd, void* buffer, size_t size, int timeoutSec = 30);  
      
    // Send con verificación de errores  
    static int safeSend(int sockfd, const void* buffer, size_t size);  
      
    // Verificar si socket está conectado  
    static bool isSocketConnected(int sockfd);  
};  
  
#endif