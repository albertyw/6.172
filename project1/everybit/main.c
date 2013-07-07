/*  Copyright (c) 2010 6.172 Staff

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

#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*test_case)(void);

extern test_case test_cases[];

static void run_test_suite(int start_idx) {
  for (int i = 0; test_cases[i] != NULL; i++) {
    if (i < start_idx)
      continue;
    fprintf(stderr, "Running test #%d...\n", i);
    (*test_cases[i])();
  }
  fprintf(stderr, "Done testing.\n");
}

extern double longrunning_rotation(void);
//extern double longrunning_flipcount(void);

void print_usage(const char *argv_0)
{
    fprintf(stderr, "usage: %s\n"
      "\t -t 0\tRun test suite, starting from the first test\n"
      "\t -r\tRun a sample long-running rotation operation\n"
      "\t -f\tRun a sample long-running flip count operation\n"
      , argv_0);
}

int main(int argc, char **argv) {
  char optchar;
  opterr = 0;
  //double runningTime = 0.0;
  while ((optchar = getopt(argc, argv, "t:rf")) != -1) {
    switch (optchar) {
      case 't':
        run_test_suite(atoi(optarg));
        return EXIT_SUCCESS;
        break;
      case 'r':
        printf("---- RESULTS ----\n");
        printf("Elapsed execution time: %.6fs\n", longrunning_rotation());
        printf("---- END RESULTS ----\n");
        /*
        for(int i=0; i < 10000; i++){
          runningTime += longrunning_rotation();
        }
        printf("%.6fs\n", runningTime);
        */
        return EXIT_SUCCESS;
        break;
        /*
      case 'f':
        printf("---- RESULTS ----\n");
        printf("Elapsed execution time: %.6fs\n", longrunning_flipcount());
        printf("---- END RESULTS ----\n");

        return EXIT_SUCCESS;
        break;
        */
    }
  }
  print_usage(argv[0]);
}
