#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdint.h> 
#include <openssl/evp.h>

#define PORT 11001
#define SOCK_ADDRESS "127.0.0.1"
#define MAX_BUFFER_SIZE 4096
#define HEADER_SIZE 8

int sockfd;
struct sockaddr_in server_addr;

int ssl_connection(void);
char* receive_data(void);
void send_data(const char*);
void authentication(void);
char* reverseString(char *);
void sha512(const char *, char *);


int main() {
    if (ssl_connection() != 0) {
        fprintf(stderr, "Error establishing SSL connection\n");
        return EXIT_FAILURE;
    }
    authentication();
    send_data("Host Test");
    char* test = receive_data();
    if (test == 0) {
        fprintf(stderr, "Error receiving data\n");
        return EXIT_FAILURE;
    }
    printf("Packet Sniffer Place Holder: %s\n", test);
    free(test);
    receive_data();
}

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
        close(sockfd);
        return EXIT_FAILURE;
    }

    puts("Connected to server");
    return EXIT_SUCCESS;
}


char* receive_data() {
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

void send_data(const char* data) {
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

void authentication(){
    puts("Time to authenticate");
    char * intial_key = receive_data();
    printf("Received data: %s\n", intial_key);
    socklen_t len = sizeof(server_addr);
    if (getsockname(sockfd, (struct sockaddr*)&server_addr, &len) == -1) {
        perror("Error getting socket name");
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
    send_data(output); // Send the hash to the server
}


char* reverseString(char str[]) {
    int length = strlen(str);
    int start = 0;
    int end = length - 1;

    while (start < end) {
        // Swap characters at start and end
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;

        // Move towards the center
        start++;
        end--;
    }
    return str;
}


void sha512(const char *input, char *output) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    md = EVP_sha512();
    mdctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, strlen(input));
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);

    EVP_MD_CTX_free(mdctx);

    for (int i = 0; i < hash_len; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[hash_len * 2] = '\0';
}
