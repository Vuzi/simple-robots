/*
 * File test_worker.c
 * ---------------------------------------------
 * Tests for the workers implementation
 * 
 */

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
    worker_pool pool1, pool2; 
    
    puts("Case 1 : 10 actions (5s) + worker_join");
    puts("---------------------------------------");
    worker_init(&pool1);
    
    for(; i < ACTION_NB; i++) {
        action_to_do[i].perform = my_action;
        action_to_do[i].args = (void*) i;
        
        worker_add(&pool1, &action_to_do[i]);
        printf("Action #%li added\n", i);
    }
    
    worker_join(&pool1);
    puts("All actions terminated");
    worker_quit(&pool1);
    puts("---------------------------------------");
    
    
    puts("Case 2 : 10 actions (5s) + worker_quit after 1s");
    puts("---------------------------------------");
    worker_init(&pool2);
    
    for(i = 0; i < ACTION_NB; i++) {
        action_to_do[i].perform = my_action;
        action_to_do[i].args = (void*) i;
        
        worker_add(&pool2, &action_to_do[i]);
        printf("Action #%li added\n", i);
    }
    
    worker_quit(&pool2);
    puts("All actions canceled");
    puts("---------------------------------------");
    
    return 0;
}
