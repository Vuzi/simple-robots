#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "workers.h"

#define ACTION_NB 10

action action_to_do[ACTION_NB];

void my_action(void* val) {
	printf("Action #%li\n", (long)val);
	sleep(5);
	printf("Action #%li terminated !\n", (long)val);
}

int main(void) {
	long i = 0;
	
	worker_init();
	
	for(; i < ACTION_NB; i++) {
		action_to_do[i].perform = my_action;
		action_to_do[i].args = (void*) i;
		
		worker_add(&action_to_do[i]);
		printf("Action #%li added\n", i);
	}
	
	worker_join();
	puts("All actions terminated");
	
	worker_quit();
	
	return 0;
}