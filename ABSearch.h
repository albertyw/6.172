
/*
** Permission is hereby granted, free of charge, to any person
 ** obtaining a copy of this software and associated documentation
 ** files (the "Software"), to deal in the Software without
 ** restriction, including without limitation the rights to use,
 ** copy, modify, merge, publish, distribute, sublicense, and/or sell
 ** copies of the Software, and to permit persons to whom the
 ** Software is furnished to do so, subject to the following
 ** conditions:

 ** The above copyright notice and this permission notice shall be
 ** included in all copies or substantial portions of the Software.

 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 ** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 ** HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 ** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 ** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 ** OTHER DEALINGS IN THE SOFTWARE.
 **/
#ifndef ABSEARCH_H
#define ABSEARCH_H 1

#include <iostream>
#include <vector>
#include <cilk/cilk.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "Abort.h"
#include "ABState.h"


namespace _ABSEARCH {
static int   timeout;
static bool global_abort;
static int   search_done;
static u64   start_wall_time;
static int   bestmove;
static int   prev_move=0;   /* best move from last iteration */

static double  seconds();
static void  init_hash_table();
static u64 new_wall_time();


static int   nodec[32]; /* array of nodecoun, one per processor */

/*
Alpha beta search function
g: initial state
max_depth: progressive deepening is used to this depth
search_time: maximum time in milliseconds to search
f(int,int,int,int): optional callback function 
is called with best move index, depth, score, and nodes searched, and time
taken to search this depth
at the end of each search to depth n from 1..max_depth

NOT threadsafe due to use of globals, running multiple searchs concurrently
results in undefined behavior
*/
template <class ABState>
int ABSearch(ABState* g, int max_depth, int search_time, 
    void(*f)(int best_move,int depth, int score ,int nodes, double time));

/*
aborts currently running alpha beta search
   */
static void abortSearch() {
  global_abort = true;
}

//can be called to get the index of the best known move so far
//using the callback function on ABSearch() may be easier, but this is slightly
//more up to date as the callback is invoked only when a round of iterative
//deepening completes
static int get_best_move() {
	return bestmove;
}

#ifndef MAX_DEPTH
#define MAX_DEPTH 100
#endif

#define INF 32700
#define MATE 32000

#define REPWIN 32001

//#define REP 0
#define HASH 1

#ifdef HASH

#define LOWER_BOUND 0
#define EXACT_SCORE 1
#define UPPER_BOUND 2

//size of hash table
#define HASH_TABLE_SIZE 40

typedef struct
{
  u64         verify;
  int          score:16;
  int        quality:16;
  int          bound:8;
  int           move;
  int           age:4;

} ttRec;

typedef struct
{
    ttRec set[4];

} transposition;

typedef struct
{
  u64  size;
  u64  mask;
  unsigned  age;
  transposition *tt;

} ttab;

static int recUsable(ttRec *tt, int depth, int beta)
{
  if (tt->quality < depth) return 0;
  // if (abs(tt->score) > WIN - 101) return 0;  

  if ( (tt->bound & LOWER_BOUND) && tt->score >= beta) return 1;
  if ( (tt->bound & UPPER_BOUND) && tt->score < beta) return 1;

  return 0;
}

//hash table of game positions
//will be used to store best move at given depth
//stores move chosen, score, depth of search
//init to 0 after being made
static ttab  tt;   // the global hashtable

static void clearHashTable( ttab *x )
{
    memset( x->tt, 0, sizeof(transposition) * x->size );
      x->age = 0;
}

static void freeHashTable( ttab *tbl ) 
{ 
    free(tbl->tt);
      tbl->tt = NULL;
}

//makes hash table of of sizeMeg where sizeMeg is a size in megabytes
static ttab  makeHashTable(int sizeMeg)
{
  ttab  x;
  uint64_t  entries = 0;
  uint64_t  sz = (uint64_t) sizeMeg * (1ULL << 20);

  // transposition is a set of 4 individual records
  entries = sz / sizeof( transposition );  // total number of entries we could have

  entries = 1 + entries / 2; 

  // round up to nearest power of 2
  entries--;
  entries |= entries >> 1;
  entries |= entries >> 2;
  entries |= entries >> 4;
  entries |= entries >> 8;
  entries |= entries >> 16;
  entries |= entries >> 32;
  entries++;

  x.size = entries;
  x.mask = entries - 1;
  x.age = 0;

  x.tt = (transposition *) malloc(sizeof(transposition) * entries);

  if (x.tt == NULL) {
    fprintf( stderr,  "Hash table too big\n");
    exit(1);
  }

  // init the table to 0
  clearHashTable(&x);

  return(x);
}// Age the hash table by incrementing global age
// ---------------------------------------------
static void ttAge( ttab *x ) {
    x->age = (x->age + 1) & 255;
}

static void  putEntry(
    ttab *x,
    u64 key,
    int depth,
    int score,
    int type,
    int move
    )
{
  uint64_t  ix;
  ttRec     *e;         // current one we are looking at
  ttRec     *r;         // the one to beat
  ttRec     *set;       // the set of records we are looking into
  int       i;

  //assert( abs(score) != INF );

  ix = key & x->mask;
  set = x->tt[ix].set;

  e = set;
  r = e;     // r is pointer to best match so far

  int  ben = -99;   // best entry found so far

  for (i=0; i<4; i++, e++) {
    int hp = 0;   // hash points 

    // always use entry if not used or has same key
    if ( !e->verify  || key == e->verify ) {

      if (move == 0) move = e->move;

      e->verify = key;
      e->quality = depth;
      e->move = move;
      e->age = x->age;
      e->score = score;
      e->bound = type;
      return;

    } else if (i == 0) { continue; }

    // otherwise ....
    if (e->age == x->age) hp -= 6;          // prefer not to use if same age
    if (e->quality < r->quality) hp += 1;   // prefer quality 

    if (hp > ben) {
      ben = hp;
      r = e;
    }

  }

  r->verify = key;
  r->quality = depth;
  r->move = move;
  r->age = x->age;
  r->score = score;
  r->bound = type;
}

static ttRec* getEntry(ttab  *x,  uint64_t key)
{
  uint64_t  ix;
  ttRec     *e;
  int       i;

  ix = key & x->mask;
  e = x->tt[ix].set;

  for (i=0; i<4; i++, e++)
    if (e->verify == key)
      return e;

  return NULL;
}

// refresh a record - if read from hash table and used update age to current
static void ttabRefresh(ttab *x, ttRec *r)
{
  r->age = x->age;
}

#endif
static void timeout_handler( int signum );


static void timer_thread(void)
{
     for (;;) {
      if (new_wall_time() >= (u64) timeout * 1000 + start_wall_time) {
           global_abort = true;
           return;
      }
      if (search_done) return;
      usleep(20000);
     }
}

static void timeout_handler( int signum )
{
  global_abort = true;
}


/* return an absolute wall time counter in milliseconds  */
static u64 new_wall_time(){
  struct timeval t;
  struct timezone tz;
  u64 result;

  gettimeofday(&t,&tz);

  result = 1000 * (u64) t.tv_sec;
  result += ((u64) t.tv_usec)/1000;

  return(result);
}

/* return an absolute process time (user+system) counter in milliseconds  */
static u64 new_process_time(){
  struct rusage ru;
  u64 result;

  getrusage(RUSAGE_SELF,&ru);

  result  = 1000 * (u64) ru.ru_utime.tv_sec;
  result += ((u64) ru.ru_utime.tv_usec)/1000;
  result += 1000 * (u64) ru.ru_stime.tv_sec;
  result += ((u64) ru.ru_stime.tv_usec)/1000;

  return(result);
}



double seconds()
{

#if 1
     return( ((double)new_wall_time())/1000);
#else
    return( ((double)new_process_time())/1000); 
#endif

}

/*
Alpha beta search function
g: initial state
max_depth: progressive deepening is used to this depth
search_time: maximum time in milliseconds to search
f(int,int,int,int): optional callback function 
is called with best move index, depth, score, and nodes searched, and time
taken to search this depth
at the end of each search to depth n from 1..max_depth
NOT threadsafe, running multiple searchs concurrently results in undefined
behavior
*/
int ABSearch(ABState* g, int max_depth, int search_time, 
    void(*f)(int best_move,int depth, int score ,int nodes, double time));

/*
starts ABSearch assuming g is the root
need this special case because this inlet should update
global var bestmove
*/
int root_search(ABState *g, int depth);

int search(ABState *prev, KhetMove *next_move, int depth );



};
#endif//header
