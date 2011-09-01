/**
 Copyright (c) 2011 by 6.172 Staff <6.172-staff@mit.edu>

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
**/


/**
 * testbed.c:
 *
 * This file runs your code, timing its execution and printing out the result.
 *
 **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ktiming.h"
#include "matrix_multiply.h"


int main(int argc, char** argv)
{
  int optchar;
  int show_usec = 0;
  int should_print = 0;
  int use_zero_matrix = 0;
  int i, j;
  matrix* A;
  matrix* B;
  matrix* C;


  // Parse command line arguments

  opterr = 0;

  while ((optchar = getopt(argc, argv, "upz")) != -1) {
    switch (optchar) {
      case 'u':
        show_usec = 1;
        break;
      case 'p':
        should_print = 1;
        break;
      case 'z':
        use_zero_matrix = 1;
        break;
      default:
        printf("Ignoring unrecognized option: %c\n", optchar);
        continue;
    }
  }

  // This is a trick to make the memory bug leads to a wrong output.

  int size = sizeof(int) * 4;
  int* temp[20];
  for (i = 0; i < 20; i++) {
    temp[i] = (int*)malloc(size);
    memset(temp[i], 1, size);
  }
  for (i = 0; i < 20; i++) {
    free(temp[i]);
  }

  fprintf(stderr, "Setup\n");

  A = make_matrix(4, 5);
  B = make_matrix(4, 4);
  C = make_matrix(4, 4);

  if (use_zero_matrix) {
    for (i = 0; i < A->rows; i++) {
      for (j = 0; j < A->cols; j++) {
        A->values[i][j] = 0;
      }
    }
    for (i = 0; i < B->rows; i++) {
      for (j = 0; j < B->cols; j++) {
        B->values[i][j] = 0;
      }
    }
  } else {
    // Generate random elements
    srand(time(NULL));
    for (i = 0; i < A->rows; i++) {
      for (j = 0; j < A->cols; j++) {
        A->values[i][j] = rand() % 10;
      }
    }
    for (i = 0; i < B->rows; i++) {
      for (j = 0; j < B->cols; j++) {
        B->values[i][j] = rand() % 10;
      }
    }
  }

  if (should_print) {
    printf("Matrix A: \n");
    print_matrix(A);

    printf("Matrix B: \n");
    print_matrix(B);
  }

  fprintf(stderr, "Running matrix_multiply_run()...\n");

  clockmark_t time1 = ktiming_getmark();
  matrix_multiply_run(A, B, C);
  clockmark_t time2 = ktiming_getmark();

  uint64_t elapsed = ktiming_diff_usec(&time1, &time2);
  float elapsedf = ktiming_diff_sec(&time1, &time2);

  if (should_print) {
    printf("---- RESULTS ----\n");
    printf("Result: \n");
    print_matrix(C);
    printf("---- END RESULTS ----\n");
  }

  if (show_usec) {
    printf("Elapsed execution time: %llu usec\n", (long long unsigned)elapsed);
  } else {
    printf("Elapsed execution time: %f sec\n", elapsedf);
  }

  return 0;
}
