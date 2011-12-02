#include <iostream>
#include <time.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <tbb/mutex.h>
#include <cilk/cilk.h>
#include "KhetState.h"

#define TOKEN_SIZE 4096
#define I_BUF_SIZE 512
#define version 0.1

using namespace std;

tbb::mutex inputMtx;

//buffer struct to hold user input
//since user input could rapidly be fed into stdin, a buffer is required
typedef struct 
{
  int count;
  int p; //next val index
  char buf[I_BUF_SIZE][TOKEN_SIZE];
} i_buf;

i_buf istr;

KhetState gameHis[1024];
int ply;


