#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cjson/cJSON.h>

//#define SERVER_IP "192.168.99.93"  // Replace with your server's IP address
//#define SERVER_IP "127.0.0.1"  // Replace with your server's IP address
#define SERVER_IP "10.23.149.5"  // Replace with your server's IP address
#define SERVER_PORT 29527
#define BUFFER_SIZE 1024

void send_json(int sock, struct sockaddr_in server_addr, cJSON *json_obj) {
    char *message = cJSON_PrintUnformatted(json_obj);
    sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Sent: %s\n", message);
    cJSON_free(message);  // Free the allocated message string
}

void receive_json(int sock, struct sockaddr_in *server_addr) {
#if 1 
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(*server_addr);
    int recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)server_addr, &addr_len);
    if (recv_len > 0) {
        buffer[recv_len] = '\0';
        printf("Received: %s\n", buffer);
    }
#endif
}

int main() {
    int sock;
    struct sockaddr_in server_addr;

    // Create UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed\n");
        return -1;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Stage 1: Start exchange
    cJSON *start_json = cJSON_CreateObject();
    cJSON_AddStringToObject(start_json, "op", "start exchange");
    send_json(sock, server_addr, start_json);
    receive_json(sock, &server_addr);  // Expect {"status": "ready"}
    cJSON_Delete(start_json);

    // Stage 2: Send credentials
    cJSON *cred_json = cJSON_CreateObject();
    cJSON_AddStringToObject(cred_json, "content", "cert");
    cJSON_AddStringToObject(cred_json, "username", "text1");
    cJSON_AddStringToObject(cred_json, "password", "text2");
    send_json(sock, server_addr, cred_json);
    receive_json(sock, &server_addr);  // Expect {"status": "received"}
    cJSON_Delete(cred_json);

    // Stage 3: End conversation
    cJSON *end_json = cJSON_CreateObject();
    cJSON_AddStringToObject(end_json, "op", "endconversation");
    send_json(sock, server_addr, end_json);
    receive_json(sock, &server_addr);  // Expect {"status": "ready"}
    cJSON_Delete(end_json);

    // Close socket
    close(sock);
    return 0;
}

