#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ncurses.h>
#include <errno.h>

#include "server.h"
#include "robot.h"
#include "workers.h"
#include "actions.h"

// Server specific actions
void action_robots_show(int argc, char **argv);
void action_robots_send_cmd(int argc, char **argv);
void action_robots_close(int argc, char **argv);

#endif