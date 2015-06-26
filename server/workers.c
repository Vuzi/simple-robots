#include "workers.h"

// -- Global values
static char worker_stop = 0;
static pthread_t threads[WORKER_THREAD_NB]; // Array of threads
static list actions;                        // List of actions
static pthread_mutex_t worker_mutex;        // General mutex
static pthread_cond_t worker_cond;          // Thread shared condition
static pthread_cond_t worker_join_cond;     // Actions empty condition

// Worker handler used in each worker thread
static void* worker_handler(void* unused) {
    
    while(1) {
        pthread_mutex_lock(&worker_mutex);
        
        if(actions.length == 0) {
            pthread_cond_signal(&worker_join_cond);
            
            if(!worker_stop) // No actions, wait
                pthread_cond_wait(&worker_cond, &worker_mutex);
        }
        
        // Stop condition : all worker must stop
        if(worker_stop)
            break;
        
        // Release mutex, and get the action to perform
        action* a = (action*) list_pop(&actions);
        pthread_mutex_unlock(&worker_mutex);
        
        if(a) {
            // In some case workers will be notified with
            // no actions to perform, only do if there is an action
            a->perform(a->args);
            free(a); // Also, free the action
        }
    }
    
    pthread_mutex_unlock(&worker_mutex);
    return NULL;
}

// Init the worker system, and start all the worker threads
void worker_init() {
    
    // Init the mutex and condition
    pthread_mutex_init(&worker_mutex, NULL);
    pthread_cond_init(&worker_cond, NULL);
    pthread_cond_init(&worker_join_cond, NULL);
    
    // Launch all the worker threads
    for(int i = 0; i < WORKER_THREAD_NB; i++)
        pthread_create(&threads[i], NULL, worker_handler, NULL);
    
    list_init(&actions);
}

// Add an action to perform
void worker_add(action* a) {
    // Copy element to have full control on its life cycle
    action* copy = malloc(sizeof(action));
    
    if(!copy)
        return;
        
    memcpy(copy, a, sizeof(action));
    
    pthread_mutex_lock(&worker_mutex);
    list_append(&actions, copy);
    pthread_cond_broadcast(&worker_cond);
    pthread_mutex_unlock(&worker_mutex);
}

// Wait for all the work to be termined
void worker_join() {
    pthread_mutex_lock(&worker_mutex);
    pthread_cond_wait(&worker_join_cond, &worker_mutex);
    pthread_mutex_unlock(&worker_mutex);
}

// Stop all the worker threads
void worker_quit() {
    // Lock, set the stop value, and unlock
    pthread_mutex_lock(&worker_mutex);
    worker_stop = 1;
    pthread_cond_broadcast(&worker_cond);
    pthread_mutex_unlock(&worker_mutex);
    
    // Wait for everyone to stop
    for(int i = 0; i < WORKER_THREAD_NB; i++)
        pthread_join(threads[i], NULL);
    
    // Free the list elements
    list_clear(&actions, free);
}
