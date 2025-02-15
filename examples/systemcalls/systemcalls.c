#include "stdlib.h"
#include "systemcalls.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
	int	retval = system(cmd);
	if (retval == -1)
		return false;

    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
	printf("Command is : ");
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
		printf("%s ", command[i]);
    }
	printf("\n");
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    //command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
	//printf("\n%s \n%s \n%s\n\n", command[0], command[1], command[2]);

    pid_t pid = fork();

    if (pid == 0) {
    	printf("Child process: PID=%d\n", getpid());

    char *envp[] =
    { 
		0
	};

		int retval = execve(command[0], command, envp);

		/*printf("Before unset HOME is %s\n", getenv("HOME"));
		setenv("HOME", "", 1);
		printf("After unset HOME is %s\n", getenv("HOME"));
		int retval = execv(command[0], command);*/

		if (retval == -1)
    		printf("Something went wrong\n");
			return false;

  }
  else if(pid > 0) {
    printf("Parent process: PID=%d\n", getpid());
	int status;
    waitpid(pid, &status, 0);
	printf("Returned status from child %d\n", status);
	if (status != 0)
		return false;
  }
  else {
    printf("Fork failed!\n");
    return false;
  }


    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;

	printf("Output file is %s\n", outputfile);
	printf("Command is : ");
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
		printf("%s ", command[i]);
    }
	printf("\n");
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

 	int fd = open("outputfile", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd < 0) {
        printf("Error creating output file %s\n", outputfile);
        return false;
    }

    pid_t pid = fork();

    if (pid == 0) {
    	printf("Child process: PID=%d\n", getpid());
       	dup2(fd, STDOUT_FILENO);
        close(fd);


		/*printf("Before unset HOME is %s\n", getenv("HOME"));
		setenv("HOME", "", 1);
		printf("After unset HOME is %s\n", getenv("HOME"));*/


    	/*char *envp[] =
    	{ 
			0
		};*/

		int retval = execv(command[0], command);


		if (retval == -1)
    		printf("Something went wrong\n");
			return false;

  }
  else if(pid > 0) {
    printf("Parent process: PID=%d\n", getpid());
	int status;
    waitpid(pid, &status, 0);
	printf("Returned status from child %d\n", status);
	if (status != 0)
		return false;
  }
  else {
    printf("Fork failed!\n");
    return false;
  }


    va_end(args);

    return true;
}
