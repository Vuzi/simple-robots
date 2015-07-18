/*
 * File robot.c
 * ---------------------------------------------
 * Robot implementation
 * 
 */

#include <robot.h>

/**
 * Create and init a new robot
 * 
 * @param  sock      Socket descriptor
 * @param  sock_info Socket info
 * @return           The created robot
 */
robot* robot_new(int sock, const struct sockaddr_in* sock_info) {
    robot* r = malloc(sizeof(robot));
    
    if(r)
        robot_init(r, sock, sock_info);
        
    return r;
}

/**
 * Init a new robot
 * 
 * @param r          The robot to init
 * @param  sock      Socket descriptor
 * @param  sock_info Socket info
 */
void robot_init(robot* r, int sock, const struct sockaddr_in* sock_info) {
    static unsigned int id = 1;
    
    if(!r)
        return;
        
    r->id = id++;
    r->sock = sock;
    if(sock_info)
        r->sock_info = *sock_info; // To test
}

int robot_search_id(int *id, robot *r) {
    if(r->id == *id)
        return 1;
    return 0;
}
