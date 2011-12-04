#ifndef ABSTATE_H
#define ABSTATE_H

#include <vector>
namespace _ABSEARCH {
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
template<class GameState>
class ABState {
public:
	void getPossibleStates(std::vector<GameState> &v ) {
    static_cast<GameState*>(this)->getPossibleStates(v);
  }
	int evaluate() {
    return static_cast<GameState*>(this)->evaluate();
  }
	//These fields are used by the Search gunctions and so must be public
  //use friendship instead?
  int alpha;
  int beta;
  //this key is used for ABSearches hashtable, should be update appropriately
  //by derived class code
  u64     key;
  ABState  *his;         /* pointer to last position for repeat check*/
  int     ply_of_game;
};

}

#endif // HEADER
