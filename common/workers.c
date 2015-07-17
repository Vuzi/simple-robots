/*
 * File workers.c
 * ---------------------------------------------
 * Workers implementation file
 * 
 */

#include "workers.h"

/**
 * Worker internal handler used in each worker thread
 * 
 * @param val Value to pass to the worker
 */
static void* worker_handler(void *val) {
    worker_info *info = (worker_info*) val;
    
    long id = info->id;
    worker_pool *wpool = info->info;
    
    while(1) {
        pthread_mutex_lock(&(wpool->mutex));
        
        if(wpool->actions.length == 0) {
            FLAG_DOWN(wpool->busy_workers, id); // Register as sleeping thread
            
            if(wpool->busy_workers == 0x0)
                pthread_cond_signal(&(wpool->join_cond)); // Send join signal
            
            if(!wpool->stop) // No actions, wait
                pthread_cond_wait(&(wpool->cond), &(wpool->mutex));
        }
        
        // Stop condition : all worker must stop
        if(wpool->stop)
            break;
        
        // Get the action to perform, and release mutex
        action* a = (action*) list_pop(&(wpool->actions));
        FLAG_UP(wpool->busy_workers, id);
        pthread_mutex_unlock(&(wpool->mutex));
        
        if(a) {
            // In some case workers will be notified with
            // no actions to perform, only do if there is an action
            a->perform(a->args);
            free(a); // Also, free the action
        }
    }
    
    pthread_mutex_unlock(&(wpool->mutex));
    return NULL;
}

/**
 * Init the worker system, and start all the worker threads
 * 
 * @param wpool The worker pool to init
 */
void worker_init(worker_pool *wpool) {
    
    // Register all threads as free
    wpool->stop = 0;
    wpool->busy_workers = 0x0;
    
    // Init action list
    list_init(&(wpool->actions));
    
    // Init the mutex and condition
    pthread_mutex_init(&(wpool->mutex), NULL);
    pthread_cond_init(&(wpool->cond), NULL);
    pthread_cond_init(&(wpool->join_cond), NULL);
    
    // Launch all the worker threads
    for(long i = 0; i < WORKER_THREAD_NB; i++) {
        wpool->workers[i].id = i;
        wpool->workers[i].info = wpool;
        
        pthread_create(&(wpool->workers[i].thread), NULL, worker_handler, (void*) &(wpool->workers[i]));
    }
}

/**
 * Add an action to perform. This method won't wait for the completion of the action
 * 
 * @param wpool The worker pool itself
 * @param a     The action to performs on the worker
 */
void worker_add(worker_pool *wpool, action *a) {
    // Copy element to have full control on its life cycle
    action* copy = malloc(sizeof(action));
    
    if(!copy)
        return;
    
    memcpy(copy, a, sizeof(action));
    
    pthread_mutex_lock(&(wpool->mutex));
    list_append(&(wpool->actions), copy);
    pthread_cond_broadcast(&(wpool->cond));
    pthread_mutex_unlock(&(wpool->mutex));
}

/**
 * Wait for all the work to be termined
 * 
 * @param wpool The worker pool itself
 */
void worker_join(worker_pool *wpool) {
    pthread_mutex_lock(&(wpool->mutex));
    if(wpool->busy_workers || wpool->actions.length > 0)
        pthread_cond_wait(&(wpool->join_cond), &(wpool->mutex));
    pthread_mutex_unlock(&(wpool->mutex));
}

/**
 * Stop all the worker threads. Note that workers already performing a task will wait the end
 * of the task to return. Tasks waiting may be cancelled
 * 
 * @param wpool The worker pool itself
 */
void worker_quit(worker_pool *wpool) {
    // Lock, set the stop value, and unlock
    pthread_mutex_lock(&(wpool->mutex));
    wpool->stop = 1;
    pthread_cond_broadcast(&(wpool->cond));
    pthread_mutex_unlock(&(wpool->mutex));
    
    // Wait for everyone to stop
    for(int i = 0; i < WORKER_THREAD_NB; i++)
        pthread_join(wpool->workers[i].thread, NULL);
    
    // Free the list elements
    list_clear(&(wpool->actions), free);
}
