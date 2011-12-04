/*
Evaluates a KhetState to an integer value
The heuristics used are based off the idea that some pieces do not
want to be in the middle, and that you dont want the laser going
near your pharaoh. Also some pieces benefit from being on the edges

While there probably are better evaluators, this was the best found
by Don Dailey. 
 */
#ifndef EVAL_HDR
#define EVAL_HDR

#include "globals.h"
#include "KhetState.h"
//distance to closest corner, normalized by subtracting 3
static int center[FILE_COUNT][RANK_COUNT] = {{-3, -2, -1, 0, 0, -1, -2, -3},
  {-2, -1, 0, 1, 1, 0, -1, -2},
  {-1, 0, 1, 2, 2, 1, 0, -1},
  {0, 1, 2, 3, 3, 2, 1, 0},
  {1, 2, 3, 4, 4, 3, 2, 1},
  {1, 2,3 ,4, 4, 3, 2, 1},
  {0, 1,2 , 3, 3, 2, 1, 0},
  {-1, 0, 1, 2, 2, 1, 0, -1},
  {-2, -1, 0, 1, 1, 0, -1, -2},
  {-3, -2, -1, 0, 0, -1, -2, -3}};
#define pyVal 100               //pyramid value

#define enemyLaserDistance -65 //shoot laser and find closest distance to enemy 
                              //pharaoh, use manhattan dist(x + y).

#define friendlyLaserDistance 5 //as above, but for friendly pharaoh

#define anVal 230               //value of anubis
#define phPST -9 //piece square table for pharaoh. Dont want him near the middle
#define scPST 10 //scarab piece square table
#define anPST -5 //Anubis table
#define pyPST 20 //pyramid table

#define pyEdge 10 //pyramid bonus for being on edge of baord
#define scEdge 10 //scarab bonus for being on edge of baord
#define exposed -120 //if laser fired from pharaoh hits mirroed surface
#define pyMob 23 // count empty squares around pyramid, and multiply

int eval(Board b, PlayerColor ctm);
int adjacentEmptySquares(Board b, int file, int rank);
bool isEdge(int file, int rank);
#endif //EVAL_HDR
