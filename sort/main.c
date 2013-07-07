#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ktiming.h" 
#include <assert.h>

typedef uint32_t data_t;

typedef void (*test_case)(int printFlag, int N, int R);
/* Extern variables */
extern test_case test_cases[];

static void run_test_suite(int start_idx, int printFlag, int N, int R) 
{
	int i ;
	for (i = 0; test_cases[i] != NULL; i++) {
		if (i < start_idx)
			continue;
		fprintf(stderr, "\nRunning test #%d...\n", i);
		(*test_cases[i])(printFlag, N, R);
	}
	fprintf(stderr, "Done testing.\n");
}


int main( int argc, char** argv )
{
	int i, j, N, R, optchar, printFlag = 0;
	unsigned int seed = 0;
	clockmark_t time1, time2;

	// process command line options
	while( ( optchar = getopt( argc, argv, "s:p" ) ) != -1 ) {
		switch( optchar ) {
			case 's':
				seed = (unsigned int) atoi(optarg);
				printf("Using user-provided seed: %u\n", seed);
				srand(seed);
				break;
			case 'p':
				printFlag = 1;
				break;
			default:
				printf( "Ignoring unrecognized option: %c\n", optchar );
				continue;
		}
	}

	// shift remaining arguments over
	int remaining_args = argc - optind;
	for( i = 1; i <= remaining_args; ++i ) {
		argv[i] = argv[i+optind-1];
	}

	// check to make sure number of arguments is correct
	if (remaining_args != 2) {
		printf("Usage: %s [-p] [-s seed] <num_elements> <num_repeats>\n", argv[0]);
		printf("-p : print before/after arrays\n");
		printf("-s : set rand() seed value\n");
		exit(-1);
	}

	N = atoi(argv[1]);
	R = atoi(argv[2]);

	if (N < 1 || N > 10000000) {
		printf("Please pick a number between 1 and 10000000\n");
		exit(-1);
	}

	run_test_suite(0, printFlag, N, R) ;	

	return 0;
}
