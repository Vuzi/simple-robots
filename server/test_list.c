#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

void show_value(void* val, void* args) {
	printf("> %s\n", (char*) val);
}

void show_value_clear(void* val) {
	printf("> %s\n", (char*) val);
}

int is_pair(void* args, void* val) {
	if(!strcmp("first_element", (char*)val))
		return 0;
	else if(!strcmp("second_element", (char*)val))
		return 1;
	else if(!strcmp("third_element", (char*)val))
		return 0;
	else if(!strcmp("fourth_element", (char*)val))
		return 1;
	else if(!strcmp("fifth_element", (char*)val))
		return 0;
	else if(!strcmp("sixth_element", (char*)val))
		return 1;
	else
		return 0;
}

int main(void) {
	char* values[] = {
		"first_element",
		"second_element",
		"third_element",
		"fourth_element",
		"fifth_element",
		"sixth_element"
	};
	
	list l;
	
	list_init(&l);
	printf("size : %u\n", l.length);
	
	list_append(&l, values[0]); // first
	list_append(&l, values[1]); // first, second
	
	list_pop(&l); // second
	printf("size : %u\n", l.length);
	
	list_push(&l, values[2]); // third, second
	list_push(&l, values[3]); // forth, third, second
	list_push(&l, values[4]); // fifth, forth, third, second
	list_push(&l, values[5]); // sixth, fifth, forth, third, second
	
	printf("size : %u\n", l.length);
	
	list_each(&l, NULL, show_value);
	puts("--------------------------");
	list_remove(&l, NULL, is_pair, NULL);
	list_each(&l, NULL, show_value);
	puts("--------------------------");
	
	list_clear(&l, show_value_clear);
	puts("--------------------------");
	
	return 0;
}
