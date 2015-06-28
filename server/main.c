#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<ncurses.h>

#include "server.h"
#include "robot.h"
#include "list.h"
#include "workers.h"
#include "actions.h"

// TODO : gestion arguments
//        quit
//        download file
//        upload file ?

// TODO : client

int port = 8080;

list robots;                 // List of robots
pthread_mutex_t robot_mutex; // Robot list mutex

worker_pool connection_pool, action_pool; // Pool of workers

// -- Prototype
static void* server_handler(void *unused);
static void action_connect(void *val);


// -- Functions
// Init ncurses
WINDOW* ncurses_init() {
	WINDOW *w = initscr();
	raw();
	keypad(stdscr, TRUE);
	scrollok(w, TRUE);
	noecho();
	
	return w;
}

// Entry point
int main(int argc, char **argv) {
	
	// TODO : args handling
	// help
	// version
	// port
	// verbose
	if(argv[1] != NULL)
		port = atoi(argv[1]);
	
	// Init ncurses
	WINDOW *w = ncurses_init();
	int x, y;
	
	// Init robot list
	list_init(&robots);
    pthread_mutex_init(&robot_mutex, NULL);
	
	// Init workers pools
	worker_init(&connection_pool);
	worker_init(&action_pool);
	
	// Init the server's listener thread
    pthread_t thread;
	if(pthread_create(&thread, NULL, server_handler, NULL) < 0) {
        printw("[x] Could not create server thread : ");
        return 1;
	}

	// Show greetings
	printw("[i] -------------------------------------- \n");
	printw("[i]             SimpleRobot\n");
	printw("[i] -------------------------------------- \n");
	printw("[i] Press escape to exit \n");
	printw("[i] Test commands : foo & bar \n");
	
	// Buffer
	char buffer[BUFFER_SIZE] = { 0 };
	
	// Options (NULL terminated)
	struct option options[] = {
		{ "foo",   action_foo },
		{ "bar",   action_bar },
		{ "show",  action_robots_show },
		{ "close", action_robots_close },
		{ "send",  action_robots_send_cmd },
		{ NULL,    NULL }
	};
	
	while(1) {
		int c = 0, i = 0, imax = 0;
		
		// Show command input
		printw("[>] ");
		
		while((c = getch())) {
			
			// End of input
			if(c == '\n') {
				buffer[i++] = '\0';
				printw("%c", c);
				break;
			}
			
		    switch(c) {
		    case KEY_UP: // TODO
				//printw("\nUp Arrow");
		        break;
		    case KEY_DOWN: // TODO
				//printw("\nDown Arrow");
		        break;
		    case KEY_LEFT: // Move to the left
				if(i > 0 ) {
					i--;
					getyx(w, y, x);
					wmove(w, y, x - 1);
				}
		        break;
		    case KEY_RIGHT: // Move to the right
				if(i < imax) {
					getyx(w, y, x);
					wmove(w, y, x + 1);
					i++;
				}
		        break;
		    case 27: // Escape key
				goto end;
			case KEY_BACKSPACE: // Remove & move to the left
				if(i > 0) {
					getyx(w, y, x);
					wmove(w, y, x - 1);
					delch();
					
					if(i == imax)
						imax--;
					i--;
				}
		        break;
		    default: // Read value
				if(i < BUFFER_SIZE) {
					buffer[i] = (char) c;
					printw("%c", c);
					
					if(i == imax)
						imax++;
					i++;
				}
		    }
		}
		
		if(!strcmp(buffer, "quit"))
			goto end;
			
		handle_command(buffer, options);
	}
	
	end:
	printw("\n\Exiting Now\n");
	
	// Stop ncurses
	endwin();
	
	// Stop all the workers
	worker_quit(&connection_pool);
	worker_quit(&action_pool);
	
	return 0;
}

// Connection handler
static void action_connect(void* val) {
	
	robot *r = (robot*) val;
	
	int n;
	char buf[NET_BUFFER_SIZE] = {0};
	
	// Read the greeting from the client
	if((n = read(r->sock, buf, NET_BUFFER_SIZE - 1)) <= 0)
		goto error;
	buf[n--] = '\0';
	
	while(n && ( buf[n] == '\n' || buf[n] == '\r' || buf[n] == ' ')) // Trim
		buf[n--] = '\0';
	
	// Extract hostname
	if(strncmp("hello ", buf, 6))
		goto error; // Not possible
	strncpy(r->hostname, buf + 6, 512);
	
	// Send ID
	snprintf(buf, NET_BUFFER_SIZE, "hello %u\n", r->id);
	if(send(r->sock, buf, strlen(buf), MSG_EOR|MSG_NOSIGNAL) <= 0)
		goto error;
	
	// Read "ready"
	if((n = read(r->sock, buf, NET_BUFFER_SIZE - 1)) <= 0)
		goto error;
	
	buf[n] = '\0';
	
	if(strncmp("ready", buf, 5))
		goto error; // Not ready
	
	// Robot OK : add to the list
    pthread_mutex_lock(&robot_mutex);
	list_append(&robots, (void*) r);
    pthread_mutex_unlock(&robot_mutex);
	return;
	
	// Robot KO : close & free
	error:
	close(r->sock);
	free(r);
	return;
}

// Server handler thread
static void* server_handler(void* w) {
	// Server startup 
	int socket_desc, socket_client_desc;
	socklen_t size;
	struct sockaddr_in server;
	struct sockaddr_in client;
	
	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	
	if(socket_desc < 0) {
        perror("[x] Could not create socket");
        return NULL;
	}
	
	// Prepare server informations
	server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
	
	// Bind
	if(bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("[x] Bind failed : ");
        return NULL;
	}
	
	// List on the socket
	if(listen(socket_desc, 15) < 0) {
        perror("[x] Listen failed : ");
        return NULL;
	}
	
    // Accept and incoming connection
	size = (socklen_t) sizeof(client);
	while((socket_client_desc = accept(socket_desc, (struct sockaddr*)&client, &size))) {
		action a;
		
		a.perform = action_connect;
		a.args = (void*) robot_new(socket_client_desc, &client);

		worker_add(&connection_pool, &a);

		// Reset the value
		size = (socklen_t) sizeof(client);
	}

    if (socket_client_desc < 0) {
        perror("[x] Accept failed : ");
        return NULL;
    }
	
	return NULL;
}
