#include "actions.h"

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
		if(!strcmp(argv[0], options[i].option)) {
			options[i].action(argc - 1, argv + 1);
			return;
		}
		
		i++;
	}
	
	printw("[i] Unkown command : %s\n", argv[0]);
}

// -- Handlers
static void show_robot(robot *r, void *unused) {
	printw("[>] %i > %s\n", r->id, r->hostname);
}

static int robot_search_id(int *id, robot *r) {
	if(r->id == *id)
		return 1;
	return 0;
}

void action_show_robots(int argc, char* argv[]) {
	
	unsigned int id = 0;
	
	if(argc >= 1) {
		if(strcmp("all", argv[0])) {
			id = atoi(argv[0]);
			
			if(!id) {
				printw("[x] Error : Invalid ID\n");
				return;
			}
		}
	}
	
    pthread_mutex_lock(&robot_mutex);
	if(!id) {
		printw("[i] Robots connected : \n");
		list_each(&robots, NULL, (void (*)(void *, void *))show_robot);
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

static void robot_send_cmd_handler(void **values) {
	robot *r = (robot*) values[0];
	char **argv = (char**) values[1];
	char buf[512] = {0};
	int i = 0;
	
	free(values);
	
	// Send all the part of the arguments
	if(!write(r->sock, "do", 2))
		goto error;
	
	while(argv[i]) {
		printw(">>> %s \n", argv[i]);
		
		if(!write(r->sock, " ", 1))
			goto error;
				
		if(!write(r->sock, argv[i], strlen(argv[i])))
			goto error;
		
		i++;
	}
	
	if(!write(r->sock, "\n", 1))
		goto error;
	
	// Read the response 'done'
	if(!read(r->sock, buf, 512))
		goto error;
		
	printf(">%s", buf);
	return;
		
	error:
		printw("\nAn error occured :(\n");
		perror("Error : ");
		// TODO
}

static void robot_send_cmd(robot* r, char **argv) {
	action a;
	void** values = malloc(sizeof(void*) * 2);
	
	values[0] = r;
	values[1] = argv;
	
	a.perform = (worker_action) robot_send_cmd_handler;
	a.args = (void*) values;

	worker_add(&a);
}

void action_robots_send_cmd(int argc, char **argv) {
	
	unsigned int id = 0;
	
	if(argc < 2) {
		printw("[x] Error : not enough arguments sended\n");
		return;
	}
	
	if(strcmp("all", argv[0])) {
		id = atoi(argv[0]);
		
		if(!id) {
			printw("[x] Error : Invalid ID\n");
			return;
		}
	}
			
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
	worker_join();
	
}

// Test handler for foo command
void action_foo(int argc, char **argv) {
	printw("[i] foo\n[i] arguments : \n");
	int i = 0;
	
	while(argv[i]) {
		printw("\t'%s'\n", argv[i++]);
	}
}

// Test handler for bar command
void action_bar(int argc, char **argv) {
	printw("[i] bar\n[i] arguments : \n");
	int i = 0;
	
	while(argv[i]) {
		printw("\t'%s'\n", argv[i++]);
	}
}