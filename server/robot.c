#include "robot.h"

// Create and init a new robot
robot* robot_new(unsigned int id, const char* hostname, int sock, const struct sockaddr_in* sock_info) {
    robot* r = malloc(sizeof(robot));
    
    if(r)
        robot_init(r, id, hostname, sock, sock_info);
        
    return r;
}

// Create and init a new robot
void robot_init(robot* r, unsigned int id, const char* hostname, int sock, const struct sockaddr_in* sock_info) {
    if(!r)
        return;
        
    r->id = id;
    strncpy(r->hostname, hostname, 512);
	r->sock = sock;
    if(sock_info)
        r->sock_info = *sock_info; // To test
}
