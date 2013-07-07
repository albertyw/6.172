#ifndef ABSTATE_H
#define ABSTATE_H


#include "KhetState.h"
#include <vector>
/*
Interface for user defined search states
Users must define the following two functions

evaluate: A static evalutor function fot the state.

getPossibleStates: returns a vector of states that are possible transitions
from this state. The states ordering must be deterministic, every call should return
the same states in the same order for a given state

The search function will call getPossibleStates to get a list of states to 
search, but we need a vector of the *derived* class. We cannot have a vector
of the base class, only a vector of pointers to the base class. Since the 
derived class must be invoked to create the list, using pointers to the base
class would require heap allocation, which is unacceptably slow.

Instead the derived class inherits from a template base class using itself as
the template parameter. The search functions are also template functions,
allowing the compiler to generate appropriate code. See

http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
*/

typedef uint64_t u64;
class ABState {
public:
  KhetState *ks;

	//These fields are used by the Search gunctions and so must be public
  //use friendship instead?
  int alpha;
  int beta;
  //this key is used for ABSearches hashtable, should be update appropriately
  //by derived class code
  u64     key;

  ABState  *his;         /* pointer to last position for repeat check*/
  int     ply_of_game;

  ABState(KhetState* _ks) {
    ks = _ks;
    ply_of_game = 0;
    key = ks->key;//^ply_of_game;
    his = 0;
  }

  ABState(ABState *prev, KhetMove mv) {
    ks = prev->ks->makeMove(mv);
    ply_of_game = prev->ply_of_game+1;
    key = ks->key;//^ply_of_game;
    his = prev;
  }

  ABState(ABState *prev, int move) {
    ks = prev->ks->makeMove(move);
    ply_of_game = prev->ply_of_game+1;
    key = ks->key;//^ply_of_game;
    his = prev;
  }

  ABState() {
    his = 0;
    ks = new KhetState();
  }

  ~ABState() {
    delete ks;
  }
};


#endif // HEADER
