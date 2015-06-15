#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#include "robot.h"

void *connection_handler(void *);

int main(int argc, char* argv[]) {
	
	robot* list = NULL;
	
	list = robot_add(list, robot_new(0, "bonjour", 0,NULL));
	list = robot_add(list, robot_new(0, "bonjour2", 0, NULL));
	list = robot_add(list, robot_new(0, "bonjour3", 0, NULL));
	
	robot_each(list, NULL, robot_show);
	puts("-------------------- delete 1");
	list = robot_remove(list, 1);
	robot_each(list, NULL, robot_show);
	puts("-------------------- delete 3");
	list = robot_remove(list, 3);
	robot_each(list, NULL, robot_show);
	puts("-------------------- delete 2");	
	list = robot_remove(list, 2);
	robot_each(list, NULL, robot_show);
	puts("--------------------");
	
	list = robot_clear(list);
	
	/*
	int socket_desc, socket_client_desc;
	socklen_t size;
	struct sockaddr_in server;
	struct sockaddr_in client;
	
	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	
	if(socket_desc < 0) {
        printf("[x] Could not create socket");
        return 1;
	}
	
	// Prepare server informations
	server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);
	
	// Bind
	if(bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("[x] Bind failed : ");
        return 1;
	}
	
	// List on the socket
	if(listen(socket_desc, 15) < 0) {
        perror("[x] Listen failed : ");
        return 1;
	}
	
    // Accept and incoming connection
    puts("[i] Waiting for incoming connections...");
	
	size = (socklen_t) sizeof(client);
	while((socket_client_desc = accept(socket_desc, (struct sockaddr*)&client, &size))) {
        printf("[i] Connection accepted from %s\n", inet_ntoa(client.sin_addr));
		
		// Start handler thread
        pthread_t thread;
        int* new_sock = malloc(sizeof(socket_client_desc));
        *new_sock = socket_client_desc;
		
		if(pthread_create(&thread, NULL, connection_handler, (void*)new_sock) < 0) {
            perror("[x] Could not create thread : ");
            return 1;
		}
		
		// Reset the value
		size = (socklen_t) sizeof(client);
	}

    if (socket_client_desc < 0) {
        perror("[x] Accept failed : ");
        return 1;
    }*/
	
	return 0;
}

/*
 * This will handle connection for each client
 *
void* connection_handler(void* socket_desc) {
    // Get the socket descriptor
    int sock = *((int*)socket_desc);
	
	char* msg = "Hello world !\n";
	write(sock, msg, strlen(msg) * sizeof(char));
	
	close(sock);
	
	return NULL;
}*/
