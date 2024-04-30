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
#include <string.h>
#include <openssl/ssl.h>

#include "../Generic/send_receive.h"

void listdir(SSL *ssl) {
    DIR *dir;
    struct dirent *entry;
    char *requested_dir = receive_data(ssl);
    if (requested_dir == NULL) {
        printf("Error: received data is NULL\n");
        return;
    }
    printf("dir is %s\n", requested_dir);
    dir = opendir(requested_dir);
    if (dir == NULL) {
        printf("Error: cannot open directory %s\n", requested_dir);
        return;
    }

    // Initialize buffer with reasonable initial size
    size_t buffer_size = 4096;
    char *buffer = (char *)malloc(buffer_size);
    if (buffer == NULL) {
        send_data(ssl, "Memory allocation error");
        closedir(dir);
        return;
    }

    // Ensure buffer starts as an empty string
    buffer[0] = '\0';
    size_t current_length = 0;

    while ((entry = readdir(dir)) != NULL) {
        char *file_name = entry->d_name;
        struct stat file_stat;
        printf("%s", file_name);
        // Get file metadata and handle potential errors
        if (stat(entry->d_name, &file_stat) != 0) {
            continue;
        }

        // Get file permissions in octal format
        char permissions[11];
        snprintf(permissions, sizeof(permissions), "%o", (unsigned int)file_stat.st_mode);

        // Get user and group information (optional)
        struct passwd *pw = getpwuid(file_stat.st_uid);
        struct group *gr = getgrgid(file_stat.st_gid);
        char *user_name = pw ? pw->pw_name : "unknown";
        char *group_name = gr ? gr->gr_name : "unknown";

        // Get formatted time (adjust format as needed)
        char formatted_time[32];
        strftime(formatted_time, sizeof(formatted_time), "%b %d %H:%M", localtime(&file_stat.st_mtime));

        // Format entry details
        size_t info_length = snprintf(NULL, 0,
                                      "%c %ld %s %s %lld %.12s %s\n",
                                      S_ISDIR(file_stat.st_mode) ? 'd' : '-', // File type (directory or -)
                                      (long)file_stat.st_nlink,  // Number of hard links
                                      user_name,
                                      group_name,
                                      (long long)file_stat.st_size, // File size
                                      formatted_time + 4,  // Skip the day of the week
                                      file_name);

        // Check and resize buffer if needed
        if (current_length + info_length + 1 > buffer_size) {
            buffer_size = buffer_size * 2 + info_length; // Adjust growth factor as needed
            char *new_buffer = (char *)realloc(buffer, buffer_size);
            if (new_buffer == NULL) {
                send_data(ssl, "Memory allocation error");
                closedir(dir);
                free(buffer);
                return;
            }
            buffer = new_buffer;
        }

        // Concatenate formatted entry details to the buffer
        char entry_details[info_length + 1];
        snprintf(entry_details, sizeof(entry_details),
                 "%c %ld %s %s %lld %.12s %s\n",
                 S_ISDIR(file_stat.st_mode) ? 'd' : '-', // File type (directory or -)
                 (long)file_stat.st_nlink,  // Number of hard links
                 user_name,
                 group_name,
                 (long long)file_stat.st_size, // File size
                 formatted_time + 4,  // Skip the day of the week
                 file_name);
        strcat(buffer, entry_details);
        current_length += info_length;
    }

    closedir(dir);
    send_data(ssl, buffer);

    // Free the allocated buffer
    free(buffer);
    free(buffer_size)
}
