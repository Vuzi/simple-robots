#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>

#include <list.h>
#include <workers.h>
#include <actions.h>
#include <macro.h>
#include <socket_utils.h>

#include "server_actions.h"
#include "robot.h"

extern list robots;                 // List of robots
extern pthread_mutex_t robot_mutex; // Robot list mutex

extern worker_pool connection_pool, action_pool; // Pool of workers

#endif