#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<ncurses.h>

#include "robot.h"
#include "list.h"

#define BUFFER_SIZE 1024

list robots;                 // List of robots
pthread_mutex_t robot_mutex; // Robot list mutex


// -- Prototype
void *connection_handler(void *);
void* server_handler(void* unused);

// -- Handlers
void show_robot(robot *r, void* unused) {
	printw("[>] %i > %s\n", r->id, r->hostname);
}

void show_robots(robot* r, int argc, char* argv[]) {
    pthread_mutex_lock(&robot_mutex);
	printw("[i] Robots connected : \n");
	list_each(&robots, NULL, (void (*)(void *, void *))show_robot);
    pthread_mutex_unlock(&robot_mutex);
	printw("[i] ------------------ \n");
}

// Test handler for foo command
void foo(robot* r, int argc, char* argv[]) {
	printw("[i] foo\n[i] arguments : \n");
	int i = 0;
	
	while(argv[i]) {
		printw("\t'%s'\n", argv[i++]);
	}
}

// Test handler for bar command
void bar(robot* r, int argc, char* argv[]) {
	printw("[i] bar\n[i] arguments : \n");
	int i = 0;
	
	while(argv[i]) {
		printw("\t'%s'\n", argv[i++]);
	}
}

// Handler option
struct option {
	char* option;
	void (*action)(robot*, int, char**);
};

// Handle a specified command with the given command and options
void handle_command(char* command, const struct option *options) {
	int i = 0, in_value = 0, argc = 0;
	char* argv[32] = { NULL };
	
	while(command[i]) {
		if(command[i] == ' ') {
			if(in_value) {
				command[i] = '\0';
				in_value = 0;
			}
		} else {
			if(!in_value) {
				argv[argc++] = command + i;
				in_value = 1;
				
				if(argc >= 32)
					break;
			}
		}
		
		i++;
	}
	
	if(!argc)
		return;
	
	i = 0;
	while(options[i].option) {
		if(strstr(argv[0], options[i].option)) {
			options[i].action(NULL, argc - 1, argv + 1);
			return;
		}
		
		i++;
	}
	
	printw("[i] Unkown command : %s\n", argv[0]);
}

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
int main(void) {
	
	// Init ncurses
	WINDOW *w = ncurses_init();
	int x, y;
	
	// Init robot list
	list_init(&robots);
    pthread_mutex_init(&robot_mutex, NULL);
	
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
		{ "foo", foo },
		{ "bar", bar },
		{ "show", show_robots },
		{ NULL, NULL },
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
				printw("\nUp Arrow");
		        break;
		    case KEY_DOWN: // TODO
				printw("\nDown Arrow");
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
		
		handle_command(buffer, options);
	}
	
	end:
	printw("\n\Exiting Now\n");
	endwin();
	return 0;
}

// Server handler thread
void* server_handler(void* w) {
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
    server.sin_port = htons(8080);
	
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
		char ip_addr[INET_ADDRSTRLEN];
	
	    pthread_mutex_lock(&robot_mutex);
		list_append(&robots, (void*) robot_new(inet_ntop(AF_INET, &(client.sin_addr), ip_addr, INET_ADDRSTRLEN), socket_client_desc, &client));
	    pthread_mutex_unlock(&robot_mutex);

		// Reset the value
		size = (socklen_t) sizeof(client);
	}

    if (socket_client_desc < 0) {
        perror("[x] Accept failed : ");
        return NULL;
    }
	
	return NULL;
}
