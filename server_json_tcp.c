#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cjson/cJSON.h>

#define SERVER_PORT 29537          // Server's listening port
#define BUFFER_SIZE 1024           // Buffer size for receiving data
#define BACKLOG 5                  // Number of pending connections queue will hold

// Function to send JSON message
int send_json(int client_sock, cJSON *json_obj) {
    char *message = cJSON_PrintUnformatted(json_obj);
    if (message == NULL) {
        fprintf(stderr, "Failed to print JSON.\n");
        return -1;
    }

    ssize_t sent_bytes = send(client_sock, message, strlen(message), 0);
    if (sent_bytes < 0) {
        perror("send failed");
        free(message);
        return -1;
    }

    printf("Sent: %s\n", message);
    free(message);
    return 0;
}

// Function to receive JSON message
cJSON* receive_json(int client_sock) {
    char buffer[BUFFER_SIZE];
    ssize_t recv_bytes = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (recv_bytes < 0) {
        perror("recv failed");
        return NULL;
    } else if (recv_bytes == 0) {
        printf("Client disconnected.\n");
        return NULL;
    }

    buffer[recv_bytes] = '\0';  // Null-terminate the received string
    printf("Received: %s\n", buffer);

    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        fprintf(stderr, "Error parsing JSON.\n");
        return NULL;
    }

    return json;
}

// Function to process received JSON and prepare response
cJSON* process_message(cJSON *received_json) {
    if (!cJSON_IsObject(received_json)) {
        fprintf(stderr, "Received JSON is not an object.\n");
        return NULL;
    }

    cJSON *op = cJSON_GetObjectItem(received_json, "op");
    cJSON *content = cJSON_GetObjectItem(received_json, "content");

    cJSON *response = cJSON_CreateObject();
    if (!response) {
        fprintf(stderr, "Failed to create JSON object.\n");
        return NULL;
    }

    if (cJSON_IsString(op)) {
        if (strcmp(op->valuestring, "start exchange") == 0) {
            cJSON_AddStringToObject(response, "status", "ready");
        } else if (strcmp(op->valuestring, "endconversation") == 0) {
            cJSON_AddStringToObject(response, "status", "ready");
        } else {
            cJSON_AddStringToObject(response, "status", "unknown operation");
        }
    } else if (cJSON_IsString(content)) {
        // Assuming content type "cert" along with username and password
        cJSON *username = cJSON_GetObjectItem(received_json, "username");
        cJSON *password = cJSON_GetObjectItem(received_json, "password");

        if (cJSON_IsString(username) && cJSON_IsString(password)) {
            // Here you can add authentication or processing logic
            printf("Processing credentials: username=%s, password=%s\n",
                   username->valuestring, password->valuestring);
            cJSON_AddStringToObject(response, "status", "received");
        } else {
            cJSON_AddStringToObject(response, "status", "invalid credentials");
        }
    } else {
        cJSON_AddStringToObject(response, "status", "invalid message");
    }

    return response;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    char client_ip[INET_ADDRSTRLEN];

    // Create TCP socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow reuse of address and port
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Configure server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to all available interfaces
    memset(&(server_addr.sin_zero), '\0', 8);  // Zero the rest of the struct

    // Bind socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen on the socket
    if (listen(server_sock, BACKLOG) == -1) {
        perror("listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("TCP server listening on port %d...\n", SERVER_PORT);

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        // Accept incoming connection
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
            perror("accept failed");
            continue;
        }

        // Convert client IP to string
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Got connection from %s\n", client_ip);

        // Handle client communication
        while (1) {
            cJSON *received_json = receive_json(client_sock);
            if (received_json == NULL) {
                // Either client disconnected or an error occurred
                break;
            }

            // Process the received JSON and prepare response
            cJSON *response_json = process_message(received_json);
            cJSON_Delete(received_json);  // Free the received JSON object

            if (response_json == NULL) {
                // If processing failed, send an error response
                cJSON *error_json = cJSON_CreateObject();
                cJSON_AddStringToObject(error_json, "status", "error processing message");
                send_json(client_sock, error_json);
                cJSON_Delete(error_json);
                continue;
            }

            // Send the response JSON to the client
            if (send_json(client_sock, response_json) < 0) {
                cJSON_Delete(response_json);
                break;
            }

            cJSON_Delete(response_json);  // Free the response JSON object
        }

        // Close the client socket after communication
        close(client_sock);
        printf("Connection with %s closed.\n", client_ip);
    }

    // Close the server socket (unreachable in this example)
    close(server_sock);
    return 0;
}
