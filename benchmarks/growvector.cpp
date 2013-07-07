/**
 *
 * growvector imitates the behavior of appending to a vector.
 *
 * Try the following (on a P-processor machine):
 *
 *  growvector 1 100000 8
 *  growvector P 100000 8
 *
 *  Written for Fall 2011 by 6.172 Staff
*/


#include <stdio.h>
#include <stdlib.h>

#include "fred.h"
#include "cpuinfo.h"
#include "timer.h"

#include "../wrapper.cpp"

// This class just holds arguments to each thread.
class workerArg {
public:
  workerArg (int objSize, int iterations)
    : _objSize (objSize),
      _iterations (iterations)
  {}

  char** _array;
  int _arraySize;
  int _objSize;
  int _iterations;
};


#if defined(_WIN32)
extern "C" void worker (void * arg)
#else
extern "C" void * worker (void * arg)
#endif
{
  workerArg * w = (workerArg *) arg;
  w->_arraySize = 8;
  w->_array = (char**) CUSTOM_MALLOC(w->_arraySize * sizeof(char*));
  
  for (int i = 0; i < w->_iterations; i++) {
    if (i == w->_arraySize) {
      // Grow the array
      w->_arraySize *= 2;
      w->_array = (char**) CUSTOM_REALLOC(w->_array, w->_arraySize * sizeof(char*));
      
      // Write everything in the array
      for (int j = 0; j < i; j++) {
        int size;
        if (j && !(j & (j - 1))) {
          size = j;
        } else {
          size = w->_objSize;
        }
        for (int k = 0; k < size; k++) {
          w->_array[j][k] = (char) k;
          volatile char ch = w->_array[j][k];
          ch++;
        }
      }
    }
    
    int size;
    if (i && !(i & (i - 1))) {
      // Alocate bigger block for power of 2
      size = i;
    } else {
      size = w->_objSize;
    }
    w->_array[i] = (char*) CUSTOM_MALLOC(size);
  }

  for (int i = 0; i < w->_iterations; i++) {
    // Write everything in the array
    int size;
    if (i && !(i & (i - 1))) {
      size = i;
    } else {
      size = w->_objSize;
    }
    for (int k = 0; k < size; k++) {
      w->_array[i][k] = (char) k;
      volatile char ch = w->_array[i][k];
      ch++;
    }
    CUSTOM_FREE(w->_array[i]);
  }
  
  CUSTOM_FREE(w->_array);
  delete w;

  end_thread();

#if !defined(_WIN32)
  return NULL;
#endif
}


int main (int argc, char * argv[])
{
  int nthreads;
  int iterations;
  int objSize;
  int repetitions;

  if (argc > 3) {
    nthreads = atoi(argv[1]);
    iterations = atoi(argv[2]);
    objSize = atoi(argv[3]);
  } else {
    fprintf (stderr, "Usage: %s nthreads iterations objSize\n", argv[0]);
    return 1;
  }

  HL::Fred * threads = new HL::Fred[nthreads];
  HL::Fred::setConcurrency (HL::CPUInfo::getNumProcessors());

  int i;

  HL::Timer t;
  t.start();

  for (i = 0; i < nthreads; i++) {
    workerArg * w = new workerArg (objSize, iterations);
    threads[i].create (&worker, (void *) w);
  }
  for (i = 0; i < nthreads; i++) {
    threads[i].join();
  }
  t.stop();

  delete [] threads;

  printf ("Time elapsed = %f seconds.\n", (double) t);
  end_program();
  return 0;
}
