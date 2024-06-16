#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <openssl/ssl.h>

#include "send_receive.h"

#define SEP "<sep>"

// Function to check if the client has permission to access the directory
int has_permission(const char *directory_path) {
    DWORD attributes = GetFileAttributesA(directory_path);
    return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
}

void listdir(SSL *ssl) {
    char *requested_dir = receive_data(ssl);
    if (requested_dir <= 0) {
        send_data(ssl, "Error: received data is NULL");
        return;
    }
    printf("dir is %s\n", requested_dir);

    // Check if client has permission to access the directory
    if (!has_permission(requested_dir)) {
        send_data(ssl, "Permission denied / dir does not exist");
        return;
    }

    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(requested_dir, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        send_data(ssl, "Failed to open directory");
        return;
    }

    // Initialize buffer with reasonable initial size
    size_t buffer_size = 4096;
    char *buffer = (char *)malloc(buffer_size);
    if (buffer == NULL) {
        send_data(ssl, "Memory allocation error");
        FindClose(hFind);
        return;
    }

    // Ensure buffer starts as an empty string
    buffer[0] = '\0';
    size_t current_length = 0;

    do {
        // Process the file
        char *file_name = find_data.cFileName;

        // Construct full path to file
        char full_path[PATH_MAX];
        snprintf(full_path, PATH_MAX, "%s/%s", requested_dir, file_name);

        // Get file attributes
        DWORD attributes = GetFileAttributesA(full_path);
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            continue;
        }

        // Check if it's a directory
        char permissions[11];
        permissions[0] = (attributes & FILE_ATTRIBUTE_DIRECTORY) ? 'd' : '-';
        permissions[1] = '-';
        permissions[2] = '-';
        permissions[3] = '-';
        permissions[4] = '-';
        permissions[5] = '-';
        permissions[6] = '-';
        permissions[7] = '-';
        permissions[8] = '-';
        permissions[9] = '-';
        permissions[10] = '\0';

        // Get formatted entry details
        size_t info_length = snprintf(NULL, 0, "%s %s\n", permissions, file_name);

        // Check and resize buffer if needed
        if (current_length + info_length + 1 > buffer_size) {
            buffer_size = buffer_size * 2 + info_length; // Adjust growth factor as needed
            char *new_buffer = (char *)realloc(buffer, buffer_size);
            if (new_buffer == NULL) {
                send_data(ssl, "Memory allocation error");
                FindClose(hFind);
                free(buffer);
                return;
            }
            buffer = new_buffer;
        }

        // Concatenate formatted entry details to the buffer
        char entry_details[info_length + 1];
        snprintf(entry_details, sizeof(entry_details), "%s %s\n", permissions, file_name);
        strcat(buffer, entry_details);
        current_length += info_length;
    } while (FindNextFileA(hFind, &find_data) != 0);

    FindClose(hFind);
    send_data(ssl, buffer);

    // Free the allocated buffer
    free(buffer);
}
