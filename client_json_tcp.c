#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>

#define SERVER_IP "10.23.149.5"  // Replace with your server's IP address
#define SERVER_PORT 29527           // Server's listening port
#define BUFFER_SIZE 1024

// Function to send JSON message
int send_json(int sock, cJSON *json_obj) {
    char *message = cJSON_PrintUnformatted(json_obj);
    if (message == NULL) {
        fprintf(stderr, "Failed to print JSON.\n");
        return -1;
    }

    ssize_t sent_bytes = send(sock, message, strlen(message), 0);
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
cJSON* receive_json(int sock) {
    char buffer[BUFFER_SIZE];
    ssize_t recv_bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (recv_bytes < 0) {
        perror("recv failed");
        return NULL;
    } else if (recv_bytes == 0) {
        printf("Server closed the connection.\n");
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

int main(int argc, char *argv[])
{
#if 0    
	if (argc < 3) {
        fprintf(stderr, "Usage: %s <username> <password>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
#endif
    const char *username = "netcfg_demo";
    const char *password = "12345678";
    int sock = -1;
    struct sockaddr_in server_addr;

    if (argc == 3) {
    	if (argv[1])
    	    username = argv[1];
    	if (argv[2])
    	    password = argv[2];
    }
    // Create TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    // Stage 1: Start Exchange
    cJSON *stage1 = cJSON_CreateObject();
    if (!stage1) {
        fprintf(stderr, "Failed to create JSON object.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    cJSON_AddStringToObject(stage1, "op", "start exchange");
    if (send_json(sock, stage1) < 0) {
        cJSON_Delete(stage1);
        close(sock);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(stage1);

    // Receive Stage 1 response
    cJSON *response1 = receive_json(sock);
    if (response1) {
        cJSON *status = cJSON_GetObjectItem(response1, "status");
        if (cJSON_IsString(status) && strcmp(status->valuestring, "ready") == 0) {
            printf("Stage 1: Server is ready.\n");
        } else {
            printf("Stage 1: Unexpected server response.\n");
        }
        cJSON_Delete(response1);
    } else {
        printf("Stage 1: No response received.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Stage 2: Content Exchange
    cJSON *stage2 = cJSON_CreateObject();
    if (!stage2) {
        fprintf(stderr, "Failed to create JSON object.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    cJSON_AddStringToObject(stage2, "content", "cert");
    cJSON_AddStringToObject(stage2, "ssid", username);
    cJSON_AddStringToObject(stage2, "passwd", password);
    if (send_json(sock, stage2) < 0) {
        cJSON_Delete(stage2);
        close(sock);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(stage2);

    // Receive Stage 2 response
    cJSON *response2 = receive_json(sock);
    if (response2) {
        cJSON *status = cJSON_GetObjectItem(response2, "status");
        if (cJSON_IsString(status) && strcmp(status->valuestring, "received") == 0) {
            printf("Stage 2: Server has received the content.\n");
        } else {
            printf("Stage 2: Unexpected server response.\n");
        }
        cJSON_Delete(response2);
    } else {
        printf("Stage 2: No response received.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Stage 3: End Conversation
    cJSON *stage3 = cJSON_CreateObject();
    if (!stage3) {
        fprintf(stderr, "Failed to create JSON object.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    cJSON_AddStringToObject(stage3, "op", "endconversation");
    if (send_json(sock, stage3) < 0) {
        cJSON_Delete(stage3);
        close(sock);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(stage3);

    // Receive Stage 3 response
    cJSON *response3 = receive_json(sock);
    if (response3) {
        cJSON *status = cJSON_GetObjectItem(response3, "status");
        if (cJSON_IsString(status) && strcmp(status->valuestring, "ready") == 0) {
            printf("Stage 3: Server is ready.\n");
        } else {
            printf("Stage 3: Unexpected server response.\n");
        }
        cJSON_Delete(response3);
    } else {
        printf("Stage 3: No response received.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Close socket
    close(sock);
    printf("Communication with server completed successfully.\n");
    return 0;
}
