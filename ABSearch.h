
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
    static Abort *global_abort;
    static int   search_done;
    static u64   start_wall_time;
    static int   bestmove;
    static int	 prev_move;

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
	/*
	int ABSearch(ABState* g, int max_depth, int search_time, 
				 void(*f)(int best_move,int depth, int score ,int nodes, double time));

	int root_search(ABState *g, int depth);

	int search(ABState *prev, KhetMove nextMove, int depth, Abort *parent_abort);
	*/
    /*
    aborts currently running alpha beta search
    */
    static void abortSearch()
	{
		global_abort->abort();
	}

    //can be called to get the index of the best known move so far
    //using the callback function on ABSearch() may be easier, but this is slightly
    //more up to date as the callback is invoked only when a round of iterative
    //deepening completes
    static int get_best_move();

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

    static int recUsable(ttRec *tt, int depth, int beta);

    //hash table of game positions
    //will be used to store best move at given depth
    //stores move chosen, score, depth of search
    //init to 0 after being made
    static ttab  tt;   // the global hashtable

    static void clearHashTable( ttab *x );

    static void freeHashTable( ttab *tbl );

    //makes hash table of of sizeMeg where sizeMeg is a size in megabytes
    static ttab  makeHashTable(int sizeMeg);
    // Age the hash table by incrementing global age
    // ---------------------------------------------
    static void ttAge( ttab *x );

    static void  putEntry(
                            ttab *x,
                            u64 key,
                            int depth,
                            int score,
                            int type,
                            int move
                            );

    static ttRec* getEntry(ttab  *x,  uint64_t key);

    // refresh a record - if read from hash table and used update age to current
    static void ttabRefresh(ttab *x, ttRec *r);

#endif
    static void timeout_handler( int signum );

    static void timer_thread(void);

    /* return an absolute wall time counter in milliseconds  */
    static u64 new_wall_time();

    /* return an absolute process time (user+system) counter in milliseconds  */
    static u64 new_process_time();

    double seconds();

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

    int search(ABState *prev, KhetMove nextMove, int depth, Abort *parent_abort);

};
#endif//header
