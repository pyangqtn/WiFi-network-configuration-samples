#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

/* Macro definitions */
#define PRE_ASSIGNED_LENGTH 996
#define PRE_PAYLOAD_ADD 64
#define ETHERNET_HEADER 14
#define IP_HEADER 20
#define UDP_HEADER 8
#define WIFI_HEADER 24    /* Adjust as per your wireless environment */
#define TOTAL_OVERHEAD (ETHERNET_HEADER + IP_HEADER + UDP_HEADER + WIFI_HEADER)

#define DEFAULT_SSID "smc"
#define DEFAULT_PASSWORD "netcfg_demo"
#define DEFAULT_TARGET_IP "255.255.255.255"
#define DEFAULT_TARGET_PORT 12345
#define DEFAULT_INTERVAL 50000     /* Default interval in microseconds (1000us = 1ms) */
#define DEFAULT_LOOP 3
#define DEFAULT_FIXLEN 10          /* Adjust length difference */

/* Function to add microsecond delay */
void delay_in_microseconds(unsigned int microseconds)
{
	struct timespec ts;

	ts.tv_sec = microseconds / 1000000;
	ts.tv_nsec = (microseconds % 1000000) * 1000;
	nanosleep(&ts, NULL);
}

/* Function to send UDP broadcast frame */
void send_broadcast(int sock, int length, const char *target_ip, int target_port,
		    unsigned int interval, int fixlen)
{
	int payload_size;
	char *message;
	struct sockaddr_in broadcast_addr;

	payload_size = length + PRE_PAYLOAD_ADD - TOTAL_OVERHEAD - fixlen;
	message = (char *)malloc(payload_size);
	memset(message, 'a', payload_size);

	broadcast_addr.sin_family = AF_INET;
	broadcast_addr.sin_port = htons(target_port);
	broadcast_addr.sin_addr.s_addr = inet_addr(target_ip);

	sendto(sock, message, payload_size, 0, (struct sockaddr *)&broadcast_addr,
	       sizeof(broadcast_addr));
	delay_in_microseconds(interval);  /* Delay using integer interval in microseconds */

	free(message);
}

/* Procedure to send UDP broadcast frames according to the rules */
void broadcast_procedure(int sock, const char *ssid, const char *password,
			 const char *target_ip, int target_port,
			 unsigned int interval, int fixlen)
{
	int i;

	/* Step 1: Send 3 frames with length PRE_ASSIGNED_LENGTH */
	for (i = 0; i < 3; i++)
		send_broadcast(sock, PRE_ASSIGNED_LENGTH, target_ip, target_port,
			       interval, fixlen);

	/* Step 2: Send frames with ASCII values of SSID */
	for (i = 0; i < strlen(ssid); i++) {
		send_broadcast(sock, ssid[i], target_ip, target_port, interval,
			       fixlen);
		printf("Send ssid %d\n", ssid[i]);
	}

	/* Step 3: Send 3 frames with length (PRE_ASSIGNED_LENGTH + 8) */
	for (i = 0; i < 3; i++)
		send_broadcast(sock, PRE_ASSIGNED_LENGTH + 8, target_ip, target_port,
			       interval, fixlen);

	/* Step 4: Send frames with ASCII values of password */
	for (i = 0; i < strlen(password); i++) {
		send_broadcast(sock, password[i], target_ip, target_port, interval,
			       fixlen);
		printf("Send passwd %d\n", password[i]);
	}

	/* Step 5: Send 3 frames with length (PRE_ASSIGNED_LENGTH + 16) */
	for (i = 0; i < 3; i++)
		send_broadcast(sock, PRE_ASSIGNED_LENGTH + 16, target_ip, target_port,
			       interval, fixlen);
}

int main(int argc, char *argv[])
{
	const char *ssid = DEFAULT_SSID;
	const char *password = DEFAULT_PASSWORD;
	const char *bind_ip = NULL;
	const char *target_ip = DEFAULT_TARGET_IP;
	int target_port = DEFAULT_TARGET_PORT;
	unsigned int interval = DEFAULT_INTERVAL;  /* Interval in microseconds */
	int loop = DEFAULT_LOOP;
	int fixlen = DEFAULT_FIXLEN;
	int sock, i;
	int broadcast_enable = 1;

	/* Argument parsing (manual for C) */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--ssid") == 0 && i + 1 < argc) {
			ssid = argv[++i];
		} else if (strcmp(argv[i], "--password") == 0 && i + 1 < argc) {
			password = argv[++i];
		} else if (strcmp(argv[i], "--bind") == 0 && i + 1 < argc) {
			bind_ip = argv[++i];
		} else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
			target_port = atoi(argv[++i]);
		} else if (strcmp(argv[i], "--target-ip") == 0 && i + 1 < argc) {
			target_ip = argv[++i];
		} else if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc) {
			interval = atoi(argv[++i]);  /* Read interval in microseconds */
		} else if (strcmp(argv[i], "--loop") == 0 && i + 1 < argc) {
			loop = atoi(argv[++i]);
		} else if (strcmp(argv[i], "--fixlen") == 0 && i + 1 < argc) {
			fixlen = atoi(argv[++i]);
		}
	}

	/* Create UDP socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("Socket creation failed");
		return 1;
	}

	/* Enable broadcast option */
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable,
		       sizeof(broadcast_enable)) < 0) {
		perror("Error enabling broadcast option");
		return 1;
	}

	/* Bind socket if needed */
	if (bind_ip != NULL) {
		struct sockaddr_in bind_addr;

		bind_addr.sin_family = AF_INET;
		bind_addr.sin_port = 0; /* Auto-assign port */
		bind_addr.sin_addr.s_addr = inet_addr(bind_ip);

		if (bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
			perror("Binding failed");
			return 1;
		}
	}

	/* Execute broadcast procedure for "loop" times */
	for (i = 0; i < loop; i++) {
		printf("Executing loop %d/%d\n", i + 1, loop);
		broadcast_procedure(sock, ssid, password, target_ip, target_port,
				    interval, fixlen);
	}

	printf("Broadcasts sent successfully!\n");

	close(sock);
	return 0;
}
