/*  Copyright (c) 2010 6.172 Staff
    Modified by 2011 6.172 Staff

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "ktiming.h"

typedef uint32_t data_t;

/* Function prototypes */

void sort(data_t *left, int p, int r);
void sort_i(data_t *left, int p, int r);
void sort_p(data_t *left, int p, int r);
void sort_b(data_t *left, int p, int r);
void sort_c(data_t *left, int p, int r);


// Call TEST_PASS() from your test cases to mark a test as successful
//
#define TEST_PASS() TEST_PASS_WITH_NAME(__func__, __LINE__)

#define TEST_PASS_WITH_NAME(name, line) \
    fprintf(stderr, " --> %s at line %d: PASS\n", (name), (line))

// Call TEST_FAIL from your test cases to mark a test as failed. TEST_FAIL
// should print a meaningful message with the reason that the test failed.
//
// Calling it is just like calling printf().
#define TEST_FAIL(failure_msg, args...) \
    TEST_FAIL_WITH_NAME(__func__, __LINE__, failure_msg, ##args)

#define TEST_FAIL_WITH_NAME(name, line, failure_msg, args...) do { \
    fprintf(stderr, " --> %s at line %d: FAIL\n    Reason:", (name), (line)); \
    fprintf(stderr, (failure_msg), ## args); fprintf(stderr, "\n"); \
} while (0)



/* A test case is a function that takes a 3 arguments and returns no values. */
typedef void (*test_case)(int printFlag, int N, int R);

/* Some global variables to make it easier to run individual tests. */
static int test_verbose = 1 ;

enum sort_type { sortd = 1, sorti, sortp, sortb, sortc, sortm, sortf } ;

static inline void display_array(data_t * data, int N )
{
	int i ;
	// display array
	for (i = 0; i < N; i++) {
		printf("%d ", data[i]);
	}
	printf("\n");
}

static inline void copy_data(data_t * data, data_t * data_bcup, int N)
{
	int i ;
	//copy data_bcup to data
	for (i = 0 ; i < N ; i++)
	{
		*data++ = *data_bcup++ ;
	}

}

static inline void print_sort(int stype)
{
	if (stype == 1)
	{
		printf ("sort : ") ;
	}
	else if (stype == 2)
	{
		printf ("sort_i : ") ;
	}
	else if (stype == 3)
	{
		printf ("sort_p : ") ;
	}
	else if (stype == 4)
	{
		printf ("sort_b : ") ;
	}
	else if (stype == 5)
	{
		printf ("sort_c : ") ;
	}
	else if (stype == 6)
	{
		printf ("sort_m : ") ;
	}
	else if (stype == 7)
	{
		printf ("sort_f : ") ;
	}
	printf("\n") ;

}

static inline int post_process(data_t * data, data_t * data_bcup, int N, 
					int printFlag, int stype, int begin,
					int end)
{
	int i ;
	int result = 1 ;
	if (printFlag)
	{
		print_sort(stype) ;
		printf("Data after sort\n") ;
		display_array(data, N) ;
	}

	//check if the array is unchanged from data[0..begin-1]
	for (i = 0 ; i < begin ; i++)
	{
		if (data [i] != data_bcup [i])
		{
			print_sort(stype) ;
			TEST_FAIL("Array outside sort boundary changed!\n");
			result = 0 ;
			break ;
		}
	}

	// check if sub-array is sorted
	for (i = begin + 1 ; i < end + 1 ; i++) 
	{
		if (data[i - 1] > data[i]) {
			print_sort(stype) ;
  			TEST_FAIL("Arrays are sorted: NO!\n");
			result = 0 ;
			break ;
		}
	}

	//check if the array is unchanged from data[end+1..N-1]
	for (i = end + 1 ; i < N ; i++)
	{
		if (data [i] != data_bcup [i])
		{
			print_sort(stype) ;
			TEST_FAIL("Array outside sort boundary changed!\n");
			result = 0 ;
			break ;
		}
	}
	copy_data(data, data_bcup, N) ;
	return result ;
}

static void test_correctness(int printFlag,int N, int R)
{
	clockmark_t time1, time2;
	float sum_time = 0, sum_time_i = 0, sum_time_p = 0, sum_time_b = 0,
	sum_time_c = 0, sum_time_m = 0, sum_time_f = 0 ;
	data_t *data, *data_bcup ;
	int i, j ;
	int success = 1 ;

	// allocate memory
	data = (data_t *) malloc(N * sizeof(data_t));
	data_bcup = (data_t *) malloc(N * sizeof(data_t));

	if (data == NULL || data_bcup == NULL)
	{
		printf("Error: not enough memory\n");
		free (data) ;
		free (data_bcup) ;
		exit(-1);
	}

	// repeat for each trial
	for (j = 0; j < R; j++) 
	{
		// initialize data with random numbers
		for (i = 0; i < N; i++) {
			data[i] = rand() ;
			data_bcup [i] = data [i] ;
		}
		if (printFlag)
		{
			printf("Data before sort\n") ;
			display_array(data, N) ;
		}

		// sort array
		time1 = ktiming_getmark( );
		sort(data, 0, N - 1);
		time2 = ktiming_getmark( );

		// compute time for this trial
		sum_time += ktiming_diff_sec( &time1, &time2 );
		success &= post_process(data, data_bcup, N, printFlag, 1, 0, N - 1) ;

		// sort array with inline sort
		time1 = ktiming_getmark( );
		sort_i(data, 0, N - 1);
		time2 = ktiming_getmark( );

		// compute time for this trial
		sum_time_i += ktiming_diff_sec( &time1, &time2 );
		success &= post_process(data, data_bcup, N, printFlag, 2, 0, N - 1) ;

		// sort array with ptr sort
		time1 = ktiming_getmark( );
		sort_p(data, 0, N - 1);
		time2 = ktiming_getmark( );

		// compute time for this trial
		sum_time_p += ktiming_diff_sec( &time1, &time2 );
		success &= post_process(data, data_bcup, N, printFlag, 3, 0, N - 1) ;

		// sort array with branchless sort
		time1 = ktiming_getmark( );
		sort_b(data, 0, N - 1);
		time2 = ktiming_getmark( );

		// compute time for this trial
		sum_time_b += ktiming_diff_sec( &time1, &time2 );
		success &= post_process(data, data_bcup, N, printFlag, 4, 0, N - 1) ;

		// sort array with coarsened sort
		time1 = ktiming_getmark( );
		sort_c(data, 0, N - 1);
		time2 = ktiming_getmark( );

		// compute time for this trial
		sum_time_c += ktiming_diff_sec( &time1, &time2 );
		success &= post_process(data, data_bcup, N, printFlag, 5, 0, N - 1) ;

		// sort array with memory optimization 1
		time1 = ktiming_getmark( );
		sort_m(data, 0, N - 1);
		time2 = ktiming_getmark( );

		// compute time for this trial
		sum_time_m += ktiming_diff_sec( &time1, &time2 );
		success &= post_process(data, data_bcup, N, printFlag, 6, 0, N - 1) ;

		// sort array with memory optimization 2
		time1 = ktiming_getmark( );
		sort_f(data, 0, N - 1);
		time2 = ktiming_getmark( );

		// compute time for this trial
		sum_time_f += ktiming_diff_sec( &time1, &time2 );
		success &= post_process(data, data_bcup, N, printFlag, 7, 0, N - 1) ;
	
		if (!success)
		{
			break ;
		}
	}
	if (success)
	{
		printf("Arrays are sorted: yes\n");
		TEST_PASS() ;
		// report total execution time
		printf( "sort : Elapsed execution time: %f sec\n", sum_time );
		printf( "sort_i : Elapsed execution time: %f sec\n", sum_time_i);
		printf( "sort_p : Elapsed execution time: %f sec\n", sum_time_p);
		printf( "sort_b : Elapsed execution time: %f sec\n", sum_time_b);
		printf( "sort_c : Elapsed execution time: %f sec\n", sum_time_c);
		printf( "sort_m : Elapsed execution time: %f sec\n", sum_time_m);
		printf( "sort_f : Elapsed execution time: %f sec\n", sum_time_f);
	}

	free (data) ;
	free (data_bcup) ;
	return ;
}

static void test_empty_array(int printFlag,int N, int R)
{
	data_t data[] = {} ;
	sort(data, 0, 0);
	sort_i(data, 0, 0);
	sort_p(data, 0, 0);
	sort_b(data, 0, 0);
	sort_c(data, 0, 0);
	sort_m(data, 0, 0);
	sort_f(data, 0, 0);
	TEST_PASS() ;	
}

static void test_one_element(int printFlag,int N, int R)
{
	data_t data[] = {1} ;
	sort(data, 0, 0);
	sort_i(data, 0, 0);
	sort_p(data, 0, 0);
	sort_b(data, 0, 0);
	sort_c(data, 0, 0);
	sort_m(data, 0, 0);
	sort_f(data, 0, 0);
	if (data [0] == 1)
	{
		TEST_PASS() ;
	}
	else
	{
		TEST_FAIL("Sorting array with one element failed") ;
	}	
}


static void test_subarray(int printFlag,int N, int R)
{
	data_t *data, *data_bcup ;
	int i, j ;
	int success = 1 ;
	// allocate memory
	data = (data_t *) malloc(N * sizeof(data_t));
	data_bcup = (data_t *) malloc(N * sizeof(data_t));
	
	if (data == NULL || data_bcup == NULL)
	{
		printf("Error: not enough memory\n");
		free (data) ;
		free (data_bcup) ;
		exit(-1);
	}

	// initialize data with random numbers
	for (i = 0; i < N; i++) {
		data[i] = rand() ;
		data_bcup [i] = data [i] ;
	}
	if (printFlag)
	{
		printf("Data before sort\n") ;
		display_array(data, N) ;
	}
	int begin = rand() % N ;
	int end = N - 1 - begin ;
	if (begin > end)
	{
		int temp = begin ;
		begin = end ;
		end = temp ;
	}

	printf("Sorting subarray A[%d..%d]\n", begin, end) ;
	// sort array
	sort(data, begin, end);
	success &= post_process(data, data_bcup, N, printFlag, 1, begin, end) ;

	// sort array with inline sort
	sort_i(data, begin, end);
	success &= post_process(data, data_bcup, N, printFlag, 2, begin, end) ;

	// sort array with ptr sort
	sort_p(data, begin, end);
	success &= post_process(data, data_bcup, N, printFlag, 3, begin, end) ;

	// sort array with branchless sort
	sort_b(data, begin, end);
	success &= post_process(data, data_bcup, N, printFlag, 4, begin, end) ;

	// sort array with coarsened sort
	sort_c(data, begin, end);
	success &= post_process(data, data_bcup, N, printFlag, 5, begin, end) ;

	// sort array with memory optimization 1
	sort_m(data, begin, end);
	success &= post_process(data, data_bcup, N, printFlag, 6, begin, end) ;

	// sort array with memory optimization 2
	sort_f(data, begin, end);
	success &= post_process(data, data_bcup, N, printFlag, 7, begin, end) ;

	if (success)
	{
		printf("Arrays are sorted: yes\n");
		TEST_PASS() ;
	}

	free (data) ;
	free (data_bcup) ;
	return ;
}

test_case test_cases[] = {
	test_correctness,
	test_empty_array,
	test_one_element,
	test_subarray,
	// ADD YOUR TEST CASES HERE
	NULL // This marks the end of all test cases. Don't change this!
};
