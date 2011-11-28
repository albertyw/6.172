
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
        global_abort->abort();
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
                global_abort->abort();
                return;
            }
            if (search_done) return;
            usleep(20000);
        }
    }

    static void timeout_handler( int signum )
    {
        global_abort->abort();
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
    template <class ABState>
    int ABSearch(ABState* g, int max_depth, int search_time, 
    void(*f)(int best_move,int depth, int score ,int nodes, double time)){

        char    buf[8];
        double starttime;

        //not really infinite
        if(search_time < 0) {
            timeout = 30000;
        }
        else { 
            timeout = search_time / 1000;
        }

        start_wall_time = new_wall_time();
        if (__cilkrts_get_nworkers() < 2) {
            fprintf(stderr, "ABSearch requires at least -nproc 2 for time control\n");
            exit(1);
        }
#ifdef HASH
        //Be sure to free this if used
        //pick a size appropriate to your environ
        tt = makeHashTable(HASH_TABLE_SIZE);
#endif  
        //node count stats
        for (int i = 0; i < __cilkrts_get_nworkers(); ++i)
        nodec[i]=0;
        int total_nc = 0;
        starttime = seconds();

        /* write a random move in buf, just in case we timeout at depth 1 */
        //  if(f) {
        //    (*f)(0, depth, score, nc, tt);
        //  }
        
        if (global_abort)
            delete global_abort;
        global_abort = new Abort();

        cilk_spawn timer_thread();
        //iterative deepening loop
        for (int depth=1; depth <= max_depth; depth++)
        {
            int     score;
            double  tt;
            int nc;

            search_done = 0;

            
            score = root_search( g, depth );

            nc = 0;
            for (int i = 0; i < __cilkrts_get_nworkers(); ++i)
            nc += nodec[i];
            total_nc += nc;

            if (global_abort->isAborted()) 
            break;
            tt = seconds() - starttime;
            if(f) {
                (*f)(bestmove, depth, score, nc, tt);
            }
            if (abs(score) >= MATE ) break;
        }
#ifdef HASH
        //preserve between searches?
        //at the very least clear instead of alloc
        freeHashTable(&tt);
#endif  
        delete global_abort;
        return bestmove; 
    }

    /*
    starts ABSearch assuming g is the root
    need this special case because this inlet should update
    global var bestmove
    */
    template<class ABState>
    int root_search(ABState *g, int depth) {
        int          mv;
        int          sc;
        static int   prev_move=0;   /* best move from last iteration */
        int bestscore = -INF;
        tbb::mutex m;

        //Cilk inlet translation
        //multiple search functions will be spawned from this state but when
        //they return, they may affect the alpha beta values or cause a prune
        //this function is invoked when the spawned computation returns, but is
        //guaranteed to run serially, so multiple returning functions dont cause
        //races on AB values

        //lambdas have hard to determine type. use auto to infer
        //both lambda and auto are C++0x features
        auto root_search_catch = [&](int ret_sc, int ret_mv )->int {
            // Inlets are run sequentially
            m.lock();
            ret_sc = -ret_sc;
            int prune = 0;

            if (!(ret_sc > bestscore)) {
                m.unlock();
                return prune;
            }

            if (ret_sc > bestscore) { 
                bestscore = ret_sc;
                bestmove = ret_mv;
                if (ret_sc >= g->beta) {
                    prune = 1;
                }
                if (ret_sc > g->alpha) g->alpha = ret_sc;
            }	

            m.unlock();
            return prune;
        };

        g->alpha = -INF;
        g->beta = INF;
        
        std::vector<ABState> next_moves;
        g->getPossibleStates(next_moves);
        /* search best move from previous iteration first */
        ABState* best_state = &next_moves[prev_move];
        root_search_catch( search( g, best_state, depth-1, global_abort), prev_move);
        
        /* cycle through all the moves */
        // #pragma cilk grainsize = 1
        for(int stateInd = 0; stateInd < next_moves.size(); stateInd++ ) {
            ABState* next_state = &next_moves[stateInd]; 
            if( stateInd != prev_move) { 
                root_search_catch( search(g, next_state, depth-1, global_abort),stateInd);
            }
        }
        //update best move for next iteration
        prev_move = bestmove;
        search_done = 1;
        return bestscore;
        
    }

    template <class ABState>
    int search(ABState *prev, ABState *next, int depth, Abort *parent_abort) {

        tbb::mutex m;
        int local_best_move = INF;
        int bestscore = -INF;
        int sc;
        int old_alpha = prev->alpha;
        int saw_rep = 0;
        std::vector<ABState> next_moves;
        Abort *local_abort = new Abort(parent_abort);
        
        auto search_catch = [&] (int ret_sc, int ret_mv )->int {
            m.lock(); 
            int prune = 0;
            ret_sc = -ret_sc;

            if (!(ret_sc > bestscore)){
                m.unlock();
                return 0;
            }

            if (ret_sc > bestscore) { 
                bestscore = ret_sc;
                local_best_move = ret_mv;
                if (ret_sc >= next->beta) {
                    prune = 1;
                }

                if (ret_sc > next->alpha) next->alpha = ret_sc;
            }		 
            m.unlock();

            return prune;
        };

        if (local_abort->isAborted()) {
            return 0;
        }
        
        nodec[__cilkrts_get_worker_number()]++;
        //   repeat mate test 
#ifdef REP
        //Code for repatition wins would follow, if needed
        //Rules for repeition vary from game to game
        //Some games its a loss if ANY repeats occurs, otehrs if repeated X times 
        // a row
        int count = 0;
        for ( ABState* rep = next->his; rep; rep = rep->his) {
            if (rep->key == next->key) {
                count++;
            }
        } 
        if(count == 2) {
            return REPWIN;
        }
#endif

        int ht_move = 0; 
#if HASH
        ttRec* entry = getEntry( &tt, next->key);
        if (entry) {
            if (recUsable(entry, depth, next->beta)) return(entry->score );
            ht_move = entry->move;
        }
#endif
        //too deep
        if (depth <= 0) {
            return next->evaluate();
        }

        next->getPossibleStates(next_moves);

        if(next_moves.size() == 0) {
            return next->evaluate(); //won game?
        }
        //flip AB values and search negamax
        next->alpha = -prev->beta;
        next->beta = -prev->alpha;

        // try best move  from hash first always 0 if no hashing 
        //paranoia check to make sure hash table isnot  malfunctioning
        if(next_moves.size() > ht_move ) {
            //focus all resources on searching this move first
            sc = search( next, &next_moves.at(ht_move), depth-1, local_abort);
            sc = -sc;
            if (sc > bestscore) { 
                bestscore = sc;
                local_best_move = ht_move;
                if (sc > next->alpha) next->alpha = sc;
                if (sc >= next->beta) {
                    //prune
                    return bestscore;
                }
            } 
        }
        else {
            //error inhash
        }

        /* cycle through all the moves */

        //if  aborted this result does not matter
        if (local_abort->isAborted()) {
            return 0;
        }

        // #pragma cilk grainsize = 1
        for(int stateInd = 0; stateInd < next_moves.size(); stateInd++ ) {
            if (stateInd != ht_move) {   /* don't try this again */
                ABState* next_state = &next_moves[stateInd]; 
                //search catch returns 1 if pruned
                search_catch( search(next, next_state, depth-1, local_abort), stateInd);
            }
        }
#if HASH
        //use hash table for one one color

        if (bestscore <= old_alpha) {
            putEntry( &tt, next->key, depth, bestscore, UPPER_BOUND, 0 );
        }  else if (bestscore >= next->beta) {
            putEntry( &tt, next->key, depth, bestscore, LOWER_BOUND, local_best_move );
        } else {
            putEntry( &tt, next->key, depth, bestscore, EXACT_SCORE, local_best_move );
        }
#endif
        
        return bestscore;

    }



};
#endif//header
