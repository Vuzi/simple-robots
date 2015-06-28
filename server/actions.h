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

// Handler option
struct server_option {
	char* option;
	void (*action)(int, char**);
};

void handle_command(char* command, const struct server_option *options);

void action_show_robots(int argc, char* argv[]);
void action_robots_send_cmd(int argc, char *argv[]);
void action_foo(int argc, char* argv[]);
void action_bar(int argc, char* argv[]);

#endif
