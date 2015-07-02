#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

// -- Common includes
#include <macro.h>
#include <actions.h>
#include <socket_utils.h>

// -- Defines
#define VERSION "0.01"
#define MAX_TRY 5

int port = DEFAULT_PORT;
char* addr = DEFAULT_IP;
int daemon = 0;
int socket_val;

// -- Prototypes
void action_do(int argc, char** argv);
void action_get(int argc, char** argv);
static void skeleton_daemon();
void parse_options(int argc, char** argv);
static void show_version();
static void show_informations();
static void show_help();
static void show_usages();

static struct command_action options[] = {
	{ "do",  action_do },
	{ "get", action_get },
	{ "goodbye",   NULL },
	{ NULL,    NULL }
};

// -- Actions
// Exec the given command
void action_do(int argc, char** argv) {
	int n = fork();
	
	if(n < 0) {
		// Error
		if(!daemon) perror("[x] Could not fork");
	} else if(n == 0) {
		execvp(argv[0], argv);
		
		// Error
		if(!daemon) perror("[x] Could not exec the command");
		exit(EXIT_FAILURE);
	} else
		send_msg(socket_val, "done"); // Send done
}

// Get a file
void action_get(int argc, char** argv) {
	
	char buf[NET_BUFFER_SIZE] = {0};
	FILE* f = NULL;
	
	if(argc < 1) {
		send_msg(socket_val, "error : no file specified\n");
		goto error;
	}
	
	if(!(f = fopen(argv[0], "rb"))) {
		if(!daemon) perror("[x] Could not open file");
		
		// File could not be opened, send error
		send_msg(socket_val, "error : %s", strerror(errno));
		goto error;
	}
	
	// Get file size
	fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	// Send "ready"
	if(send_msg(socket_val, "got it\n%zu\n", size)) goto error;
	
	// Read response from server
	if(read_msg(socket_val, buf, NET_BUFFER_SIZE)) goto error;

	if(strncmp("ok", buf, 2)) {
		// Not OK
		if(!daemon) puts("[x] Server refused file");
		goto error;
	}
	
	// Send the file content
	send_file(socket_val, f);
	
	error:
	return;
}

// Connect at the specified port to the specified address, using the given hostname as name
int server_connect(int port, char* addr, char* hostname) {
	
	// Create socket
	int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in client;
	char buf[NET_BUFFER_SIZE] = { 0 };
	
	if(socket_desc < 0) {
		if(!daemon)
		   perror("[x] Could not create socket");
		   
        return EXIT_FAILURE;
	}
	
	// Prepare server informations
	client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(addr);
    client.sin_port = htons(port);
	
	if(connect(socket_desc, (struct sockaddr*)&client, sizeof(client)) < 0) {
		if(!daemon)
			perror("[x] Bind failed");
			
		goto error;
	}
	
	// Send "hello" with the hostname
	if(send_msg(socket_desc, "hello %s\n", hostname)) goto error;
	
	// Receive "hello" with the id (ignore)
	if(read_msg(socket_desc, buf, NET_BUFFER_SIZE)) goto error;
	
	// Send "ready"
	if(send_msg(socket_desc, "ready\n")) goto error;
	
	return socket_desc;
	
	error:
	return -1;
}

// Handle the commands send by the server
int server_handler(int sock) {
	
	int n = 0;
	char buf[NET_BUFFER_SIZE] = { 0 };
	
	// Read commands
	while((n = read(sock, buf, NET_BUFFER_SIZE - 1)) > 0) {
		buf[n--] = '\0';
			
		while(n && ( buf[n] == '\n' || buf[n] == '\r' || buf[n] == ' ')) // Trim
			buf[n--] = '\0';
		
		if(!daemon)
			printf("[i] read : %s\n", buf);
			
		handle_action(buf, options);
		
		if(!daemon)
			puts("[i] done !");
	}
	
	if(n < 0)
		goto error;
	else
		return 0;
	
	error:
	return -1;
}

// Entry point
int main(int argc, char** argv) {
	int sock = 0, i = 0;
	
	// Arguments handling with getopt_long
	parse_options(argc, argv);
	
	printf("[i] Connecting on %s:%d\n", addr, port);
	
	// Get computer name
	char hostname[NET_BUFFER_SIZE];
	hostname[NET_BUFFER_SIZE - 1] = '\0';
	gethostname(hostname, NET_BUFFER_SIZE - 1);
	
	if (daemon){
		printf("[i] Started in daemon mode.\n");
		skeleton_daemon();
	} else {
		printf("[i] Computer name : %s\n", hostname);
	}
	
	connection:
	socket_val = sock = server_connect(port, addr, hostname);
	
	if(sock <= 0) {
		// If too much tries, go to error
		if(i++ > MAX_TRY)
			goto error;
			
		// Wait and retry
		if(!daemon)
			puts("[x] Connection failed, trying again in 5s ...");
		sleep(5);
		goto connection;
	}
	
	if(!daemon)
		puts("[i] Connected to server !");
	i = 0; // Reset try counter
	
	// Read commands
	if(server_handler(sock) == 0) {
		if(!daemon)
			puts("[i] The server closed the connection");
			
		return EXIT_SUCCESS;
	} else {
		if(!daemon)
			puts("[x] Connection lost with the server");
					
		goto connection;
	}
	
	error:
	return EXIT_FAILURE;
}

// Handle the daemon creation
static void skeleton_daemon(){
    pid_t pid;

    // Fork off the parent process
    pid = fork();

    // An error occurred
    if (pid < 0) {
		perror("[x] Impossible to fork");
        exit(EXIT_FAILURE);
	}

    // Success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // On success: The child process becomes session leader
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    // Catch, ignore and handle signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // Fork off for the second time
    pid = fork();

    // An error occurred
    if (pid < 0)
        exit(EXIT_FAILURE);

    // Success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);
}

// Handle the option parsing with getopt_long
void parse_options(int argc, char** argv){
	int opt = 0;
	
	// Specify the expected options
    static struct option long_options[] = {        
        {"version",		no_argument,		NULL,  	'v' },
        {"informations",no_argument, 		NULL,	'i' },
		{"help",		no_argument,       	NULL,  	'h' },
		{"daemon",		no_argument,       	NULL,  	'd' },
        {"port",		required_argument, 	NULL,  	'p' },
		{"address",		required_argument, 	NULL,  	'a' },
        {NULL,          0,                  NULL,   0   }
    };
		
	int long_index = 0;
    while ((opt = getopt_long(argc, argv, "vihdp:a:", long_options, &long_index	)) != -1) {
        switch (opt) {
             case 'v' : show_version();
                 exit(EXIT_SUCCESS);
             case 'i' : show_informations();
                 exit(EXIT_SUCCESS);
             case 'h' : show_help();
                 exit(EXIT_SUCCESS);
			 case 'd' : daemon = 1;
			 	 break;
             case 'p' : port = atoi(optarg);
                 break;
			 case 'a' : addr = optarg;
                 break;
             default: show_usages();
			 	exit(EXIT_FAILURE);	
        }
    }
}

// Print the server version
static void show_version(){
	printf("robot_client v%s\nCompiled at %s %s\n", VERSION, __TIME__, __DATE__);
	printf("\nWritten by Vuzi & Dimitri\n");
	printf("See https://github.com/Vuzi/SimpleRobots for more informations\n");
}

// Print the server informations
static void show_informations(){
	printf("No informations yet.");
}

// Display help about the server commands
static void show_help(){
	puts("robot_client\n");
	
	puts("usage : ");
	puts("\trobot_client [-v|--version] [-i|--information] [-h|--help] [-p|--port {port_number}] [-a|--address {address_name}]");
	
	puts("\nOptions : ");
	puts("--version\t-v");
	puts("\tShow the client version.\n");
	
	puts("--informations\t-i");	
	puts("\tShow the client informations.\n");
	
	puts("--help\t-h");
	puts("\tDisplay help.\n");
	
	puts("--daemon\t-d");
	puts("\tRun in daemon mode.\n");
	
	puts("--port\t-p\t{port}");
	puts("\tAllow to set the port to use. Set by default to '8080'.\n");
	
	puts("--address\t-a\t{address}");
	puts("\tAllow to set the address to use. Set by default to '127.0.0.1'.\n");
	
	show_version();
}

// Print usages of the application
static void show_usages(){
	puts("usage : ");
	puts("\trobot_client [-v|--version] [-i|--information] [-h|--help] [-d|--daemon] [-p|--port {port_number}] [-a|--address {address_name}]");
	puts("\tTry --help for more informations");
}

