#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

#include <actions.h>

// -- Defines
#define VERSION "0.01"
#define NET_BUFFER_SIZE 2048
#define MAX_TRY 5

// -- Prototypes
void action_do(int argc, char** argv);
void action_get(int argc, char** argv);

static struct command_action options[] = {
	{ "do",  action_do },
	{ "get", action_get },
	{ "goodbye",   NULL },
	{ NULL,    NULL }
};

// -- Actions
// Exec the given command
void action_do(int argc, char** argv) {
	int n = fork();
	
	if(n < 0) {
		// Error
        perror("[x] Could not fork");
	} else if(n == 0) {
		execvp(argv[0], argv);
		
		// Error
        perror("[x] Could not exec the command");
		exit(EXIT_FAILURE);
	}
}

// Get a file
void action_get(int argc, char** argv) {
	// TODO
}

// Connect at the specified port to the specified address, using the given hostname as name
int server_connect(int port, char* addr, char* hostname) {
	
	// Create socket
	int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
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
	
	return socket_desc;
	
	error:
	return -1;
}

// Handle the commands send by the server
int server_handler(int sock) {
	
	int n = 0;
	char buf[NET_BUFFER_SIZE] = { 0 };
	
	// Read commands
	while((n = read(sock, buf, NET_BUFFER_SIZE - 1)) > 0) {
		buf[n--] = '\0';
			
		while(n && ( buf[n] == '\n' || buf[n] == '\r' || buf[n] == ' ')) // Trim
			buf[n--] = '\0';
		
		printf("[i] read : %s\n", buf);
		handle_action(buf, options);
		puts("[i] done !");
		
		// Send done
		if(send(sock, "done", 4, MSG_EOR|MSG_NOSIGNAL) <= 0) {
			perror("[x] Send failed");
			goto error;
		}
	}
	
	if(n < 0)
		goto error;
	else
		return 0;
	
	error:
	return -1;
}

// Entry point
int main(int argc, char** argv) {
	
	int port = 8080, sock = 0, i = 0;
	char* addr = "127.0.0.1";

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
	
	connection:
	sock = server_connect(port, addr, hostname);
	
	if(sock <= 0) {
		// If too much tries, go to error
		if(i++ > MAX_TRY)
			goto error;
			
		// Wait and retry
		puts("[x] Connection failed, trying again in 5s ...");
		sleep(5);
		goto connection;
	}
	
	puts("[i] Connected to server !");
	i = 0; // Reset try counter
	
	// Read commands
	if(server_handler(sock) == 0) {
		puts("[i] The server closed the connection");
		return EXIT_SUCCESS;
	} else {
		puts("[x] Connection lost with the server");			
		goto connection;
	}
	
	error:
	return EXIT_FAILURE;
}

