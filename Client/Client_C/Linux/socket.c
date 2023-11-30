#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include "linux_function.h"
#include "../Generic/send_receive.h"
#define PORT 11001
#define SOCK_ADDRESS "127.0.0.1"

int sockfd;
struct sockaddr_in server_addr;

int ssl_connection() {

    // Specify the server address and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SOCK_ADDRESS, &server_addr.sin_addr);

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to the server");
        return EXIT_FAILURE;
    }

    puts("Connected to server");
    return sockfd;
}


char* get_hostname(){
    char buffer[1024];
    buffer[1023] = '\0';
    gethostname(buffer, 1023);
    printf("Hostname: %s\n", buffer);
    char *hostname = malloc(sizeof(char) * 1024);
    strcpy(hostname, buffer);
    return hostname;
}

void authentication(){
    puts("Time to authenticate");
    char * intial_key = receive_data(sockfd); // Receive the initial key from the server
    printf("Received data: %s\n", intial_key); 
    socklen_t len = sizeof(server_addr); // Get the length of the server address
    if (getsockname(sockfd, (struct sockaddr*)&server_addr, &len) == -1) { // Get the socket name
        perror("Error getting socket name"); // Print error for debugging
    } 
    printf("Port number %d\n", ntohs(server_addr.sin_port));
    char port_str[6]; // Assuming the port number will not exceed 5 digits
    sprintf(port_str, "%d", ntohs(server_addr.sin_port)); // Convert port number to string
    char *key = malloc(strlen(intial_key) + strlen(port_str) + 1); // Allocate memory for the concatenated string
    strcpy(key, intial_key); // Copy the initial key to the new string
    free(intial_key); // Free the memory for the initial key
    strcat(key, port_str); // Concatenate the port number string to the new string
    char* rev_key = reverseString(key); // Reverse the string
    printf("Key: %s\n", rev_key); // Print the key for debugging
    char output[129]; // Allocate memory for the hash
    sha512(rev_key, output); // Hash the key
    printf("Hash: %s\n", output); // Print the hash for debugging
    send_data(sockfd, output); // Send the hash to the server
}