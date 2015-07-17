/*
 * File client.h
 * ---------------------------------------------
 * Action of the server
 * 
 */

#ifndef CLIENT_H
#define CLIENT_H

#define VERSION "0.01"
#define NET_BUFFER_SIZE 2048
#define MAX_TRY 5

#define show_error(args...) fprintf(stderr, args);
#define show_msg(args...) fprintf(stdout, args);

#endif
