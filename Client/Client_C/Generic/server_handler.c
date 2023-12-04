#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "send_receive.h"
#include "../Linux/systeminfo.h"
#include "hash_file.h"
#include "../Linux/list_dir.h"


void server_handler(int sockfd){
    printf("Server handler\n");
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
        } else if (strcmp(data, "systeminfo") == 0) {
            systeminfo(sockfd);
        } else if (strcmp(data, "checkfiles") == 0) {
            printf("Hashing file\n");
            hash_file(sockfd);
        } else if (strcmp(data, "list_dir") == 0) {
            printf("Listing directory\n");
            listdir(sockfd);
        }        
        else {
            send_data(sockfd, "Invalid command");
        }
        free(data);
    }
}