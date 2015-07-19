/*
 * File server_actions.c
 * ---------------------------------------------
 * Action of the server
 * 
 */

#include "server_actions.h"

static int get_robot_id(unsigned int *id, int argc, char* argv[]);

static void robot_send_cmd_handler(void **values);
static void robot_send_cmd(robot* r, char **argv);
static void robot_recv_file(robot* r, const char *source, const char *dest);
static void robot_show(robot *r, void *unused);
static void robot_error_remove(robot* r);
static void robot_close_stat(robot *r);

// -- Actions
/**
 * Show help
 * 
 * @param argc Number of arguments
 * @param argv Arguments sended
 */
void action_robots_help(int argc, char **argv) {
    printw("[i] Simple Robot Server help : \n");
    printw("[i] -------------------------- \n");
    printw("[i] show {all|id} : \n");
    printw("[i]    Show informations about a specified client (detailed)\n");
    printw("[i]    or about all the connected clients\n");
    printw("[i] send {all|id} {command} : \n");
    printw("[i]    Send a command to a specified client\n");
    printw("[i]    or to all the connected clients\n");
    printw("[i] get {id} {source path} {destination path} : \n");
    printw("[i]    Download a file from the specified client\n");
    printw("[i] close {id|all} : \n");
    printw("[i]    Close the connection with one or all the clients\n");
    printw("[i] -------------------------- \n");
}

/**
 * Show informations about all the robots, or a specified robot
 * 
 * @param argc Number of arguments
 * @param argv Arguments sended
 */
void action_robots_show(int argc, char **argv) {
    
    unsigned int id = 0;
    
    if(argc > 0) {
        if(get_robot_id(&id, argc, argv))
            return; // No ID
    }
    
    pthread_mutex_lock(&robot_mutex);
    if(!id) {
        printw("[i] Robots connected : \n");
        list_each(&robots, NULL, (void (*)(void *, void *))robot_show);
        printw("[i] ------------------ \n");
    } else {
        robot* r = list_find(&robots, &id, (int (*)(void *, void *))robot_search_id);
        if(!r) {
            printw("[x] Error : no robot with id %d found\n", id);
        } else {
            printw("[i] Robot %d : \n", id);
            printw("[i] id : %d \n", r->id);
            printw("[i] hostname : %s \n", r->hostname);
            printw("[i] ------------------ \n");
        }
    }
    pthread_mutex_unlock(&robot_mutex);
}

/**
 * Get a file from the server
 * 
 * @param argc Number of arguments
 * @param argv Arguments sended
 */
void action_robots_rcv_file(int argc, char **argv) {
    unsigned int id = 0;
    
    if(argc < 3) {
        printw("[x] Not enough arguments provided\n");
        printw("[i] Usage : get [id] [source] [destination]\n");
        return;
    }
    
    if(get_robot_id(&id, argc, argv))
        return; // No ID
    
    if(!id) {
        printw("[i] File donwload is not possible for all clients, please specify an ID\n");
    } else {
        pthread_mutex_lock(&robot_mutex);
        robot* r = list_find(&robots, &id, (int (*)(void *, void *))robot_search_id);
        
        if(!r)
            printw("[x] Error : no robot with id %d found\n", id);
        else
            robot_recv_file(r, argv[1], argv[2]);
            
        pthread_mutex_unlock(&robot_mutex);
    }
}

/**
 * Close a specified robot, or all
 * 
 * @param argc Number of arguments
 * @param argv Arguments sended
 */
void action_robots_close(int argc, char **argv) {
    unsigned int id = 0;
    
    if(get_robot_id(&id, argc, argv))
        return; // No ID
    
    pthread_mutex_lock(&robot_mutex);
    if(!id) {
        printw("[i] Closing all robots\n");
        list_each(&robots, NULL, (void (*)(void *, void *))robot_close_stat);
    } else {
        robot* r = list_find(&robots, &id, (int (*)(void *, void *))robot_search_id);
        if(!r) {
            printw("[x] Error : no robot with id %d found\n", id);
        } else {
            printw("[i] Closing robot %s (%d)\n", r->hostname, r->id);
            robot_close_stat(r);
        }
    }
    pthread_mutex_unlock(&robot_mutex);
}

/**
 * Send a specified command to all the robots, or to a specified robot
 * 
 * @param argc Number of arguments
 * @param argv Arguments sended
 */
void action_robots_send_cmd(int argc, char **argv) {
    
    unsigned int id = 0;
    
    if(argc < 2) {
        printw("[x] Error : not enough arguments sended\n");
        return;
    }
    
    if(get_robot_id(&id, argc, argv))
        return; // No ID
    
    printw("[i] Sending command(s)...\n");
    refresh();
            
    pthread_mutex_lock(&robot_mutex);
    if(id) {
        robot* r = list_find(&robots, &id, (int (*)(void *, void *))robot_search_id);
        
        if(!r)
            printw("[x] Error : no robot with id %d found\n", id);
        else
            robot_send_cmd(r, argv + 1);
    } else {
        if(robots.length > 0)
            list_each(&robots, argv + 1, (void (*)(void *, void *))robot_send_cmd);
        else
            printw("[x] Error : no robots connected\n");
    }
    pthread_mutex_unlock(&robot_mutex);
    worker_join(&action_pool);
}

// -- Handlers
/**
 * Get from the first argument the ID. Return 0 if the ID is valid, 1 otherwise
 * @param  id   [description]
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
static int get_robot_id(unsigned int *id, int argc, char* argv[]) {
    
    if(argc >= 1) {
        if(strcmp("all", argv[0])) {
            *id = atoi(argv[0]);
            
            if(!(*id)) {
                printw("[x] Error : Invalid ID\n");
                return 1;
            }
            
            return 0; // Specified
        } else
            return 0; // All
    }
    
    printw("[x] Error : No ID provided\n");
    return 1; // Error
}

/**
 * Close a connection and delete the robot
 * 
 * @param r Robot to close
 */
static void robot_close_stat(robot *r) {
    //  Try to say goodbye
    send_msg(r->sock, "goodbye\n");
    
    // Close & free
    close(r->sock);
    list_remove(&robots, &(r->id), (int (*)(void *, void *))robot_search_id, free);
}

/**
 * Show informations about a given robot
 * 
 * @param r      Robot to display
 * @param unused Not used
 */
static void robot_show(robot *r, void *unused) {
    printw("[>] %i > %s\n", r->id, r->hostname);
}

/**
 * Send the given action to the given robot, using the worker pool
 * 
 * @param r    Send asynchronous command
 * @param argv Command arguments
 */
static void robot_send_cmd(robot* r, char **argv) {
    
    action a;
    void** values = malloc(sizeof(void*) * 2);
    
    values[0] = r;
    values[1] = argv;
    
    a.perform = (worker_action) robot_send_cmd_handler;
    a.args = (void*) values;

    worker_add(&action_pool, &a);
}

/**
 * Send the given action to the given robot
 * 
 * @param values Values to send
 */
static void robot_send_cmd_handler(void **values) {
    robot *r = (robot*) values[0];
    char **argv = (char**) values[1];
    char buf[512] = {0};
    int i = 0;
    
    free(values);
    
    // Send all the part of the arguments
    if(send(r->sock, "do", 2, MSG_MORE|MSG_NOSIGNAL) <= 0)
        goto error;
    
    while(argv[i]) {
        if(send(r->sock, " ", 1, MSG_MORE|MSG_NOSIGNAL) <= 0)
            goto error;
                
        if(send(r->sock, argv[i], strlen(argv[i]), MSG_MORE|MSG_NOSIGNAL) <= 0)
            goto error;
        
        i++;
    }
    
    if(send_msg(r->sock, "\n")) goto error;
    
    // Read the response 'done'
    if(read_msg(r->sock, buf, 512)) goto error;
    
    return;
    
    error:
        robot_error_remove(r);
}

/**
 * Receive a file from a robot
 * 
 * @param r      The robot to receive from
 * @param source Source name
 * @param dest   Destination name
 */
static void robot_recv_file(robot* r, const char *source, const char *dest) {
    
    // Try to open the destination file
    FILE* f = fopen(dest, "wb");
    char buf[NET_BUFFER_SIZE] = {0};
    int n = 0;
    
    if(!f) {
        printw("[x] An error occured with the local file %s : %s\n", r->hostname, r->id, strerror(errno));
        return;
    }
    
    // Send the command
    
    if(send_msg(r->sock, "get %s", source)) goto error;
    
    // Read response
    if(read_msg(r->sock, buf, NET_BUFFER_SIZE)) goto error;
    
    if(strncmp(buf, "got it\n", 7)) {
        // An error occured
        printw("[x] Error with %s (%d) : %s\n", r->hostname, r->id, buf);
        goto end;
    }
    
    int size = atoi(buf + 7);
    
    // Send ok
    if(send_msg(r->sock, "ok\n")) goto error;
    
    
    printw("[i] Downloading a %d bytes file...\n", size);
    
    // Get the file content
    int size_read = 0;
    int to_read = NET_BUFFER_SIZE > size ? size : NET_BUFFER_SIZE;
    
    while((n = read(r->sock, buf, to_read)) > 0) {
        //printw("[i] read %d bytes\n", n);
        
        size_read += n;
        to_read = size - size_read;
        if(to_read > NET_BUFFER_SIZE)
            to_read = NET_BUFFER_SIZE;
        
        if(fwrite(buf, 1, n, f) != n) {
            printw("[x] An error occured with the local file %s : %s\n", dest, strerror(errno));
            goto end;
        }
        
        if(to_read <= 0)
            break;
    }

    if(n < 0) {
        printw("[x] Error with %s (%d) : %s\n", r->hostname, r->id, strerror(errno));
        goto error; // Finished
    }
    
    printw("[i] File dowloaded to %s\n", dest);
    
    end:
        fclose(f);
        return;
        
    error:
        robot_error_remove(r);
        goto end;
}

/**
 * When an error occured with a robot
 * 
 * @param r Robot to remove
 */
static void robot_error_remove(robot* r) {
    printw("[x] An error occured with %s (%d) : %s\n", r->hostname, r->id, strerror(errno));
    refresh();
    
    pthread_mutex_lock(&robot_mutex);
    list_remove(&robots, &(r->id), (int (*)(void *, void *))robot_search_id, free);
    pthread_mutex_unlock(&robot_mutex);
}
