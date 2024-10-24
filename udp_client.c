#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 29527
#define SERVER_IP "10.23.149.5"
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *json_message = "{\"name\": \"Fengpangzi\"}";
    char buffer[BUFFER_SIZE] = {0};

    // Create a TCP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    // Server address setup
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Send JSON message to the server
    if (sendto(sock, json_message, strlen(json_message), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Send message failed\n");
        close(sock); // Close the client socket
        return -1;
    }
    printf("Message sent to server : %s\n", json_message);

    // Optionally, read the server's response
//    int valread = read(sock, buffer, BUFFER_SIZE);
//    printf("Server response: %s\n", buffer);

    close(sock); // Close the client socket
    return 0;
}

