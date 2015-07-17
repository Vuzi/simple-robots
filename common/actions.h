/*
 * File actions.h
 * ---------------------------------------------
 * Action handler headers
 * 
 */

#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

// -- Types
/**
 * Handler option
 */
struct command_action {
	char* option;
	void (*action)(int, char**);
};

// -- Prototypes
void handle_action(char* command, const struct command_action *options);

#endif
