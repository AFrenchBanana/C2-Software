#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <openssl/ssl.h>
#include "send_receive.h"
#include "../Linux/systeminfo.h"
#include "hash_file.h"
#include "../Linux/list_dir.h"


void server_handler(SSL* ssl){
    while(true){
        char *data = receive_data(ssl);
        if (data == NULL){
            free(data);
            continue;
        }
        if (strcmp(data, "shutdown") == 0) {
            free(data);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            exit(0);
        } else if (strcmp(data, "systeminfo") == 0) {
            systeminfo(ssl);
        } else if (strcmp(data, "checkfiles") == 0) {
            printf("Hashing file\n");
            hash_file(ssl);
        } else if (strcmp(data, "list_dir") == 0) {
            printf("Listing directory\n");
            listdir(ssl);
        }        
        else {
            send_data(ssl, "Invalid command");
        }
        free(data);
    }
}