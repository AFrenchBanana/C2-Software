#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <openssl/ssl.h>

#ifndef S_ISVTX
#define S_ISVTX  01000 // Sticky bit: not supported
#endif



// Function to convert permission bits to human-readable format
char *get_permissions(mode_t mode) {
    char *permissions = (char *)malloc(10);

    return permissions;
}

// Function to check if the client has permission to access the directory
int has_permission(const char *directory_path) {
    return access(directory_path, R_OK | X_OK) == 0; // Check read and execute permissions
}

void listdir(SSL *ssl) {
   puts("working");
}

