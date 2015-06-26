#include "robot.h"

// Create and init a new robot
robot* robot_new(int sock, const struct sockaddr_in* sock_info) {
    robot* r = malloc(sizeof(robot));
    
    if(r)
        robot_init(r, sock, sock_info);
        
    return r;
}

// Create and init a new robot
void robot_init(robot* r, int sock, const struct sockaddr_in* sock_info) {
    static unsigned int id = 0;
    
    if(!r)
        return;
        
    r->id = id++;
	r->sock = sock;
    if(sock_info)
        r->sock_info = *sock_info; // To test
}
