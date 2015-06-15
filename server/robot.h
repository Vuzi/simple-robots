#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define NEXT(i) (i = i->next)

// Robot client structure
struct robot {
    unsigned int id;                // Unique ID
    char* hostname;                 // Hostname
    int sock;                       // Socket descriptor
    struct sockaddr_in sock_info;   // Connection information
    
    struct robot* next;             // Next robot
};

typedef struct robot robot;

// Prototypes
robot* robot_last(robot* iterator);
robot* robot_add(robot* iterator, robot* new_robot);
robot* robot_remove(robot* iterator, unsigned int id);
void robot_each(robot* iterator, void* arg, void (*func)(robot*, void*));
robot* robot_new(unsigned int id, char* hostname, int sock, struct sockaddr_in* sock_info);
robot* robot_clear(robot* iterator);
void robot_show(robot* r, void* unused);
