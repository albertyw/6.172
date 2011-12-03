/*
Represents a game state in Khet.
Contains layout of board, and can generate possible subsequent states
*/
#ifndef KHET_STATE_HDR
#define KHET_STATE_HDR

#include <string>
#include <vector>
#include "ABState.h"
#include "eval.h"
#include "globals.h"
#include <locale>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <map>


using namespace std;

static locale loc;
static string silverLetters = "-ASYPH-";
static string redLetters = "-asyph-";
static string rotationLetters = "ruld";

static string fileLetters = "abcdefghij";
/*
    board rep is 
        hd -  -  -  ad pd ad yd -  -
        -  -  yl -  -  -  -  -  -  -
        -  -  -  Yu -  -  -  -  -  -
        yr -  Yl -  sl su -  yd -  Yu
        yd -  Yu -  Su Sl -  yr -  Yl
        -  -  -  -  -  -  yD -  -  -
        -  -  -  -  -  -  -  Yr -  -
        -  -  Yr Au Pu Au -  -  -  Hu w
    
    where h -> Sphinx
        a -> Anubis
        p -> Pharoah
        y -> Pyramid
        s -> Scarab

    the letters u,d,l,r represent rotation of piece with down
    representing rotation with mirror facing bottom right corner  
    lowercase letters for pieces indicate black(red in Khet) player(top)
The last element in the string is a space followed by the  color to move
*/

//str[0] should correspond to rank 8 file a ie top left
static string classicOpen = "hd------adpdadyd----"  
"----yl--------------"  
"------Yu------------" 
"yr--Yl--slsu--yd--Yu" 
"yd--Yu--SuSl--yr--Yl" 
"------------yd------" 
"--------------Yr----" 
"----YuAuPuAu------Hu w";

static string dynastyOpen = "hd------yladyd------"
"----------pd--------"
"yr------yladsd------"
"yd--sr--Yu--Yd------"
"------yu--yd--Sl--Yu"
"------SuAuYr------Yl"
"--------Pu----------"
"------YuAuYr------Hu w";

static string imhotepOpen = "hd------adpdadsd----"  
"--------------------"  
"------Yu----yr------" 
"yrYl----Ydsu----ydYu" 
"ydYu----Suyu----yrYl" 
"------Yl----yd------" 
"--------------------" 
"----SuAuPuAu------Hu w";

#define KHET_CACHE_SIZE 1<<26
#define KHET_CACHE_MASK 0x3ffffff


typedef struct CacheEntry {
    uint64_t key;
    void* obj;
} CacheEntry;




class KhetState {
public:


    static KhetState* getKhetState(uint64_t key);
    static KhetState* getKhetState(KhetState* s, KhetMove* mv);
    static KhetState* getKhetState(string b);


    KhetState();
    KhetState(KhetState* s, KhetMove* mv);
    KhetState(string b);

    int init(string b);
    string getBoardStr();
    string getBoardPrettyStr();

    //functions needed by ABState
    int evaluate();
    void getPossibleStates(std::vector<KhetMove> &v);
    KhetState* makeMove(KhetMove m);

    string getMove(int i);
    KhetMove getMove2(int i);
    string getCtmStr();

    
    static uint64_t zob[FILE_COUNT][RANK_COUNT][150];
    //attempts to make move in the from of "a8a7" or "a8r" in algstr
    //returns 1 if move is invalid, returns 0 otherwise
    int move(string algstr);
    int move(KhetMove move);
    

    bool isWon();
    //counts the number of possible states up to depth. Useful for 
    //debugging move generator
    uint64_t perft(int depth);
    //generates all possible mvoes and stores them in moves vector
    long gen();
    void debugMoves();

    //fires laser on board from tFile tRank in laserDirection
    //closestToFile and rank are used to measure closest distance from any 
    //pt on lasers path to this point. Used by some eval functions
    //returns a LaserHitInfo with information about hit piece if any
    static LaserHitInfo fireLaser(Board board, int tFile, int tRank, Rotation laserDir,
    int closestToFile, int closestToRank);
    //static CacheEntry khet_cache[KHET_CACHE_SIZE];
	static map<uint64_t,KhetState*> khet_cache;

    u64 key;
	
private:
    inline
    static bool isOppositeDirections(Rotation dir1, Rotation dir2)
    {
      return (((int)dir1)^((int) dir2)) == 2;
    }
    uint64_t hashBoard();
    Board board;
    PlayerColor ctm; //color to move
    KhetPiece strToPiece(string sq);
    void initBoard(string board); 
    
    vector<KhetMove> moves;
    bool moves_init;

    //performs move on this state, assumes move is valid
    void imake(KhetMove mv);
    //converts a move into str notation
    string alg(KhetMove mv);
    bool gameOver;
    PlayerColor winner;
    
};

#endif //KHET_STATE_HDR
