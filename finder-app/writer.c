#include <stdio.h>
#include <fcntl.h>
#include <syslog.h>

int main(int argc, char **argv) {
	FILE *fptr;

	openlog(NULL, 0, LOG_USER);

	if (argc != 3) {
		syslog(LOG_ERR, "This function expects 2 arguments\n");
		return 1;
	}

	char *writefile = argv[1];
	char *writestr = argv[2];

	//syslog(LOG_DEBUG, "writefile is %s\n", writefile);
	//syslog(LOG_DEBUG, "writestr is %s\n", writestr);

	syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile); 


	// Open a file in writing mode
	fptr = fopen(writefile, "w");
	if (fptr == NULL) {
		syslog(LOG_ERR, "Error opening file %s", writefile);
		return 1;

	}

	// Write some text to the file
	int retval = fprintf(fptr, writestr); 
	printf("fprintf returns %d\n", retval);
	if (retval < 0) {
		syslog(LOG_ERR, "Error writing to file %s", writefile);
		return 1;
	}

	// Close the file
	fclose(fptr); 
	}

