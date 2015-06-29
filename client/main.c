#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

#define VERSION "0.01"
#define NET_BUFFER_SIZE 2048

int port = 8080;
char* addr = "127.0.0.1";

int main(int argc, char** argv) {
	
	if(argc >= 2)
		addr = argv[1];
		
	if(argc >= 3)
		port = atoi(argv[2]);
		
	printf("[i] Connecting on %s:%d\n", addr, port);
	
	// Get computer name
	char hostname[NET_BUFFER_SIZE];
	hostname[NET_BUFFER_SIZE - 1] = '\0';
	gethostname(hostname, NET_BUFFER_SIZE - 1);
	
	printf("[i] Computer name : %s\n", hostname);
	
	// Create socket
	int socket_desc = socket(AF_INET, SOCK_STREAM, 0), n = 0;
	struct sockaddr_in client;
	char buf[NET_BUFFER_SIZE] = { 0 };
	
	if(socket_desc < 0) {
        perror("[x] Could not create socket");
        return EXIT_FAILURE;
	}
	
	// Prepare server informations
	client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(addr);
    client.sin_port = htons(port);
	
	if(connect(socket_desc, (struct sockaddr*)&client, sizeof(client)) < 0) {
		perror("[x] Bind failed");
		goto error;
	}
	
	// Send "hello" with the hostname
	snprintf(buf, NET_BUFFER_SIZE, "hello %s\n", hostname);
	if(send(socket_desc, buf, strlen(buf), MSG_EOR|MSG_NOSIGNAL) <= 0) {
		perror("[x] Send failed");
		goto error;
	}
	
	// Receive "hello" with the id
	read(socket_desc, buf, NET_BUFFER_SIZE - 1);
	
	// Send "ready"
	if(send(socket_desc, "ready\n", 6, MSG_EOR|MSG_NOSIGNAL) <= 0) {
		perror("[x] Send failed");
		goto error;
	}
	
	// Read commands
	while((n = read(socket_desc, buf, NET_BUFFER_SIZE - 1)) > 0) {
		buf[n--] = '\0';
			
		while(n && ( buf[n] == '\n' || buf[n] == '\r' || buf[n] == ' ')) // Trim
			buf[n--] = '\0';
			
		// TODO : handle do command
		
		printf("[i] read : %s\n", buf);
		
		// Send done
		if(send(socket_desc, "done", 4, MSG_EOR|MSG_NOSIGNAL) <= 0) {
			perror("[x] Send failed");
			goto error;
		}
	}
	
	if(n < 0)
		goto error;
		
	puts("[i] The server closed the connection");
	return 0;
	
	error:
	return EXIT_FAILURE;
}

