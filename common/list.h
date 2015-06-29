#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>

#define NEXT(i) (i = i->next)

// -- Types
struct node {
	void* data;           // Data
	struct node* next;	  // Next element, or NULL
};

struct list {
	unsigned int length;  // Number of element in the list
	struct node* nodes;   // First node of the list
};

typedef struct list list;
typedef struct node node;

// -- Prototypes
void list_init(list* l);
void list_append(list* l, void* data);
void list_push(list* l, void* data);
void list_each(list* l, void* args, void(*func)(void*, void*));
void* list_find(list* l, void* args, int(*test)(void*, void*));
void list_remove(list* l, void* args, int(*test)(void*, void*), void(*func)(void*));
void* list_pop(list* l);
void* list_pop_last(list* l);
void list_clear(list* l, void(*func)(void*));

#endif