#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

	struct thread_data* thread_func_args = (struct thread_data *) thread_param;

	usleep(thread_func_args->wait_to_obtain_ms * 1000);

	int retval = pthread_mutex_lock(thread_func_args->lock);
	if (retval != 0) {
		printf("Could not lock mutex %d %s\n", retval, strerror(retval));
		thread_func_args->thread_complete_success = false;
    	return thread_param;
	}
	printf("try lock succeeded, mutex was not locked\n");	


	thread_func_args->thread_complete_success = true;

	usleep(thread_func_args->wait_to_release_ms * 1000);


	retval = pthread_mutex_unlock(thread_func_args->lock);
	if (retval != 0) {
		printf("Could not unlock mutex %d %s\n", retval, strerror(retval));
		thread_func_args->thread_complete_success = false;
    	return thread_param;
	}

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

	//struct thread_data *thData = malloc(sizeof *thData);

	struct thread_data *thData = malloc(sizeof(struct thread_data));

    if (thData == NULL) {
        printf("Memory not allocated.\n");
        exit(0);
    }

	thData->wait_to_obtain_ms = wait_to_obtain_ms;
	thData->wait_to_release_ms = wait_to_release_ms;
	thData->lock = mutex;


	int retval = pthread_create(thread, NULL, &threadfunc, thData);
	if (retval != 0) {
            printf("can't create thread :[%s]\n", strerror(retval));
			thData->thread_complete_success = false;
			return false;
		}


    printf("After Thread\n");
	thData->thread_complete_success = true;
    return true;
}

