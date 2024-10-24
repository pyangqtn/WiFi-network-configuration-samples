#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cjson/cJSON.h>

#define PORT 29527
#define BUFFER_SIZE 1024

void respond_json(int sock, struct sockaddr_in client_addr, cJSON *json_obj) {
    char *message = cJSON_PrintUnformatted(json_obj);
    sendto(sock, message, strlen(message), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    printf("Sent: %s\n", message);
    cJSON_free(message);  // Free the allocated message string
}

void process_message(int sock, struct sockaddr_in *client_addr, const char *message) {
    cJSON *json = cJSON_Parse(message);
    if (!json) {
        printf("Invalid JSON received\n");
        return;
    }

    // Handle different stages
    cJSON *op = cJSON_GetObjectItem(json, "op");
    if (op && strcmp(op->valuestring, "start exchange") == 0) {
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "ready");
        respond_json(sock, *client_addr, response);
        cJSON_Delete(response);
    } else if (op && strcmp(op->valuestring, "endconversation") == 0) {
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "ready");
        respond_json(sock, *client_addr, response);
        cJSON_Delete(response);
    } else if (cJSON_HasObjectItem(json, "content")) {
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "received");
        respond_json(sock, *client_addr, response);
    }

    cJSON_Delete(json);
}

int main() {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed\n");
        return -1;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Bind failed\n");
        close(sock);
        return -1;
    }

    printf("UDP server listening on port %d...\n", PORT);

    while (1) {
        // Receive data from client
        int recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';  // Null-terminate the received string
            printf("Received: %s\n", buffer);
            process_message(sock, &client_addr, buffer);
        }
    }

    // Close socket
    close(sock);
    return 0;
}

