#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include "process_ancestors.h"


/* KENNY:
    The only thing that I think is left to do is make this test suite better.
    
    1). TEST_GOOD_CALLS
    Right now, each test in the good tests suite (if run alone) works okay, but 
        if the parent exits before all of the children processes are done, the children are
        adopted by init, and the expected number of siblings for the highest-level call is undeterminable.
    One solution I'm thinking of is if we find a way to kill all children without killing us (probably the best way)
    Another solution would be to simply disable error checking on the higher-level calls. Pretty janky this way, but it'll work..

    2). TEST_BAD_CALLS
    We need to create some tests for calls that we expect to fail.
    Stuff like if our array isn't big enough, or pointer is null. 
    Check the provided test file (For array_stats) for help! 
    
    Good luck :) 
*/
        




/* Sys-call number: */
#define _PROCESS_ANCESTORS_ 342

/**********************
 * Function prototypes
 *********************/
void fork_children(long masterChildArray[], int masterSize, long childArray[], int size, 
                    long pidArray[], int isRoot, long syscallsize, int topSiblings);
void forkAndTest(long childArray[], int size, long syscallsize, int topSiblings);
void fork_and_test(long masterChildArray[], int masterSize, long childArray[], int size, 
                    long pidArray[], int isRoot, long syscallsize, int topSiblings);
void test_children_good(long childArray[], int size, long pidArray[], long syscallsize, int topSiblings);
void test_children_bad(long childArray[], int size, long pidArray[], long syscallsize, int topSiblings);
static _Bool test_internal(_Bool success, int lineNum, char* argStr);
void test_good_calls();
void test_bad_calls();
static void do_syscall_working(long masterChildArray[], int size, long pidArray[], long syscallsize, int topSiblings);
static int do_syscall(struct process_info *info_array, long size, long *num_filled);
static void do_syscall_bad(struct process_info info_array[], int syscallsize, long *num_filled, int expectederr);
static void test_print_summary(void);
void sigquit_handler(int sig);

pid_t parent_pid; 
/*pid_t pid_to_kill = 0;*/

/**********************************
 * Main function
 *********************************/
int main(){
    signal(SIGQUIT, sigquit_handler);

    test_good_calls();

    test_bad_calls();

    printf("\n\n\nAll tests are finished being run! \n");
    printf("For performance during these tests, please scroll up to parse through the input. \nExiting now.\n");

    /* Keep this line! If this line disappears , then the first children will be given to init,
        and have far more siblings than expected, and we wouldn't be able to control that.*/
    //while(1);
    return 0;
}



/************************************
 * Testing routines
 ***********************************/
void test_good_calls(){
    
//    forkAndTest((long[]){1}, 1);

    parent_pid = getpid();

    /* Template:
    forkAndTest( Child array, Size of child array, size of info_array for syscall, sum of 0'th entries for previous child arrays );
    Child array can be anything you want
    Size of child array must be accurate
    Size of info_array can be anything you want (> 0)
    Sum of 0'th entries for previous child arrays must be accurate.
    */

    forkAndTest((long[]){2,6,11}, 3, 5, 0);
    sleep(2);
    sleep(2);
    sleep(2);
    kill(-parent_pid, SIGQUIT);

    forkAndTest((long[]){4, 5, 9, 2}, 4, 5, 2);
    sleep(2);
    sleep(2);
    sleep(2);
    kill(-parent_pid, SIGQUIT);
    
    forkAndTest((long[]){11, 8, 2, 6, 12}, 5, 8, 4+2);
    sleep(2);
    sleep(2);
    sleep(2);
    sleep(2);
    kill(-parent_pid, SIGQUIT);

    //                   1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20
    forkAndTest((long[]){1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 20, 25, 11+4+2);
    sleep(2);
    sleep(2);
    sleep(2);
    sleep(2);
    kill(-parent_pid, SIGQUIT);

    forkAndTest((long[]){3, 25}, 2, 5, 11+4+2+1);
    sleep(2);
    sleep(2);
    sleep(2);
    sleep(2);
    kill(-parent_pid, SIGQUIT);
}



void test_bad_calls(){

    printf("\n\n-----------------------------------------------------------------------------------------\
                \nTesting bad calls!\n\
              \n-----------------------------------------------------------------------------------------\n\n");

    /* Template:
    forkAndTest( Child array, Size of child array, size of info_array for syscall, sum of 0'th entries for previous child arrays );
    Child array can be anything you want
    Size of child array must be accurate
    Size of info_array can be anything you want (> 0)
    Sum of 0'th entries for previous child arrays must be accurate.
    */
    
    struct process_info structSize1[1];
    struct process_info structSize3[3];
    struct process_info structSize10[10];
    struct process_info structSize100[100];

    long* num_filled_good = malloc(sizeof(long));
    long* num_filled_bad3 = NULL;

    printf("sizeoflong = %ld", (unsigned long) sizeof(long));
    printf("sizeoflongint = %ld", (unsigned long) sizeof(long int));
    printf("sizeoflong no cast = %ld", sizeof(long int));
    printf("sizeoflongint no cast = %ld", sizeof(long int));

    do_syscall_bad(structSize3, -3, num_filled_good, EINVAL);
    sleep(2);
    do_syscall_bad(structSize10, -3, num_filled_good, EINVAL);
    sleep(2);
    do_syscall_bad(structSize100, -3, num_filled_good, EINVAL);
    sleep(2);

    do_syscall_bad((struct process_info*) 1LL, 3, num_filled_good, EFAULT);
    sleep(2);
    do_syscall_bad(NULL, 3, num_filled_good, EFAULT);
    sleep(2);

    do_syscall_bad(structSize3, 3, NULL, EFAULT);
    sleep(2);
    do_syscall_bad(structSize3, 3, (void*)test_print_summary, EFAULT);
    sleep(2);
    do_syscall_bad(structSize3, 3, (void*)1, EFAULT);
    sleep(2);


    test_print_summary();
    


}






/******************************************
 * Internal testing framework
 *****************************************/
static int numTests = 0;
static int numTestsPassed = 0;

static int currSysCallTestNum = 0;
static int prevSysCallTestNumFailed = -1;
static int numSysCallTestsFailed = 0;

/* Macro to allow us to get the line number and argument text: */
#define TEST(arg) test_internal((arg), __LINE__, #arg)

/* Function used to check success/failure: */
static _Bool test_internal(_Bool success, int lineNum, char* argStr){
    ++numTests;
    if (!success) {
        if (currSysCallTestNum != prevSysCallTestNumFailed){
            prevSysCallTestNumFailed = currSysCallTestNum;
            ++numSysCallTestsFailed;
        }
        printf("\n ERROR! ERROR! %4d: test on line %d failed: %s\n",
                numTestsPassed, lineNum, argStr);

    } else {
        ++numTestsPassed;
    }
    return success;
}

static void test_print_summary(void){
    
    printf("\nFor the latest test:\n");
    printf("%4d/%d tests passed.\n", numTestsPassed, numTests);
    if (numTests - numTestsPassed == 0){
        printf("%4so failures! :)\n", "N");
    } else {
        printf("%4d/%d tests FAILED!!\n", numTests - numTestsPassed, numTests);
    } 
    printf("%4d/%d unique sys-call testing configurations failed.\n\n\n\n\n", numSysCallTestsFailed, currSysCallTestNum);
}

void forkAndTest(long childArray[], int size, long syscallsize, int topSiblings){
    /* Wrapper for fork_and_test, because that one is a recursive call. At the top level,
        most of the inputs are redundant. */
    pid_t retval = 0;
    long pidArray[size];
    fork_and_test(childArray, size, childArray, size, pidArray, 1, syscallsize, topSiblings);
    
    return;
}

void fork_and_test(long masterChildArray[], int masterSize, long childArray[], int size,
                    long pidArray[], int isRoot, long syscallsize, int topSiblings){
    /* Params: 
        masterChildArray: keeps track of the actual forking tree requested.
        masterSize: size of the masterChildArray
        size: The size of the childArray
        childArray: Used for the recursion. The lowest-level call must keep
            the masterChildArray, but each recursive call must have a sub-copy. This is that sub-copy.
        pidArray: array of PIDs, for each of the "first" children at each layer.
        isRoot: "Are we the ORIGINAL process?"
    */
    /* Example: If called with masterChildArray = {2, 3, 2} build a process tree like this:
                o
               / \
              o   o
             /|\
            o o o
           / \
          o   o
          ^
          Testing is done on this child! This is why we need the masterChildArray.
    */
    
    fork_children(masterChildArray, masterSize, childArray, size, pidArray, isRoot, syscallsize, topSiblings);
    if (isRoot == 1){
        sleep(2);
        return;
    }

    test_children_good(masterChildArray, masterSize, pidArray, syscallsize, topSiblings);
    if (isRoot == 0){
        test_print_summary();
        exit(0);
    }
}

void fork_children(long masterChildArray[], int masterSize, long childArray[], int size, 
                    long pidArray[], int isRoot, long syscallsize, int topSiblings){

    for (int j = 0; j < childArray[0]; ++j){
        pid_t forkpid = fork();
	    long mypid = (long) getpid();

        /* We are one of the "left-side" processes on the tree,
           meaning we DID fork. We need to add our PID to the array
           and give it to the child we are about to fork. */
        pidArray[masterSize - size] = mypid;

        if (forkpid == -1){
            printf("\n\nSomething went wrong in the test suite! fork() returned -1.");
            return;
        } else if (forkpid == 0){
            /* If we are the child: */
            if (j == 0){
                /* The first child must continue forking, if required */
                /* Otherwise, we exit for testing */
                if ( size > 1 ){
                    /* If we still have entries in the array, keep forking */
                    /* We compare to 1 because we are a child of someone who just forked. 
                        THEIR size was effectively 1, we don't have any more entries. */
                    fork_and_test(masterChildArray, masterSize, &childArray[1], size - 1, pidArray, 0, syscallsize, topSiblings);
                } else {
                    /* If not, we are the last child. Exit for testing. */
                    pidArray[masterSize - size] = mypid;
                    sleep(2);
                    return;
                }
            } else {
                /* All other children busywait */
                while(1);
            }
        } else {
            /* If we are the parent: */
            /* We are one of the "left-side" processes on the tree,
                meaning we DID fork. We need to add our PID to the array
                and give it to the child */

            /* Sleep to ensure the children are being made */
            sleep(0.1*size);
            
        }
    }
    
    if (isRoot == 0){
        while(1);
    }

}
                
void test_children_good(long masterChildArray[], int size, long pidArray[], long syscallsize, int topSiblings){

    printf("\ntest_children called!\n");
    do_syscall_working(masterChildArray, size, pidArray, syscallsize, topSiblings);

}


void sigquit_handler(int sig){
    if (sig != SIGQUIT){
        return;
    } 
    pid_t self = getpid();
    if (parent_pid != self)
        exit(0);
}







/*********************************************
 * Functions to actually call the syscall:
 ********************************************/


static int do_syscall(struct process_info *infoArray, long size, long *num_filled){
    printf("do_syscall called!\n");
    ++currSysCallTestNum;
    printf("\nTest %d: ...Diving to kernel level\n", currSysCallTestNum);
    int result = syscall(_PROCESS_ANCESTORS_, infoArray, size, num_filled);
    int myErrno = errno;
    printf(" ... Rising to user level with result = %d", result);

    if (result != 0) {
        printf(", errno = %d", myErrno);
    } else {
        myErrno = 0;
    }

    printf("\nAbout to exit do_syscall!\n");

    return myErrno;
}

static void do_syscall_bad(struct process_info infoArray[], int syscallsize, long *num_filled, int expectederr){

    int result = do_syscall(infoArray, syscallsize, num_filled);
    if (! TEST(result == expectederr)){
        printf("\tresult = %d, expectederr = %d\n", result, expectederr);
    }
    
}

static void do_syscall_working(long masterChildArray[], int size, long pidArray[], long syscallsize, int topSiblings){
    struct process_info infoArray[syscallsize];
    long num_filled;

    long uid = 0;
    FILE *fp;
    char line[30];
    /* User popen to read the expected user ID: */
    fp = popen("/usr/bin/id -u $USER", "r");
    if (fp == NULL){
        printf("Failed to find UID\n");
        return;
    }
    fgets(line, sizeof(line)-1, fp) == NULL;
    /* Get the expected user id: */
    uid = atoi(line);
    printf("\nGot UID = %ld\n", uid);
    fclose(fp);


    /* Above here is all declarations and init stuff.*/

    int result = do_syscall(infoArray, syscallsize, &num_filled);
    if (result != 0){
        printf("\n\nFAILED in do_syscall_working: result was nonzero.\n");
    }
    printf("Right after do_syscall!\n");

    printf("\n\ninfoArray:\n");
    for (int i = 0; i < num_filled; ++i){

        struct process_info cur = infoArray[i];

        printf("\ni = %d, pid = %ld, name = %s, state = %ld, uid = %ld, nvcsw = %ld,\
 nivcsw = %ld, num_children = %ld, num_siblings = %ld\n", i, cur.pid, cur.name, 
                    cur.state, cur.uid, cur.nvcsw, cur.nivcsw, cur.num_children, cur.num_siblings);
    }


    for (int i = 0; i < num_filled /*num_filled*/; ++i){
        struct process_info cur = infoArray[i];
       
        if (i < size){
            if (! TEST(cur.pid == pidArray[size - i - 1])){
                printf("\ti = %d, curpid = %ld, expected pid = %ld\n", i, cur.pid, pidArray[size - i - 1]);
            }
            /* TEST(cur.name == "./cs300_test_processancestors.c");*/
            if (! TEST(cur.uid == uid)){
                printf("\ti = %d, cur.uid = %ld, expected uid = %ld\n", i, cur.uid, uid);
            }
            if (i == 0){
                if (! TEST(cur.num_children == 0)){
                    printf("\ti = %d, cur.num_children = %ld, expected children = 0", i, cur.num_children);
                }
            } else {
                if (! TEST(cur.num_children == masterChildArray[size - i])){
                    printf("\ti = %d, cur.num_children = %ld, expect children = %ld", i, cur.num_children, masterChildArray[size - i]);
                }
            }
            if (i == size - 1){
                if (! TEST(cur.num_siblings == masterChildArray[size - i - 1] + topSiblings - 1) ){
                    printf("\ti = %d, cur.num_siblings = %ld, expected siblings = %ld", i, cur.num_siblings, masterChildArray[size-i-1] + topSiblings - 1);
                }
            } else {
                if (! TEST(cur.num_siblings == masterChildArray[size - i - 1] - 1)){
                    printf("\ti = %d, cur.num_siblings = %ld, expected siblings = %ld", i, cur.num_siblings, masterChildArray[size - i - 1] - 1);
                }
            }
        }
    }
    printf("\nExiting do_syscall_working! Num_filled = %ld\n", num_filled);

}

