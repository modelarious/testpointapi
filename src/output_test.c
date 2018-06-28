#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <pthread.h>
#include "testpoint.c"
#include "testpointsimple.c"

#ifdef __APPLE__
#include "/Users/MichaelHackman/C/includes/pthread_barriers.c"
#endif

#ifndef EOK
#define EOK 0
#endif

#define SHMEM_PATH "/a_great_pathname"

pthread_barrier_t * shared_barrier;

/* print out a descriptive error message */
void error_print(char *funcName, int rc) {
	fprintf(stderr, "%s() failed: %d %s\n", funcName,
		rc, strerror(rc));
	fflush(stderr);
}

/* returns failure on already initialized or successful initialization */
int init_shared_memory() {
	int fd, rc;
	char shmem_path[500];
	
	/* set the shared memory path to ${SHMEM_PATH}_${PID} */
	sprintf(shmem_path, "%s_%d", SHMEM_PATH, getpid());
	
	/* open shared memory object */
	fd = shm_open(shmem_path, 
		O_CREAT| /* create if non existant */
		O_RDWR, /* open for reading and writing */
		S_IRWXU /* user has read, write and execute */
	);
	
	/* fd should be nonnegative */
	if (fd < 0) {
		error_print("shm_open", errno);
		return EXIT_FAILURE;
	}
	
	/* resize using fd */
	rc = ftruncate(fd, sizeof(pthread_barrier_t));
	
	/* error handle */
	if (rc != EOK) {
		error_print("ftruncate", errno);
		return EXIT_FAILURE;
	}
	
	/* map shared memory to barrier pointer */
	shared_barrier = mmap(NULL, sizeof(pthread_barrier_t),
		PROT_READ|
		PROT_WRITE, /* read/write */
		MAP_SHARED, /* shared memory */
		fd, 0 );
	
	/* error handle */
	if (shared_barrier == MAP_FAILED) {
		error_print("mmap", errno);
		return EXIT_FAILURE;
	}
	
	/* done with fd */
	close(fd);
	
	return EXIT_SUCCESS;
}


int main(void) {
	int filedes[2];
	pid_t pid;
	int rc;
	
	if (init_shared_memory() != EXIT_SUCCESS) {
		return EXIT_FAILURE;
	}

#ifdef __linux__ 
	pthread_barrierattr_t attr;
	
	/* make sure it's shared between processes */
	if ((rc = pthread_barrierattr_init(&attr) != 0 )) {
		error_print("pthread_barrierattr_init", rc);
		return EXIT_FAILURE;
	}
	
	/* make sure it's shared between processes */
	if ((rc = pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != 0 ) {
		error_print("pthread_barrierattr_setpshared", rc);
		return EXIT_FAILURE;
	}
	
	if ((rc = pthread_barrier_init(shared_barrier, &attr, 2)) != 0 ) {
#else
	if ((rc = pthread_barrier_init(shared_barrier, NULL, 2)) != 0 ) {
#endif
		error_print("pthread_barrier_init", rc);
		return EXIT_FAILURE;
	}
		
	/* open a pipe */
	if (pipe(filedes) != 0) {
		perror("pipe");
		return -1;
	}
	
	/* fork a new process */
	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
		
	/* child program */
	} else if (pid == 0) {
	
		if (dup2(filedes[1], STDOUT_FILENO) == -1) {
			perror("dup2");
			exit(1);
		}
		
		close(filedes[1]);
		close(filedes[0]);
		fprintf(stdout, "hello world\n");fflush(stdout);
		
		if ((rc = pthread_barrier_wait(shared_barrier)) == EINVAL) {
			error_print("Child: pthread_barrier_wait", rc);
			exit(1);
		}
		
		printf("whoa");fflush(stdout);
		if ((rc = pthread_barrier_wait(shared_barrier)) == EINVAL) {
			error_print("Child: pthread_barrier_wait", rc);
			exit(1);
		}
		
		printf("whoe");fflush(stdout);
		if ((rc = pthread_barrier_wait(shared_barrier)) == EINVAL) {
			error_print("Child: pthread_barrier_wait", rc);
			exit(1);
		}
		
		printf("hello\n");fflush(stdout);
		if ((rc = pthread_barrier_wait(shared_barrier)) == EINVAL) {
			error_print("Child: pthread_barrier_wait", rc);
			exit(1);
		}
		
		printf("hello\n");fflush(stdout);
		if ((rc = pthread_barrier_wait(shared_barrier)) == EINVAL) {
			error_print("Child: pthread_barrier_wait", rc);
			exit(1);
		}
		
		printf("Last one\n");
		
		exit(1);
	}
	
	char mybuff[8192];
	while (1) {
		ssize_t count = read(filedes[0], mybuff, sizeof(mybuff));
		if (count == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("read");
				return -1;
    		}
  		} else if (count == 0) {
    		break;
  		} else {
  			mybuff[count] = 0;
  			printf("Parent received: '%s' with count %ld\n", mybuff, count);
  			
  			if (!strcmp(mybuff, "Last one\n")) {
  				printf("Comparison yay\n");
  				return 0;
  			} else {
  				printf("Comparison nay\n"); 
  			}
    		//handle_child_process_output(mybuff, count);
  		}
  		
  		if ((rc = pthread_barrier_wait(shared_barrier)) == EINVAL) {
			error_print("Parent: pthread_barrier_wait", rc);
			return EXIT_FAILURE;
		}
	}
}