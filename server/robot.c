#include "robot.h"

// Handler to delete a robot
void static robot_delete(robot* r, void* unused) {
    if(r)
        free(r);
}

// Create and init a new robot
robot* robot_new(unsigned int id, char* hostname, int sock, struct sockaddr_in* sock_info) {
    robot* r = malloc(sizeof(robot));
    
    if(!r)
        return NULL;
        
    r->id = id;
    r->hostname = hostname;
	r->sock = sock;
    if(sock_info)
        r->sock_info = *sock_info; // To test
    r->next = NULL;
	
	return r;
}

// Return the last robot of the list, or NULL
robot* robot_last(robot* iterator) {
    if(!iterator)
        return NULL;
    
    while(iterator->next)
        NEXT(iterator);
        
    return iterator;
}

// Add a robot at the end of the list
robot* robot_add(robot* iterator, robot* new_robot) {
    if(!new_robot)
        return iterator;
    
    if(!iterator) {
		new_robot->id = 1;
        return new_robot;
    } else {
        robot* last = robot_last(iterator);
		new_robot->id = last->id + 1;
        last->next = new_robot;
        return iterator;
    }
}

// Remove a robot in list
robot* robot_remove(robot* iterator, unsigned int id) {
	robot* first = iterator, *previous = iterator;
	
	if(!first)
		return NULL;
	
	// First to delete
	if(first->id == id) {
		robot* next = first->next;
		robot_delete(first, NULL);
		return next;
	}
	
	NEXT(iterator);
	
	while(iterator) {
		if(iterator->id == id) {
			previous->next = iterator->next;
			robot_delete(iterator, NULL);
			return first;
		}
		
		previous = iterator;
		NEXT(iterator);
	}
	
	return first;
}

// Apply a method for each robot
void robot_each(robot* iterator, void* arg, void (*func)(robot*, void*)) {
    while(iterator) {
        robot* next = iterator->next;
        func(iterator, arg);
        iterator = next;
    }
}

// Remove all the robot from the list
robot* robot_clear(robot* iterator) {
    robot_each(iterator, NULL, robot_delete);
    return NULL;
}

void robot_show(robot* r, void* unused) {
	printf("[%i] Hostname : %s\n", r->id, r->hostname);
}

