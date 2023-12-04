#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>


#include "Generic/send_receive.h"
#include "Linux/socket.h"
#include "Generic/string_manipulation.h"
#include "Linux/systeminfo.h"
#include "Generic/server_handler.h"
#include "Generic/hash_file.h"


int main() {
    while(true) {
        int sockfd = ssl_connection();
        if (sockfd == 0) {
            fprintf(stderr, "Error establishing SSL connection\n");
            return EXIT_FAILURE;
        }
        authentication();
        char* hostname = get_hostname();
        send_data(sockfd, hostname);
        //free(hostname);
        char* sniffer_dummy = receive_data(sockfd);
        printf("Sniffer Reply:  %s\n", sniffer_dummy); // sniffer dummy
        free(sniffer_dummy);
        server_handler(sockfd);
        close(sockfd);
    }
}




