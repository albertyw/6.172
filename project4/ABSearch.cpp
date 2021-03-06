#include "ABSearch.h"

Abort *ABORT::global_abort;

namespace _ABSEARCH {
  KhetMove killer_moves[MAX_DEPTH];
  int curdepth;
  static Abort *global_abort;
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
    void(*f)(KhetMove best_move,int depth, int score ,int nodes, double time)){

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
  if(f) {
    (*f)(8, 0, 0, 0, 0);
  }
  //remember best move from iterations of root search
  //done in only this ABSearch
  prev_move = 0;

  //if (global_abort)
  //	delete global_abort;
  ABORT::global_abort = new Abort();
  global_abort = ABORT::global_abort;
  search_done = 0;
  cilk_spawn timer_thread();

  memset(killer_moves,INVALID_MOVE,sizeof(KhetMove)*MAX_DEPTH);

  //iterative deepening loop
  for (int depth=1; depth <= max_depth; depth++)
  {
    int     score;
    double  tt;
    int nc;

    curdepth = depth;
    assert(KhetState::checkKhetCache()==0);
    score = root_search( g, depth );
    assert(KhetState::checkKhetCache()==0);

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
  //we finished search before time. end timer_thread
  search_done = 1;
  return bestmove; 
}

/*
starts ABSearch assuming g is the root
need this special case because this inlet should update
global var bestmove
*/
int root_search(ABState *g, int depth) {
	 int          mv;
	 int          sc;
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
     
	 unsigned int num_moves = g->ks->getNumPossibleMoves();
/* search best move from previous iteration first */
   // KhetMove* best_state = next_moves[prev_move];
	 root_search_catch( search( g, g->ks->getMove(prev_move), depth-1, global_abort), prev_move);
	 
   /* cycle through all the moves */
	 #pragma cilk grainsize = 1
   cilk_for(int stateInd = 0; stateInd < num_moves; stateInd++ ) {
     // ABState* next_state = next_moves[stateInd]; 
     if( stateInd != prev_move) { 
	   if( root_search_catch( search(g, g->ks->getMove(stateInd), depth-1, global_abort),stateInd ) )
         global_abort->abort();
     }
   }
   //update best move for next iteration
	 prev_move = bestmove;
	 return bestscore;
	 
}

  int search(ABState *prev, KhetMove next_move, int depth, Abort *parentabort) {

    tbb::mutex m;
    int local_best_move = INF;
    int bestscore = -INF;
    int sc;
    int old_alpha = prev->alpha;
    int saw_rep = 0;
    unsigned int num_moves;
    ABState *next;
    KhetMove move;
	Abort localabort(parentabort);
	
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

	if (localabort.isAborted()) {
      return 0;
  }
	
	nodec[__cilkrts_get_worker_number()]++;

	next = new ABState(prev,next_move);

  //   repeat mate test 
#ifdef REP
  //Code for repatition wins would follow, if needed
  //Rules for repeition vary from game to game
  //Some games its a loss if ANY repeats occurs, otehrs if repeated X times 
  // a row
    int count = 0;
    for ( ABState* rep = next->his; rep; rep = rep->his) {
      if (rep->ks->key == next->ks->key) {
        count++;
      }
    } 
    if(count == 2) {
      delete next;
      return REPWIN;
    }
#endif

	int ht_move = 0; 
#if HASH
  ttRec* entry = getEntry( &tt, next->key);
   if (entry) {
     if (recUsable(entry, depth, next->beta))
	 {
	   delete next;
	   return(entry->score );
	 }
     ht_move = entry->move;
   }
#endif
	
  //too deep
  if (depth <= 0) {
    int ret = next->ks->evaluate();
    delete next;
    return ret;
  }
  num_moves = next->ks->getNumPossibleMoves();

  if(num_moves == 0) {
    int ret = next->ks->evaluate();
    delete next;
    return ret; //won game?
  }
  //flip AB values and search negamax
  next->alpha = -prev->beta;
  next->beta = -prev->alpha;
  
  // try best move  from hash first always 0 if no hashing 
  //paranoia check to make sure hash table isnot  malfunctioning
  if(num_moves > ht_move ) {
    //focus all resources on searching this move first
    move = next->ks->getMove(ht_move);
    sc = search( next, move, depth-1, &localabort);
    sc = -sc;
    if (sc > bestscore) { 
      bestscore = sc;
      local_best_move = ht_move;
      if (sc > next->alpha) next->alpha = sc;
      if (sc >= next->beta) {
        //prune
        killer_moves[curdepth-depth] = move;
        delete next;
        return bestscore;
      }
    } 
  }
  else {
    //error inhash
  }

  // KILLER MOVES
  move = killer_moves[curdepth-depth];
  if (move!=INVALID_MOVE && next->ks->isValidMove(move))
  {
    // try searching the killer move
    sc = search( next, move, depth-1, &localabort);
    sc = -sc;
    if (sc > bestscore) { 
      bestscore = sc;
      local_best_move = ht_move;
      if (sc > next->alpha) next->alpha = sc;
      if (sc >= next->beta) {
        //prune
        delete next;
        return bestscore;
      }
    }
  }

  /* cycle through all the moves */

	//if  aborted this result does not matter
  if (localabort.isAborted()) {
    delete next;
    return 0;
  }
	#pragma cilk grainsize = 1
   cilk_for(int stateInd = 0; stateInd < num_moves; stateInd++ ) {
    if (stateInd != ht_move) {   /* don't try this again */
      // ABState* next_state = next_moves[stateInd]; 
      //search catch returns 1 if pruned
      move = next->ks->getMove(stateInd);
      if( search_catch( search(next, move, depth-1,&localabort), stateInd) )
      {
        killer_moves[curdepth-depth] = move;
        localabort.abort();
      }
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
  delete next;
	return bestscore;

}

};

