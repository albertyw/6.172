#include "khet.h"

#define INFO(a) fprintf a
#define PERF_DEPTH 6
string best_move_buf;
void uci();

void inputThread() {
    char s[TOKEN_SIZE];
    istr.p = 0;
    istr.count = 0;

    while(fgets(s, 4095, stdin) != NULL) {
        inputMtx.lock();
        strcpy(istr.buf[istr.count], s);
        istr.count = (istr.count + 1) % I_BUF_SIZE;
        inputMtx.unlock();
    }
}

string getInput() {
    inputMtx.lock();
    if(istr.p == istr.count) {
        inputMtx.unlock();
        return "";
    }
    string s = string(istr.buf[istr.p]);
    istr.p = (istr.p + 1) % I_BUF_SIZE;
    inputMtx.unlock();
    return s;
}

void notate_helper(int best_move, int depth, int score, int nc, double tt) {
  best_move_buf = gameHis[ply].ks->getMove(best_move);
  INFO(( stdout, 
        "info depth %d time %7.1f score %6d  currmove %s nodes %d nps %7.1f \n", 
        depth, 
        tt, 
        score, 
        best_move_buf.c_str(), 
        nc,
        (nc / tt) ));
}
//uses old state and move to generate a new state and save it in newState
//returns 0 if ok, 1 otherwise
int uciMakeMove(_ABSEARCH::ABState* prevState, _ABSEARCH::ABState* newState, string move) {

	*newState = *prevState;
    //maintain history for repetition checking
    newState->his = prevState;
	int index = newState->ks->move(move);
	if (index<0) return 1;
	newState->ks = newState->ks->makeMove(index);
    return 0;
}



int parseString(string s, string (&tokens)[1024] ) {
    int count = 0;
    int i;
    char *token;
    char initCString[s.size() + 1];
    strcpy(initCString,s.c_str());
    //strtok breaks string into tokens separated by any of the delimiters
    //first call expects string, subsequent calls take null as arg
    for(i = 0, token = strtok(initCString, " \t\n\r");
    token != NULL;
    token = strtok(NULL, " \t\n\r"), i++) {
        tokens[i] = token;
        count++;
    }
    tokens[count] = "";
    return count;
}
/*
Starts a search of search_time milliseconds and outputs
best move in string format when done
*/
void start_search(int search_time, int depth) { 
    if(search_time == 0) search_time = 1;
    _ABSEARCH::ABSearch(&gameHis[ply], depth, search_time, notate_helper);

    //best-move_buf could be empty if you did not have time to complete a search
    //to depth 1. You should consider how this case should be handled
    cout << "bestmove " << best_move_buf << endl;
}
/*
Process uki commmands from input buffer
*/

#ifdef PERF
int count = 0;
#endif

void uci() {
    while( 1 ) {
        usleep(200);
        string s;
#ifdef PERF
        if (count==0) s = "uki";
        else if (count == 1) s = "position classic";
        else if (count == 2) {
           
            stringstream perf;
            perf  << PERF_DEPTH;
            s = "go depth ";
            s.append(perf.str());
        }
        ++count;
#endif
        string tokens[1024];
#ifndef PERF
        s = getInput();
#endif
        int token_count = parseString(s, tokens);
        
        if(token_count == 0) {
            continue;
        }

        if(tokens[0].compare("uki") == 0) {
            cout << "id name Khet "<< version << endl;
            cout << "id author YOUR NAME" << endl;
            cout << "ukiok" << endl;
            continue;
        }
        else if(tokens[0].compare("isready") == 0) {
            cout << "readyok" << endl;
            continue;;
        }
        else if(tokens[0].compare("ukinewgame") == 0) {
            continue;
        }
        else if(tokens[0].compare("setoption") == 0) {
            continue;
        }
        else if(tokens[0].compare("position")== 0) {
            ply = 0;
			string* boardstr = gameHis[ply].ks->init(tokens[1]);
            if(boardstr == 0) {
                cout << "invalid input position" << endl;
                exit(1);
            }
			gameHis[ply].ks = KhetState::getKhetState(*boardstr);
            //make all moves inlist
            if(token_count > 2 && tokens[2].compare("moves") != 0) {
                cout << "invalid input position" << endl;
                exit(1);
            }
            for(int i = 3; i < token_count; i++) {
                if(uciMakeMove(&gameHis[ply], &gameHis[ply + 1], tokens[i]) != 0 ) {
                    cout << s <<" Invalid move:" << tokens[i] << endl;
                    exit(1);
                }
                ply++;
            }

        }
        else if(tokens[0].compare("go")== 0) {
            int search_time = -1; //indicates infintie
            int increment = 0;
            int depth = 10;
            for(int i = 1; i < token_count; i ++) {
                if(tokens[i].compare("wtime") == 0) {
                    search_time = strtol(tokens[i+1].c_str(), NULL, 10) ;
                    i++;
                }
                else if(tokens[i].compare("btime") == 0) {
                    search_time = strtol(tokens[i+1].c_str(), NULL, 10) ;
                    i++;
                }
                else if(tokens[i].compare("infinite") == 0) {
                    search_time = -1;
                }
                else if(tokens[i].compare("winc") == 0) {
                    increment = strtol(tokens[i+1].c_str(), NULL, 10);
                    i++;
                }
                else if(tokens[i].compare("binc") == 0) {
                    increment = strtol(tokens[i+1].c_str(), NULL, 10);
                    i++;
                }
                else if(tokens[i].compare("depth") == 0) {
                    depth = atoi(tokens[i+1].c_str());
                    i++;
                }
            }
            //try to use only a bit of your remaining time per move
            if(search_time != -1)
            search_time = search_time * .02 + (increment/1000) * 0.8;
            cilk_spawn start_search(search_time, depth);
#ifdef PERF
            cilk_sync;
            exit(0);
#endif
            continue;
        }
        else if(tokens[0].compare("stop")== 0) {
            _ABSEARCH::abortSearch();
        }
        //prints out string format of board
        else if(tokens[0].compare("display")== 0) {
            string bd = gameHis[ply].ks->getBoardPrettyStr();
            cout << bd << endl ;
        }
        //forces a move generation of current board
        else if(tokens[0].compare("gen")== 0) {
            cout << gameHis[ply].ks->gen() << endl;
        }
        else if(tokens[0].compare("quit")== 0) {
            exit(0);
        }
        //Not part of uki, prints out list of all possible moves form current state
        else if(tokens[0].compare("debug")== 0) {
            gameHis[ply].ks->debugMoves();
        }
        else if(tokens[0].compare("eval")== 0) {
            cout << gameHis[ply].ks->evaluate() <<endl;
        }
        //The perft command is not part of UKI
        //prints out the counts of possible moves at from current position at depth i
        //ie the leaves of the game tree. Good move gen debug method
        else if(tokens[0].compare("perft")== 0) {
            int depth = 1;
            for(;depth < atoi(tokens[1].c_str()); depth++) {
                uint64_t nodec = gameHis[ply].ks->perft(depth);
                cout << "Node count for depth " << depth << " is " << nodec << endl;
            }
        }
        else {
            cout << "Unrecognized UCI command: "<< tokens[0] << endl;
        }

    }
}

/* Public domain code for JLKISS64 RNG - long period KISS RNG producing 64-bit results */
unsigned long long myrand()
{
    static int first_time = 0;
    static unsigned long long x = 123456789123ULL,y = 987654321987ULL;     /* Seed variables */
    static unsigned int z1 = 43219876, c1 = 6543217, z2 = 21987643, c2 = 1732654; /* Seed variables */
    static unsigned long long t;

    if (first_time) {
        int  i;
        FILE *f = fopen( "/dev/urandom", "r" );
        for (i=0; i<64; i+=8 ) {
            x = x ^ getc(f) << i;
            y = y ^ getc(f) << i;
        }

        fclose(f);
        first_time = 0;
    }

    x = 1490024343005336237ULL * x + 123456789;
    y ^= y << 21; y ^= y >> 17; y ^= y << 30; /* Do not set y=0! */
    t = 4294584393ULL * z1 + c1; c1 = t >> 32; z1 = t;
    t = 4246477509ULL * z2 + c2; c2 = t >> 32; z2 = t;
    return x + y + z1 + ((unsigned long long)z2 << 32); /* Return 64-bit result */
}

uint64_t KhetState::zob[FILE_COUNT][RANK_COUNT][150];
uint64_t KhetState::ABzob[500];
//initalizes input thread and then starts UCI loop
int main(int argc, char *argv[])
{
    //prepare our hash table
    for (int i = 0; i < FILE_COUNT; i++) {
        for (int j = 0; j < RANK_COUNT; j++) {
            //k represents the enumeration of all possible khetpieces as an int
            //see khetpiece::id()
            for (int k = 0; k < 150; k++) {
                KhetState::zob[i][j][k] = myrand();
            }
        }
    }

    for (int i=0; i<500; i++) {
        KhetState::ABzob[i] = myrand();
    }

    //this will be reading input in the background
    cilk_spawn inputThread();
    ply = 0;
    uci();
    return 0;
}
