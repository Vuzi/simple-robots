/*
 * File actions.c
 * ---------------------------------------------
 * Action handler implementation 
 * 
 */
#include "actions.h"

/**
 * Handle a specified command with the given command and options
 * @param command The command to handle
 * @param options The action to use
 */
void handle_action(char* command, const struct command_action *options) {
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
			if(options[i].action)
				options[i].action(argc - 1, argv + 1);
			return;
		}
		
		i++;
	}
	
	printf("[i] Unkown command : %s\n", argv[0]);
}
