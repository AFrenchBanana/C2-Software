#include <stdio.h>
#include <stdlib.h>
#include <openssl/ssl.h>

#include "hashing.h"
#include "send_receive.h"

void hash_file(SSL* ssl){
    char * file = receive_data(ssl); 
    send_data(ssl, "1");
    unsigned char data[SHA256_DIGEST_LENGTH];
    FILE *fp = fopen(file, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file\n");
        return;
    }
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    unsigned char *file_data = malloc(file_size);
    if (file_data == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        fclose(fp);
        return;
    }
    if (fread(file_data, 1, file_size, fp) != file_size) {
        fprintf(stderr, "Error reading file\n");
        fclose(fp);
        free(file_data);
        return;
    }
    fclose(fp);
    sha256(file_data, data);
    free(file_data);
    printf("Hash: ");
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
    send_data(ssl, file);
    send_data(ssl, data);
    puts("Hash sent");
}
