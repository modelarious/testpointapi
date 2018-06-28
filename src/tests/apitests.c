/* make a function that runs all the unit tests.  Then you can just fork a 
new process and run all the tests again to verify that it works in a forked process 
and do the same with another thread XXX */


#include "testpointsimple.c"
#include "testpoint.c"
#include <time.h>

int verify_testInfo(void);
int point_one_teststart(void);
int point_two_teststart(void);
int unit_test_teststart(void);
int unit_test_increment_api(void);
int main(void);

/* this is not a guide of how to use the testpoint.c api, 
as I have to use testpointsimple.c to test it */

typedef struct {
	int (*func)(char *, ...);
	char * funcName;
	test_state_t state;
} test_case_info_t;

int verify_testInfo() {
	int rc;
	struct timespec mutex_timeout;
	
	mutex_timeout.tv_sec = 5;
	mutex_timeout.tv_nsec = 0;
	
	for (int i = 0; i < NUMBER_OF_STATES; i++) {
		if (testInfo->testResults[i] != 0) {
			simpletestnote("testResults[%d] was not equal to 0", i);
			return EXIT_FAILURE;
		}

		/* add kernel timeout XXX */
#ifdef __linux__
		rc = pthread_mutex_timedlock(&testInfo->mutexes[i], &mutex_timeout);
#elif __APPLE__
		rc = pthread_mutex_lock(&testInfo->mutexes[i]);
#endif
		if (rc != EOK) {
			simpletestnote("mutexes[%d] returned %d %s", i, rc, strerror(rc));
			return EXIT_FAILURE;
		}
			
		if ((rc=pthread_mutex_unlock(&testInfo->mutexes[i])) != EOK) {
			simpletestnote("mutexes[%d] returned %d %s", i, rc, strerror(rc));
			return EXIT_FAILURE;
		}

	}
		
	if ((rc=pthread_mutex_lock(&testInfo->exclusivePrint)) != EOK) {
		simpletestnote("exclusivePrint returned %d %s", rc, strerror(rc));
		return EXIT_FAILURE;
	}
			
	if ((rc=pthread_mutex_unlock(&testInfo->exclusivePrint)) != EOK) {
		simpletestnote("exclusivePrint returned %d %s", rc, strerror(rc));
		return EXIT_FAILURE;
	}
		
	if (testInfo->filestream != stdout) {
		simpletestnote("filestream should start as stdout");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

/* returns EXIT_FAILURE on fail, EXIT_SUCCESS on pass */
int point_one_teststart() {

	simplepointstart("call test start and verify contents of default shared memory object");
	
	/* test info must be NULL if it is not in use */
	if (testInfo != NULL) {
		simplepointfail("testInfo was not NULL before assignment");
		return EXIT_FAILURE;
	}

	/* creates shared memory object in the background and populates it */
	if (teststart("**this is a test") != EXIT_SUCCESS) {
		simplepointfail("teststart() returned a failure");
		testend("**this statement erases the shared memory structure");
		return EXIT_FAILURE;
	}

	/* should have new values now */
	if (testInfo == NULL) {
		simplepointfail("testInfo was not initialized");
		testend("**this statement erases the shared memory structure");
		return EXIT_FAILURE;
	
	/* check if all the correct defaults are in place */
	} else if (verify_testInfo() == EXIT_FAILURE) {
		simplepointfail("testInfo was not initialized to proper values");
		testend("**this statement erases the shared memory structure");
		return EXIT_FAILURE;
	}
	
	/* remove the shared memory object */
	testend("**this statement erases the shared memory structure");
	
	/* test info must be NULL if it is not in use */
	if (testInfo != NULL) {
		simplepointfail("testInfo did not return to being NULL");
		return EXIT_FAILURE;
	}
	
	/* all is well */
	simplepointpass("testInfo contents verified");
	return EXIT_SUCCESS;
}

int point_two_teststart() {
	simplepointstart("calling teststart() twice returns an error");
	
	/* creates shared memory object in the background and populates it */
	if (teststart("**this is a test") != EXIT_SUCCESS) {
		simplepointfail("teststart() returned a failure");
		testend("**this statement erases the shared memory structure");
		return EXIT_FAILURE;
	}
	
	/* creates shared memory object in the background and populates it */
	if (teststart("**this is a test") != EXIT_FAILURE) {
		simplepointfail("teststart() should have returned a failure on second initialization");
		testend("**this statement erases the shared memory structure");
		return EXIT_FAILURE;
	}
	
	/* remove the shared memory object */
	testend("**this statement erases the shared memory structure");
	
	simplepointpass("teststart() returned an error on second call");
	return EXIT_SUCCESS;
}

int unit_test_teststart() {
	int (*tests[])(void) = {
		/* verify that contents of testInfo are 
		correct when initialized with teststart()*/
		point_one_teststart,
		
		/* verify that calling teststart() twice will
		return an error */
		point_two_teststart
	};
	
	size_t test_list_len = sizeof(tests)/sizeof(tests[0]);
	
	simpleteststart("teststart() api test");
	for (size_t test = 0; test < test_list_len; test++) {
		tests[test]();
	}
	simpletestend("teststart() api test");
	
	return EXIT_SUCCESS;
}

int unit_test_testend() {
	int (*tests[])(void) = {
		/* verify that contents of testInfo are 
		correct when initialized with teststart()*/
		point_one_teststart,
		
		/* verify that calling teststart() twice will
		return an error */
		point_two_teststart
	};
	
	size_t test_list_len = sizeof(tests)/sizeof(tests[0]);
	
	simpleteststart("teststart() api test");
	for (size_t test = 0; test < test_list_len; test++) {
		tests[test]();
	}
	simpletestend("teststart() api test");
	
	return EXIT_SUCCESS;
}

int unit_test_increment_api() {
	
	test_case_info_t caseInfo[] = {
		{pointpass,  "pointpass",  PASS},
		{pointfail,  "pointfail",  FAIL},
		{pointunres, "pointunres", UNRES},
		{pointerrormsg, "pointerrormsg", ERROR},
	};
	size_t caseInfoLen = 4;
	
	long before;
	int rc;
	
	simpleteststart("incrementing api test");
	
	/* test point to verify that the shared memory object has ben created, if not,
	   abort test case as many next points would fail */
	simplepointstart("Create shared memory object to run tests");
	
	/* call teststart() to initialize the shared memory object */
	if (teststart("Starting testing") != EXIT_SUCCESS) {
		simplepointfail("Couldn't initialize object using teststart()");
		simpletestend("incrementing api test"); //XXX could the entire logic be moved and a function created like so: "
		                                        //void run_test() { simpleteststart(); all_the_logic_in_separate_function(); simpletestend(); }
		return EXIT_FAILURE;
	} else {
		simplepointpass("Initialized object using teststart()");
	}
	
	//fwrite(stdout, sizeof(char), );
	//open_memstream();
	
	/* run test cases */
	for (size_t test = 0; test < caseInfoLen; test++) {
		
		before = testInfo->testResults[caseInfo[test].state];

		simplepointstart("Use of function %s() should increase value of related state", caseInfo[test].funcName);
		rc = caseInfo[test].func("My great message %d %s", 12, "hello world");
		
		/* if the state hasn't increased by one, fail point */
		if (before + 1 != testInfo->testResults[caseInfo[test].state]) {
			
			/* important to verify if it fails, it returns the correct code */
			if (rc != EXIT_FAILURE) {
				simplepointerrormsg("%s() did not return a failure", caseInfo[test].funcName);
			}
			simplepointfail("%s() failed to increase related state", caseInfo[test].funcName);
			
		/* if function returned a failure, fail as well */
		} else if (rc == EXIT_FAILURE) {
			simplepointfail("%s() returned a failure", caseInfo[test].funcName);
		
		} else {
			simplepointpass("%s() increased related state", caseInfo[test].funcName);
		}
	}
	testend("done testing, removing shared memory");
	
	simpletestend("incrementing api test");
	
	
	return EXIT_SUCCESS;
}

/* call testend() before teststart() XXX*/

int main() {
	unit_test_teststart();
	unit_test_increment_api();
}

