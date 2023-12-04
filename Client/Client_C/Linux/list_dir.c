#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/syscall.h>
#include <string.h>

#include "../Generic/send_receive.h"

void listdir(int sockfd) {
    DIR *dir;
    struct dirent *entry;
    char *requested_dir = receive_data(sockfd);
    dir = opendir(requested_dir);

    if (dir == NULL) {
        send_data(sockfd, "Error: opening directory");
        return;
    }

    // Initialize a buffer with reasonable initial size
    size_t buffer_size = 4096;  // Adjust this size as needed
    char *buffer = (char *)malloc(buffer_size);
    if (buffer == NULL) {
        send_data(sockfd, "Memory allocation error");
        closedir(dir);
        return;
    }

    buffer[0] = '\0';  // Ensure the buffer is an empty string
    size_t current_length = 0;

    while ((entry = readdir(dir)) != NULL) {
        char *file_name = entry->d_name;
        size_t file_name_length = strlen(file_name);

        // Check if there's enough space in the buffer
        if (current_length + file_name_length + 2 > buffer_size) {
            // Resize the buffer using realloc
            buffer_size *= 2;
            char *new_buffer = (char *)realloc(buffer, buffer_size);
            if (new_buffer == NULL) {
                send_data(sockfd, "Memory allocation error");
                closedir(dir);
                free(buffer);
                return;
            }
            buffer = new_buffer;
        }

        strcat(buffer, file_name);
        strcat(buffer, "\n");
        current_length += file_name_length + 1;
    }
    printf("Buffer: %s\n", buffer);
    closedir(dir);
    send_data(sockfd, buffer);

    // Free the allocated buffer
    free(buffer);
}