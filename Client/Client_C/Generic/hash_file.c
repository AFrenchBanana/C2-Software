#include <stdio.h>
#include <stdlib.h>


#include "hashing.h"
#include "send_receive.h"

void hash_file(int sockfd){
        char * file = receive_data(sockfd); 
        send_data(sockfd, "1");
        unsigned char data[1024];
        int bytes;
        FILE *fp = fopen(file, "rb");
        if (fp == NULL) {
            fprintf(stderr, "Error opening file\n");
            return;
        }
        while ((bytes = fread(data, 1, 1024, fp)) != 0) {
            sha256(data, data);
        }
        fclose(fp);
        printf("Hash: %s\n", data);
        send_data(sockfd, file);
        send_data(sockfd, data);
        puts("Hash sent");
    }
