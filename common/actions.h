#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

// Handler option
struct command_action {
	char* option;
	void (*action)(int, char**);
};

void handle_action(char* command, const struct server_option *options);

#endif
