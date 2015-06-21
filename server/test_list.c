#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void show_value(void* val, void* args) {
	printf("> %s\n", (char*) val);
}

void show_value_clear(void* val) {
	printf("> %s\n", (char*) val);
}

int main(void) {
	char* values[] = {
		"first_element",
		"second_element",
		"third_element",
		"forth_element"
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
	
	printf("size : %u\n", l.length);
	
	list_each(&l, NULL, show_value);
	puts("--------------------------");
	
	list_clear(&l, show_value_clear);
	puts("--------------------------");
	
	return 0;
}