#include "testpointsimple.c"
#include "testpoint.c"
#include <time.h>

/* this is not a guide of how to use the testpoint.c api, 
as I have to use testpointsimple.c to test it */



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
int point_one() {

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

int point_two() {
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
		point_one,
		
		/* verify that calling teststart() twice will
		return an error */
		point_two
	};

	int (*similar_funcs[])(char * str, ...) = {
		pointpass,
		pointfail,
		pointunres,
	};
	
	char * similar_func_names[] = {
		"pointpass",
		"pointfail",
		"pointunres",
	};
	
	test_state_t associated_test_states[] = {
		PASS,
		FAIL,
		UNRES
	};
	
	size_t test_list_len = sizeof(tests)/sizeof(tests[0]);
	size_t similar_funcs_len = sizeof(similar_funcs)/sizeof(similar_funcs[0]);
	
	simpleteststart("teststart() api test");
	for (size_t test = 0; test < test_list_len; test++) {
		tests[test]();
	}
	simpletestend("teststart() api test");
	
	teststart("Starting testing");
	for (size_t test = 0; test < similar_funcs_len; test++) {
		simpleteststart("%s() api test", similar_func_names[test]);
		similar_funcs[test]("My great message %d %s", 12, "hello world");
		simpletestend("%s() api test", similar_func_names[test]);
	}
	testend("done testing testing");
	
	return EXIT_SUCCESS;
}

/* call testend() before teststart() XXX*/


	

int main() {
	unit_test_teststart();
}