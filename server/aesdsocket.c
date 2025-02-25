/*
   C socket server example
 */

#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include <syslog.h>


//#include <memory.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include "fcntl.h"
#include "stdbool.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>


#define PORT 9000
#define MAX_BUFFER_SIZE 65536 


int daemonise() {
    pid_t pid;

    // Step 1: Fork the parent process
    if ((pid = fork()) < 0) {
        return -1; // Fork failed
    } else if (pid != 0) {
        exit(0); // Parent process exits
    }

    // Step 2: Create a new session
    if (setsid() < 0) {
        return -1; // Failed to create a new session
    }

    // Step 3: Change working directory to root
    if (chdir("/") < 0) {
        return -1; // Failed to change directory
    }

    // Step 4: Reset file permissions
    umask(0);

    // Step 5: Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Step 6: Redirect standard input, output, and error to /dev/null
    open("/dev/null", O_RDONLY); // Redirect standard input
    open("/dev/null", O_WRONLY); // Redirect standard output
    open("/dev/null", O_WRONLY); // Redirect standard error

    return 0; // Daemon initialized successfully
}




int closedown = 0;

bool caught_sigint = false;
bool caught_sigterm = false;


void	gracefulclose(int client_sock, int socket_desc, FILE *fptr, char *outfilename) {

	//int retVal;

	printf("Inside gracefulclose\n");

	//printf("fptr is now %x\n", fptr);
	/*if (fptr == NULL) {
		printf("fptr is already closed\n");
	}
	printf("Closing fptr\n");
	fclose(fptr);*/


	/*printf("client_sock is now %x\n", client_sock);
	if (client_sock > 0) {
		//int retVal = shutdown(client_sock, SHUT_RDWR);
		//perror("shutdown client");
		//printf("Shutdown Client - retVal is %d\n", retVal);

		retVal = close(client_sock);
		perror("close client");
		printf("Close Client - retVal is %d\n", retVal);
	}

	else {
		printf("client_sock is already closed\n");
	}

	printf("socket_desc is now %x\n", socket_desc);
	if (socket_desc > 0) {
		//int retVal = shutdown(socket_desc, SHUT_RDWR);
		//perror("shutdown socket ");
		//printf("Shutdown socket - retVal is %d\n", retVal);

		retVal = close(socket_desc);
		perror("close server");
		printf("Close socket - retVal is %d\n", retVal);
	}
	else {
		printf("socket_desc is already closed\n");
	}*/


	struct stat   buffer;   
	if (stat(outfilename, &buffer) == 0) {
		printf("File %s exists - will delete it\n", outfilename);
		unlink(outfilename);
	}
	else {
		printf("File %s does not exist \n", outfilename);
	}

	printf("Exiting\n");
	syslog(LOG_ERR, "Caught signal, exiting");
	exit(0);

}

static void signal_handler ( int signal_number )
{
	/**
	 * Save a copy of errno so we can restore it later.  See https://pubs.opengroup.org/onlinepubs/9699919799/
	 * "Operations which obtain the value of errno and operations which assign a value to errno shall be
	 *  async-signal-safe, provided that the signal-catching function saves the value of errno upon entry and
	 *  restores it before it returns."
	 */
	int errno_saved = errno;
	printf("Inside signal handler\n");
	if ( signal_number == SIGINT ) {
		caught_sigint = true;
		closedown = 1;
	} else if ( signal_number == SIGTERM ) {
		caught_sigterm = true;
		closedown = 1;
	}
	errno = errno_saved;
}

int report(char *filename, int sock) {

	FILE *fptr;

	char *dataLine = malloc(MAX_BUFFER_SIZE);
	if (dataLine == NULL) {
		printf("Memory could not be allocated.\n");
		return(-1);
	}

	fptr = fopen(filename, "r");
	if (fptr == NULL) {
		syslog(LOG_ERR, "Error opening file %s", filename);
		return -1;
	}

	while (fgets(dataLine, MAX_BUFFER_SIZE, fptr)) {
		// Print each dataLine to the standard output.
		//printf("sizeof(dataline) is %ld\n", MAX_BUFFER_SIZE);
		printf("Replying with %ld\n", strlen(dataLine));
		syslog(LOG_DEBUG, "Replying with %ld\n", strlen(dataLine));
		write(sock , dataLine , strlen(dataLine));
	}

	fclose(fptr);
	free(dataLine);
	return(0);

}


int main(int argc , char *argv[])
{
	int socket_desc , client_sock ;
	int c , read_size, retVal;
	struct sockaddr_in server , client;
	//char client_message[65535];
	FILE *fptr;



	struct sigaction new_action;
	memset(&new_action,0,sizeof(struct sigaction));
	new_action.sa_handler=signal_handler;
	if( sigaction(SIGTERM, &new_action, NULL) != 0 ) {
	  printf("Error %d (%s) registering for SIGTERM",errno,strerror(errno));
	}
	if( sigaction(SIGINT, &new_action, NULL) ) {
		printf("Error %d (%s) registering for SIGINT",errno,strerror(errno));
	}





	char *client_message = malloc(MAX_BUFFER_SIZE);
	if (client_message == NULL) {
		printf("Memory could not be allocated.\n");
		return(-1);
	}

	openlog(NULL, 0, LOG_USER);
	//syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

	char *outfilename = "/var/tmp/aesdsocketdata";

	// Open a file in append mode
	fptr = fopen(outfilename, "w");
	if (fptr == NULL) {
		syslog(LOG_ERR, "Error opening file %s", outfilename);
		return 1;
	}
	fclose(fptr);

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
		//syslog(LOG_ERR, "Error opening file %s", writefile);
		exit(-1);
	}
	printf("Socket created\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );

	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		printf("bind failed. Error");
		exit(-1);
	}
	printf("bind successful\n");


	if (argc > 1) {
		printf("argv[1] is %s\n", argv[1]);
		if (strcmp(argv[1], "-d") == 0) {
			printf("Running as a daemon\n");
			if (daemonise() < 0) {
				exit(-1);
			}
		}
	}



	while (1)
	{

		//Listen
		listen(socket_desc , 3);

		//Accept and incoming connection
		puts("Waiting for incoming connections...");
		c = sizeof(struct sockaddr_in);

		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (errno == EINTR) {
			perror("accept failed");
			printf("Special handling required for this\n");
			gracefulclose(client_sock, socket_desc, fptr, outfilename);

		}

		else if (client_sock < 0)
		{
			perror("accept failed");
			return 1;
		}
		puts("Connection accepted");
		syslog(LOG_DEBUG, "Accepted connection from %s", inet_ntoa(client.sin_addr));
		printf("Accepted connection from  IP %s\n", inet_ntoa(client.sin_addr));


		//Receive a message from client
		while( (read_size = recv(client_sock , client_message , MAX_BUFFER_SIZE , 0)) > 0 )
		{


			// Open a file in append mode
			fptr = fopen(outfilename, "a");
			if (fptr == NULL) {
				syslog(LOG_ERR, "Error opening file %s", outfilename);
				return 1;
			}


			//printf("Received message %s\n", client_message);
			printf("Message length is  %d\n", read_size);
			syslog(LOG_DEBUG, "Message length is  %d\n", read_size);
			printf("Last char is   %x\n", client_message[read_size - 1]);

			//Make sure the message is null terminated
			client_message[read_size] = 0;

			//write(client_sock , client_message , read_size);


			// Write some text to the file
			int retval = fprintf(fptr, client_message); 
			syslog(LOG_DEBUG, "client_message length is %ld\n", strlen(client_message));
			printf("fprintf returns %d\n", retval);
			syslog(LOG_DEBUG, "fprintf returns %d\n", retval);
			if (retval < 0) {
				syslog(LOG_ERR, "Error writing to file %s", outfilename);
				return 1;
			}

			// Close the file
			fclose(fptr); 

			if (client_message[read_size - 1] == '\n') {
				printf("This is a complete message - should reply back\n");
				syslog(LOG_ERR, "This is a complete message - should reply back\n");
				report(outfilename, client_sock);
			}
			else {
				printf("This was part of a message - waiting for the rest\n");
				syslog(LOG_ERR, "This was part of a message - waiting for the rest\n");
			}


		}

		if(read_size == 0)
		{
			puts("Client disconnected");
			fflush(stdout);
		}
		else if(read_size == -1)
		{
			perror("recv failed");
		}
		printf("Closing the connection\n");

		retVal = shutdown(client_sock, SHUT_RDWR);
		perror("shutdown client");
		printf("Shutdown Client - retVal is %d\n", retVal);

		retVal = close(client_sock);
		perror("close client");
		printf("Close Client - retVal is %d\n", retVal);

		syslog(LOG_DEBUG, "Closed connection from %s", inet_ntoa(client.sin_addr));

		if (closedown == 1) {
			printf("Received a request to close down\n");
		}

	}
	retVal = shutdown(socket_desc, SHUT_RDWR);
	perror("shutdown socket ");
	printf("Shutdown socket - retVal is %d\n", retVal);

	retVal = close(socket_desc);
	perror("close  server");
	printf("Close socket - retVal is %d\n", retVal);

	/*printf("Waiting 10 seconds\n");
	  sleep(10);
	  printf("Ready to go again\n");*/



	return 0;
}




