#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "send_receive.h"

#define MAX_BUFFER_SIZE 4096

int send_header(int sockfd, uint32_t total_length, uint32_t chunk_size) {
    total_length = htonl(total_length); // convert from host to network byte order
    chunk_size = htonl(chunk_size); // convert from host to network byte order

    // Create a buffer to hold the serialized data
    char buffer[sizeof(uint32_t) * 2];
    // Copy the total_length and chunk_size to the buffer
    memcpy(buffer, &total_length, sizeof(uint32_t));
    memcpy(buffer + sizeof(uint32_t), &chunk_size, sizeof(uint32_t));
    // Print the bytes
    printf("Header Bytes: ");
    for (size_t i = 0; i < sizeof(buffer); ++i) {
        printf("%02X ", (unsigned char)buffer[i]);
    }
    printf("\n");
    // Send the serialized data
    ssize_t sent_bytes = send(sockfd, buffer, sizeof(buffer), 0);
    if (sent_bytes < 0 || sent_bytes != sizeof(buffer)) {
        perror("Error sending header");
        return -1;
    }
    return 0;
}

void send_data(int sockfd, const char* data) {
    // Get the total length of the data
    uint32_t total_length = strlen(data);
    uint32_t chunk_size = MAX_BUFFER_SIZE;
    // Send the header
    if (send_header(sockfd, total_length, chunk_size) < 0) {
        return;
    }
    // Allocate a buffer and initialize it with zeros
    char* zeroed_data = (char*)calloc(total_length + 1, sizeof(char));

    // Copy the original data to the zeroed buffer
    strcpy(zeroed_data, data);
    // Send the data in chunks
    for (size_t i = 0; i < total_length; i += chunk_size) {
        size_t end_index = i + chunk_size < total_length ? i + chunk_size : total_length; //
        size_t chunk_size_to_send = end_index - i; 
        const char* chunk = zeroed_data + i; 
        ssize_t sent_bytes = send(sockfd, chunk, chunk_size_to_send, 0); 
        if (sent_bytes < 0 || sent_bytes != (ssize_t)chunk_size_to_send) {
            perror("Error sending data chunk");
            free(zeroed_data);
            return;
        }
    }
    // Free the allocated buffer
    free(zeroed_data);
    return;
}

char* receive_data(int sockfd) {
    uint32_t total_length, chunk_size;
    // Receive total_length
    if (recv(sockfd, &total_length, sizeof(uint32_t), 0) < 0) {
        perror("Error receiving total length");
        return NULL;
    }
    // Receive chunk_size
    if (recv(sockfd, &chunk_size, sizeof(uint32_t), 0) < 0) {
        perror("Error receiving chunk size");
        return NULL;
    }
    // Convert from network to host byte order
    total_length = ntohl(total_length);
    chunk_size = ntohl(chunk_size);
    // Allocate memory for received_data
    char* received_data = (char*)malloc(total_length + 1);
    if (received_data == NULL) {
        perror("Memory allocation error");
        return NULL;
    }
    // Initialize received_data with zeros
    size_t received_bytes = 0;
    while (total_length > 0) {
        char chunk[MAX_BUFFER_SIZE];
        ssize_t bytes_received = recv(sockfd, chunk, sizeof(chunk), 0);
        // Check for errors
        if (bytes_received < 0) {
            perror("Error receiving data");
            free(received_data);
            return NULL;
        }
        if (bytes_received == 0) {
            fprintf(stderr, "Connection closed by the remote end\n");
            free(received_data);
            return NULL;
        }
        // Print the size using the correct format specifier
        printf("Received %zd bytes\n", bytes_received);
        // Copy the received chunk to the allocated buffer
        memcpy(received_data + received_bytes, chunk, bytes_received);
        received_bytes += bytes_received;
        printf("received data: %s\n", received_data);
        // Update total_length within the loop
        total_length -= bytes_received;
    }
    // Null-terminate the received data
    received_data[received_bytes] = '\0';
    // Print received data
    return received_data;
}