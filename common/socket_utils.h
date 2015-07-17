/*
 * File soket_utils.h
 * ---------------------------------------------
 * Collection of functions using sockets
 * 
 */

#ifndef H_SOCKET_UTILS
#define H_SOCKET_UTILS

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>

#include <macro.h>

// -- Prototypes
int read_msg(int sock, char* buffer, size_t max);
int send_msg(int sock, const char* format, ...);
int send_file(int sock, FILE* f);

#endif
