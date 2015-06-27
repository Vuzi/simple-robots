#ifndef SERVER_H
#define SERVER_H

#include<pthread.h>

#include "list.h"

#define BUFFER_SIZE 1024
#define NET_BUFFER_SIZE 8196

extern list robots;                 // List of robots
extern pthread_mutex_t robot_mutex; // Robot list mutex

#endif