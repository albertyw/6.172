#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ktiming.h" 
#include <assert.h>

typedef uint32_t data_t;

/* Function prototypes */

void sort_p(data_t *left, int p, int r);


/* Function definitions */

int main( int argc, char** argv )
{
  int i, j, N, R, optchar, printFlag = 0;
  unsigned int seed = 0;
  clockmark_t time1, time2;
  float sum_time = 0 ;
  data_t *data ; 

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
    printf("  -p : print before/after arrays\n");
    printf("  -s : set rand() seed value\n");
    exit(-1);
  }

  N = atoi(argv[1]);
  R = atoi(argv[2]);

  if (N < 2 || N > 10000000) {
    printf("Please pick a number between 2 and 10000000\n");
    exit(-1);
  }

  // allocate memory
  data = (data_t *) malloc(N * sizeof(data_t));
  if (data == NULL) {
    printf("Error: not enough memory\n");
    exit(-1);
  }

  // repeat for each trial
  for (j = 0; j < R; j++) {

    // initialize data with random numbers
    for (i = 0; i < N; i++) {
      data[i] = rand();
    }

    // display array
    if (printFlag) {
      for (i = 0; i < N; i++) {
        printf("%d ", data[i]);
      }
      printf("\n");
    }

    // sort array
    time1 = ktiming_getmark( );
    sort_p(data, 0, N - 1);
    time2 = ktiming_getmark( );

    // compute time for this trial
    sum_time += ktiming_diff_sec( &time1, &time2 );

    // display array
    if (printFlag) {
      for (i = 0; i < N; i++) {
        printf("%d ", data[i]);
      }
      printf("\n");
    }

    // check if array is sorted
    for (i = 1; i < N; i++) {
      if (data[i - 1] > data[i]) {
        printf("Default sort :Arrays are sorted: NO!\n");
        exit(-1);
      }
    }
  }
  printf("Arrays are sorted: yes\n");

  // report total execution time
  printf( "sort_p : Elapsed execution time: %f sec\n", sum_time);

  free (data) ;
  return 0;
}
