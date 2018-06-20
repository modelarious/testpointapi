/*XXXXXXXXX must be initialized before any forking is done or else the other processes won't be able to access it */
/* should I dup2() the stderr and stdout file descriptors incase someone closes them as part of a test case? */

/* anything but ERROR ends testpoint. Error is not a test state, unres, fail and pass are. It is up to you to 
call pointfail after errors.  Recommended: create functions to run each testpoint.  if any error codes
are generated by any functions, then pointerrormsg() and return EXIT_FAILURE.  The code looking for the return
value should then fail the test with pointfail() */

/* considered creating a state machine that enforced the correct ordering of states 
      |-> unres ->|
test  |-> fail  ->|-> test -> etc...
      |-> pass  ->|

where the error state can be entered at any time as many times as needed but will not move testing forward.
But this would have added a ton of unneeded overhead.
*/

/* must call before forking */


/* TODO: make testend() formatted as well
consider state machine to enforce ordering of api calls
*/

#ifndef EOK
#define EOK 0
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define SHMEM_PATH "/tmp/testpoint"

/* check if testInfo is uninitialized, if that is the case, 
print a message and return a failure */
#define return_fail_on_no_shared_memory {\
if (testInfo == NULL) { \
	fprintf(stderr, "ERROR:\ttestInfo not initialized. "\
    "Make sure to call teststart()\n"); \
    return EXIT_FAILURE; \
}}

/* will expand into a function that uses va_list 
to pass in a formatted string to __printf */
#define point_generic(state, message) {\
	int rc;	\
	va_list arg; \
	/* points to the last arg before the elipsis */ \
	va_start (arg, str); \
	/* pass the argument list into __printf */ \
	rc = __printf(arg, state, message, str); \
	va_end(arg); \
	return rc; \
}
/* example of above macro:

int pointpass(char * str, ...) {
	point_generic(PASS, "PASS");
}

will become:

int pointpass(char * str, ...) {
	int rc;
	va_list arg;
	// points to the last arg before the elipsis
	va_start (arg, str);
	// pass the argument list into __printf
	rc = __printf(arg, PASS, "PASS", str);
	va_end(arg);
	return rc;
}
*/

typedef enum {
	PASS=0,
	FAIL,
	UNRES,
	ERROR,
	POINT,
	/* add states here */
	
	NUMBER_OF_STATES
} test_state_t;

typedef struct {
	long testResults[NUMBER_OF_STATES]; /* array indexed by test_state_t */
	pthread_mutex_t mutexes[NUMBER_OF_STATES]; /* lock to modify testResults */
	pthread_mutex_t exclusivePrint; /* lock so only one test point message can occur at a time */
	FILE * filestream; /* buffer to print to, makes testing this code a lot easier and 
	                      allows output to be directed to a file if stdout and stderr need 
	                      to be closed as part of a test case*/
} testInfo_t;

/* in shared memory */
testInfo_t * testInfo = NULL;

/* print out a descriptive error message */
void error_print(char *funcName, int rc) {
	fprintf(stderr, "%s() failed in point api: %d %s\n", funcName,
		rc, strerror(rc));
	fflush(stderr);
}

void print_test_point_id() {}
	

/* lock associated mutex, increase test point, unlock associated mutex */
int increment_state(test_state_t state) {
	int rc;
	/* if shared memory is not initialized, fail */
	return_fail_on_no_shared_memory;

	/* increase occurrence of passed in state */
	if ((rc = pthread_mutex_lock(&testInfo->mutexes[state])) != EOK) {
		error_print("pthread_mutex_lock", rc);
		return EXIT_FAILURE;
	}
	
	testInfo->testResults[state]++;

	if ((rc = pthread_mutex_unlock(&testInfo->mutexes[state])) != EOK) {
		error_print("pthread_mutex_unlock", rc);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;

}

/* print the formatted output passed into a test point function,
 * increment the occurrence count of the passed in "state" */
 /* maybe store every message in a database? XXX */
int __printf (va_list arg, test_state_t state, char *apiMessage, char * str)
{
	int rc;
	
	/* if shared memory is not initialized, fail */
	return_fail_on_no_shared_memory;
	
	/* to ensure only one message is ever printed at a time,
	 * lock the exclusive print mutex */
	if ((rc = pthread_mutex_lock(&testInfo->exclusivePrint)) != EOK) {
		error_print("pthread_mutex_lock", rc);
		return EXIT_FAILURE;
	}
	
	/* print: 
		apiMessage ":\t" str "\n" 
	*/
	fprintf(testInfo->filestream, "%s:\t", apiMessage);
	vfprintf(testInfo->filestream, str, arg);
	fprintf(testInfo->filestream, "\n");
	va_end (arg);
	fflush(testInfo->filestream);
    
    /* done with this mutex */
	if ((rc = pthread_mutex_unlock(&testInfo->exclusivePrint)) != EOK) {
		error_print("pthread_mutex_unlock", rc);
		return EXIT_FAILURE;
	}
	
	/* state == POINT indicates we are just printing information and exiting */
	if (state == POINT) {
		return EXIT_SUCCESS;
	}
	
	/* increment the passed in state */
	if (increment_state(state) != EXIT_SUCCESS) {
		return EXIT_FAILURE;
	}
	
	/* if passed in state is not ERROR, 
	increase test point count too */
	if (state != ERROR) {
		return increment_state(POINT);
    }
    
    return EXIT_SUCCESS;

}

/* sets values in shared memory to defaults, initializes mutexes */
int init_testInfo(FILE * filestream) {
	pthread_mutexattr_t attr;
	int rc;
	
	/* setup testInfo */
	/* if testInfo has been initialized, do the following: */
	if (testInfo) {
		memset(testInfo, 0, sizeof(testInfo_t));
		
		/* create attributes */
		if ((rc = pthread_mutexattr_init(&attr)) != EOK) {
			error_print("pthread_mutexattr_init", rc);
			return EXIT_FAILURE;
		}
		
		/* as this will be shared across processes, must set
		 * PTHREAD_PROCESS_SHARED */
		if ((rc = pthread_mutexattr_setpshared(&attr, 
			PTHREAD_PROCESS_SHARED)) != EOK) {
			error_print("pthread_mutexattr_setpshared", rc);
			return EXIT_FAILURE;
		}
		
		/* initialize mutexes */
		for (int i = PASS; i < NUMBER_OF_STATES; i++) {
			if ((rc = pthread_mutex_init(&testInfo->mutexes[i], &attr)) != EOK) {
				error_print("pthread_mutex_init", rc);
				return EXIT_FAILURE;
			}
		}
		
		/* initialize mutex for exclusive printing */
		if ((rc = pthread_mutex_init(&testInfo->exclusivePrint, &attr)) != EOK) {
			error_print("pthread_mutex_init", rc);
			return EXIT_FAILURE;
		}
		
		testInfo->filestream = filestream;
		
		return EXIT_SUCCESS;
	}
	
	/* testInfo was not yet initialized */
	return EXIT_FAILURE;
}
	
/* returns failure on already initialized or successful initialization */
int init_shared_memory() {
	int fd, rc;
	char shmem_path[500];
	
	/* check if testInfo is being initialized again before having called testend() */
	if (testInfo) {
		fprintf(stderr, "testInfo already initialized\n");
		return EXIT_FAILURE;
	}
	
	/* set the shared memory path to ${SHMEM_PATH}_${PID} */
	sprintf(shmem_path, "%s_%d", SHMEM_PATH, getpid());
	
	/* open shared memory object */
	fd = shm_open(shmem_path, 
		O_CREAT| /* create if non existant */
		//O_TRUNC| /* truncate to 0 if already existing */
		O_RDWR, /* open for reading and writing */
		S_IRWXU /* user has read, write and execute */
	);
	
	/* fd should be nonnegative */
	if (fd < 0) {
		error_print("shm_open", errno);
		return EXIT_FAILURE;
	}
	
	/* resize using fd */
	rc = ftruncate(fd, sizeof(testInfo_t)); //size of testInfo_t
	
	/* error handle */
	if (rc != EOK) {
		error_print("ftruncate", errno);
		return EXIT_FAILURE;
	}
	
	/* map shared memory to testInfo */
	testInfo = mmap(NULL, sizeof(testInfo_t), 
		//PROT_EXEC|
		PROT_READ|
		PROT_WRITE, /* read/write */
		MAP_SHARED, /* shared memory */
		fd, 0 );
	
	/* error handle */
	if (testInfo == MAP_FAILED) {
		error_print("mmap", errno);
		return EXIT_FAILURE;
	}
	
	/* done with fd */
	close(fd);
	
	return EXIT_SUCCESS;
}

/* initialize all required data for testing to begin */
int testinit() {
	/* initialize shared memory and mmap it to testInfo pointer */
	if (init_shared_memory() != EXIT_SUCCESS) {
		return EXIT_FAILURE;
	}
	
	/* set default values and initialize mutexes */
	if (init_testInfo(stdout) != EXIT_SUCCESS) {
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
	
/* must be called at the start of testing, before any forking */
int teststart(char * str, ...) {

	/* if successfully initialize shared memory, print start message */
	if (testinit() == EXIT_SUCCESS) {
		point_generic(POINT, "START");
	}
	
	/* if something went wrong initializing shared memory, fail */
	return EXIT_FAILURE;
}

/* generic information to display, the testpointapi version of printf */
int testnote(char * str, ...) {
	point_generic(POINT, "NOTE");
}

/* called at the beginning of each and every test point. Considered making 
a state machine to enforce this, but decided it's up to the programmer.
Made calling this optional, but it is still a good idea. XXX */
int pointstart(char * str, ...) {
	point_generic(POINT, "TEST");
}

/* called to end a test point and declare it a pass.
also increments point counter */
int pointpass(char * str, ...) {
	point_generic(PASS, "PASS");
}

/* called to end a test point and declare it a fail.
also increments point counter */
int pointfail(char * str, ...) {
	point_generic(FAIL, "FAIL");
}

/* called to end a test point and declare it unresolved.
also increments point counter */
int pointunres(char * str, ...) {
	point_generic(UNRES, "UNRES");
}

/* called at any point to display an error message.
DOES NOT increment point counter */
int pointerrormsg(char *str, ...) {
	point_generic(ERROR, "ERROR");
}

/* called at the end of testing procedure to print
out a summary of the points and destruct all shared
memory */
int testend(char * str, ...) {
	char shmem_path[500];
	int rc;
	va_list arg;
	
	/* if shared memory is not initialized, fail */
	return_fail_on_no_shared_memory;

	// points to the last arg before the elipsis
	va_start (arg, str);
	// pass the argument list into __printf
	rc = __printf(arg, POINT, "END", str);
	va_end(arg);
	
	printf("Points: %ld, Pass: %ld, Fail: %ld, Unresolved: %ld, Errors:%ld\n",
		testInfo->testResults[POINT],
		testInfo->testResults[PASS],
		testInfo->testResults[FAIL],
		testInfo->testResults[UNRES],
		testInfo->testResults[ERROR]);
	
	/* unmap it from our process space */
	if (munmap(testInfo, sizeof(testInfo)) != EOK) {
		error_print("munmap", errno);
		return EXIT_FAILURE;
	}
	
	/* set the shared memory path to ${SHMEM_PATH}_${PID} */
	sprintf(shmem_path, "%s_%d", SHMEM_PATH, getpid());
	
	/* goodbye shared memory */
	if (shm_unlink(shmem_path) != EOK) {
		error_print("shm_unlink", errno);
		return EXIT_FAILURE;
	}
	
	/* set it to NULL explicitly */
	testInfo = NULL;
	
	return rc;
}
