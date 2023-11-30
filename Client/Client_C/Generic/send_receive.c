#define MAX_BUFFER_SIZE 4096
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../Linux/linux_function.h"

char* receive_data(int sockfd){
    uint32_t total_length, chunk_size;  // create 2 integers to store header data
    char* received_data = NULL;        // create a pointer to store received data
    // Receive header
    //check if the total length has been received
    if (recv(sockfd, &total_length, sizeof(uint32_t), 0) < 0) { 
        perror("Error receiving total length");
        return NULL;
    }
    //check if the chunk size has been received
    if (recv(sockfd, &chunk_size, sizeof(uint32_t), 0) < 0) {
        perror("Error receiving chunk size");
        return NULL;
    }
    //convert network byte order to host byte order
    total_length = ntohl(total_length);
    chunk_size = ntohl(chunk_size);
    //print out the total length and chunk size for debugging
    printf("Total length: %d\n", total_length);
    printf("Chunk size: %d\n", chunk_size);

    // Allocate memory for received data
    received_data = (char*)malloc(total_length + 1); // +1 for null terminator
    if (received_data == NULL) {
        perror("Error allocating memory");
        return NULL;
    }

    size_t received_bytes = 0; // sets the number of bytes received to 0
    // loops until the total length remaining is 0
    while (total_length > 0) {
        char chunk[MAX_BUFFER_SIZE];  // Use MAX_BUFFER_SIZE as the chunk size
        ssize_t bytes_received = recv(sockfd, chunk, sizeof(chunk), 0); //receive data from the server
        if (bytes_received < 0) { //check if the data has been received
            perror("Error receiving data"); //print for debugging
            free(received_data);
            return NULL;
        }
        printf("Received %ld bytes\n", bytes_received); //print for debugging
        //check if the connection has been closed
        if (bytes_received == 0) {
            fprintf(stderr, "Connection closed by the remote end\n"); //print for debugging
            free(received_data); //free the memory
            return NULL;
        }
        //copy the received data to the received data pointer
        memcpy(received_data + received_bytes, chunk, bytes_received);
        //update the number of bytes received and the total length remaining
        received_bytes += bytes_received;
        //update the total length remaining
        total_length -= bytes_received;
    }
    // Null-terminate the received data
    received_data[received_bytes] = '\0'; // Null-terminate the received data
    return received_data; //return the received data
}

void send_data(int sockfd, const char* data) {
    // Get the total length of the data
    uint32_t total_length = strlen(data);
    // Set the max chunk size
    uint32_t chunk_size = MAX_BUFFER_SIZE;

    // Send header consisting of total length and max chunk size
    uint32_t total_length_network = htonl(total_length); //convert to network byte order
    uint32_t chunk_size_network = htonl(chunk_size); //convert to network byte order
    send(sockfd, &total_length_network, sizeof(uint32_t), 0); //send the total length
    send(sockfd, &chunk_size_network, sizeof(uint32_t), 0); //send the chunk size

    // Send data in chunks or less than max chunk size
    for (size_t i = 0; i < total_length; i += chunk_size) {
        // Calculate the end index of the chunk to send, this means it wont send more than the total length of the data
        size_t end_index = i + chunk_size < total_length ? i + chunk_size : total_length; 
        // Calculate the chunk size to send
        size_t chunk_size_to_send = end_index - i;
        // Send the chunk
        const char* chunk = data + i;
        send(sockfd, chunk, chunk_size_to_send, 0);
    }
}