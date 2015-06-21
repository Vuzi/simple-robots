#ifndef WORKERS_H
#define WORKERS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"

#define WORKER_THREAD_NB 5

// -- Types
// Action for the worker thread
struct action {
	void (*perform)(void*);
	void* args;
};

typedef struct action action;

// -- Prototypes
void* worker_handler(void* unused);
void worker_init();
void worker_add(action* a);
void worker_join();
void worker_quit();

#endif