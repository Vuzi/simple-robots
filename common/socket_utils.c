/*
 * File soket_utils.c
 * ---------------------------------------------
 * Collection of functions using sockets
 * 
 */

#include "socket_utils.h"

/**
 * Read message from socket
 * 
 * @param  sock   Socket descriptor
 * @param  buffer Buffer where to read
 * @param  max    Max size
 * @return        0 on success, otherwise the error code
 */
int read_msg(int sock, char* buffer, size_t max) {
    int n;
    
    // Read from socket
    if((n = read(sock, buffer, max - 1)) <= 0) {
        show_error("[x] Reponse failed : %s", strerror(errno));
        return errno;
    }
    buffer[n] = '\0';
    
    return 0;
}

/**
 * Send message to socket
 * 
 * @param  sock   Socket descriptor
 * @param  format Message to send
 * @param  ...    Formated values
 * @return        0 on success, otherwise the error code
 */
int send_msg(int sock, const char* format, ...) {
    char buf[NET_BUFFER_SIZE];
    va_list args;
    size_t n;
    
    // Write into buffer
    va_start(args, format);
    vsnprintf(buf, NET_BUFFER_SIZE, format, args);
    va_end(args);
    
    // Write into socket
    n = strlen(buf);
    if(send(sock, buf, n, MSG_NOSIGNAL) < n) {
        show_error("[x] Send failed : %s", strerror(errno));
        return errno;
    }
    
    return 0;
}

/**
 * Send file to socket
 * 
 * @param  sock   Socket descriptor
 * @param  f      File to send
 * @return        0 on success, otherwise the error code
 */
int send_file(int sock, FILE* f) {
    char buf[NET_BUFFER_SIZE];
    int n;
    
    while((n = fread(buf, 1, NET_BUFFER_SIZE, f))) {
        show_msg("[i] Read %d bytes\n", n);
            
        int size = n;
        
        if((n = send(sock, buf, size , MSG_MORE|MSG_NOSIGNAL)) <= 0 || size != n) {
            show_error("[x] Send failed : %s", strerror(errno));
            return errno;
        }
    }
    
    if(!feof(f)) {
        show_error("[x] Read failed : %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

