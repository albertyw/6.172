/*
  Represents a game state in Khet.
  Contains layout of board, and can generate possible subsequent states
*/
#ifndef KHET_STATE_HDR
#define KHET_STATE_HDR

#include <string>
#include <string.h>
#include <vector>
//#include "ABSearch.h"
//#include "ABState.h"
#include "eval.h"
#include "globals.h"
#include <locale>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <map>


using namespace std;

static locale loc;
static string silverLetters = "-ASYPH-";
static string redLetters = "-asyph-";
static string pieceLetters = "PHSSAAYYYYYYYphssaayyyyyyy--";
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
// static string classicOpen = "hd------adpdadyd----"  
//                             "----yl--------------"  
//                             "------Yu------------" 
//                             "yr--Yl--slsu--yd--Yu" 
//                             "yd--Yu--SuSl--yr--Yl" 
//                             "------------yd------" 
//                             "--------------Yr----" 
//                             "----YuAuPuAu------Hu w";

// static string dynastyOpen = "hd------yladyd------"
//                             "----------pd--------"
//                             "yr------yladsd------"
//                             "yd--sr--Yu--Yd------"
//                             "------yu--yd--Sl--Yu"
//                             "------SuAuYr------Yl"
//                             "--------Pu----------"
//                             "------YuAuYr------Hu w";

// static string imhotepOpen = "hd------adpdadsd----"  
//                             "--------------------"  
//                             "------Yu----yr------" 
//                             "yrYl----Ydsu----ydYu" 
//                             "ydYu----Suyu----yrYl" 
//                             "------Yl----yd------" 
//                             "--------------------" 
//                             "----SuAuPuAu------Hu w";

                                                   
                                             
static string classicOpen = "o3------r3n3s3t3----"
                            "----u2--------------"
                            "------g1------------"
                            "v0--h2--p2q1--w3--i1"
                            "x3--j1--c1d2--y0--k2"
                            "------------z3------"
                            "--------------l0----"
                            "----m1e1a1f1------b1 w";

static string dynastyOpen = "o3------t2r3u3------"
                            "----------n3--------"
                            "v0------w2s3p3------"
                            "x3--q0--g1--h3------"
                            "------y1--z3--c2--i1"
                            "------d1e1j0------k2"
                            "--------a1----------"
                            "------l1f1m0------b1 w";

static string imhotepOpen = "o3------r3n3s3p3----"
                            "--------------------"
                            "------g1----t0------"
                            "u0h2----i3q1----v3j1"
                            "w3k1----c1x1----y0l2"
                            "------m2----z3------"
                            "----d1e1a1f1------b1 w";

class KhetState {
  public:

    static int checkKhetCache();
    int checkKhetBoard();
	static KhetState* getKhetState(uint64_t key);
    static KhetState* getKhetState(KhetState* s, KhetMove mv);
    static KhetState* getKhetState(KhetState* s, int mvi);
    static KhetState* getKhetState(string b);

    KhetState();
    KhetState(KhetState* s, KhetMove mv);
    KhetState(KhetState* s, int mvi);
    KhetState(string b);
    int init(string b);
    string getBoardStr();
    string getBoardPrettyStr();

    //functions needed by ABState
    int evaluate();
    void getPossibleStates(std::vector<KhetState*> &v);
    void getPossibleMoves(std::vector<KhetMove> &v);
    unsigned int getNumPossibleMoves();

    string getMove(int i);
    string getCtmStr();

    
    static uint64_t zob[FILE_COUNT][RANK_COUNT][150];
    static KhetPiece evalboard[FILE_COUNT][RANK_COUNT];
    //attempts to make move in the from of "a8a7" or "a8r" in algstr
    //returns empty str if move is invalid, returns algrstr otherwise
    int makeMove(string algstr);
	KhetState* makeMove(KhetMove mv);
	KhetState* makeMove(int index);

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
    static LaserHitInfo fireLaser(int tFile, int tRank, Rotation laserDir,
                                    int closestToFile, int closestToRank);
    
    static map<uint64_t,KhetState*> khet_cache;
    uint64_t key;
    
  private:
    inline
    static bool isOppositeDirections(Rotation dir1, Rotation dir2)
    {
      return (((int)dir1)^((int) dir2)) == 2;
    }

    uint64_t hashBoard();

    bool moves_init;
    Board board;
    PlayerColor ctm; //color to move
    // KhetPiece strToPiece(string sq);
    void initBoard(string board); 
    vector<KhetMove> moves;
    //performs move on this state, assumes move is valid
    void imake(KhetMove mv);
    //converts a move into str notation
    string alg(KhetMove mv);
    bool gameOver;
    PlayerColor winner;
};

#endif //KHET_STATE_HDR
