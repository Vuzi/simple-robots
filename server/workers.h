#ifndef WORKERS_H
#define WORKERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "list.h"

#define WORKER_THREAD_NB 5 // Should be less than 64

#define FLAG_DOWN(flag, pos) (flag &= ~(1 << pos))
#define FLAG_UP(flag, pos) (flag |= (1 << pos))

// -- Types
// Action for the worker thread
struct action {
	void (*perform)(void*);
	void* args;
};

typedef struct action action;

typedef void(*worker_action)(void*);

// -- Prototypes
void worker_init();
void worker_add(action* a);
void worker_join();
void worker_quit();

#endif