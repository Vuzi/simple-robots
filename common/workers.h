/*
 * File workers.h
 * ---------------------------------------------
 * Workers implementation header file
 * 
 */

#ifndef WORKERS_H
#define WORKERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "list.h"

#define WORKER_THREAD_NB 5 // Should be less than 64

// -- Defines
// Flag defines
#define FLAG_DOWN(flag, pos) (flag &= ~(1 << pos))
#define FLAG_UP(flag, pos) (flag |= (1 << pos))

// -- Types
/**
 * Worker info
 */
struct worker_info {
    struct worker_pool *info;  // Info about the pool
    pthread_t thread;          // Info about the thread
    long unsigned id;          // ID
};

/**
 * Pool of workers
 */
struct worker_pool {
    char stop;                      // True if the workers must stop
    struct worker_info workers[WORKER_THREAD_NB]; // Array of threads
    int busy_workers;               // List of busy threads
    list actions;                   // List of actions   
    pthread_mutex_t mutex;          // General mutex
    pthread_cond_t cond;            // Thread shared condition
    pthread_cond_t join_cond;       // Actions empty condition
};

/**
 * Action for the worker thread
 */
struct action {
    void (*perform)(void*);  // Task to perform
    void* args;              // Arguments given to the task
};

typedef struct worker_info worker_info;
typedef struct worker_pool worker_pool;
typedef struct action action;

typedef void(*worker_action)(void*);

// -- Prototypes
void worker_init(worker_pool *wpool);
void worker_add(worker_pool *wpool, action *a);
void worker_join(worker_pool *wpool);
void worker_quit(worker_pool *wpool);

#endif
