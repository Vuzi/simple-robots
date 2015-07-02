#ifndef MACRO_H
#define MACRO_H

#ifdef SERVER
	#define show_error(args...) fprintf(stderr, args);
	#define show_msg(args...) fprintf(stdout, args);
#else
	extern int daemon;
	#define show_error(args...) if(!daemon) fprintf(stderr, args);
	#define show_msg(args...) if(!daemon) fprintf(stdout, args);
#endif

#define NET_BUFFER_SIZE 4096
#define BUFFER_SIZE 1024

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8080

#endif