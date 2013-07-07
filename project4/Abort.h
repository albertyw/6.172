
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
#ifndef ABORT_H
#define ABORT_H 1

#ifdef STATS
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
cilk::reducer_opadd<unsigned int> abort_count;
cilk::reducer_opadd<unsigned int> poll_count;
#endif
#include <tbb/mutex.h>
#include <stdio.h>
/*
Abort class for speculative computation
Functions can use Abort obejcts to signal termination to spawned
computations

Ex.
void pFunc() {
  Abort rootAbort = abort(); //create an Abort token
  cilk_spawn child(&rootAbort); //spawn child computations
  cilk_spawn child(&rootAbort);
  rootAbort.abort(); //terminate them
}

void child(Abort* pAbort) {
  Abort localAbort = abort(pAbort); //use copy constructor to create a parent
                                    //child relation ship. Abort at parent ends
                                    //child computations.
  //spin and wait for parent to signal termination
  while(1) {
    if(localAbort.isAborted()) return;
  }
}

Implementation is done via polling, where a child polls its parent recursively
to see if they should end. O(h) in height of tree of spawned computations
abort() sets a local flag to signify termination. Does nothing to children O(1)

While other approaches to doing polling exist, see attached paper for analysis
of tradeoffs

Because of polling, it is up to user code to have polling calls, as well as
deal with return values of aborted computations. typically a function is called
with the return value of such computatiosn, which includes logic for 
determining if the value is valid, and/or signalling abort. See ABSearch.h for 
an example
 */
class Abort
{
	public:
		Abort() : aborted(false), parent(NULL) {
			pollGranularity = 1;
      count = 0;
		}

		explicit Abort(Abort *p) : aborted(false), parent(p) {
			 pollGranularity = 1;
			 count = 0;
		}
		void setGranularity(int newGranularity) {
			pollGranularity = newGranularity;
		}

		int isAborted() {
      //allows for polling every i'th call
      if(count >= pollGranularity - 1 ) {
				count = 0;
			}
			else {
				count++;
				return 0;
			}
			return aborted || (parent && this->parent->isAborted());
		}

		void abort() {
#ifdef STATS
			if(!aborted){
				abort_count += 1;
			}
#endif
			aborted = true;
		}

	private:
		Abort *parent; 
		bool aborted;
		int pollGranularity;
		int count;
};

#endif //abort_H
