#ifndef ROBOT_H
#define ROBOT_H

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define NEXT(i) (i = i->next)

// Robot client structure
struct robot {
    unsigned int id;                // Unique ID
    char hostname[512];             // Hostname
    int sock;                       // Socket descriptor
    struct sockaddr_in sock_info;   // Connection information
};

typedef struct robot robot;

// Prototypes
robot* robot_new(const char* hostname, int sock, const struct sockaddr_in* sock_info);
void robot_init(robot* r, const char* hostname, int sock, const struct sockaddr_in* sock_info);

#endif