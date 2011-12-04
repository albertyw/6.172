#ifndef GLOBALS_HDR
#define GLOBALS_HDR

#include <iostream>
#define FILE_COUNT 10
#define RANK_COUNT 8

typedef uint64_t u64;
using namespace std;
enum PieceType {
  ANUBIS = 1,
  SCARAB = 2,
  PYRAMID = 3,
  PHAROAH = 4,
  SPHINX = 5,
  EMPTY = 6
};

enum PlayerColor {
  SILVER = 0,
  RED = 1
};

enum Rotation {
  RIGHT = 0,
  UP = 1,
  LEFT = 2,
  DOWN = 3,
};

typedef struct KhetPiece {
  PieceType type;
  Rotation rot;
  PlayerColor color;
  int id() {
    return (((int)color))|(((int)rot)<<1)|(((int)type))<<3;
  }
} KhetPiece;

typedef struct LaserHitInfo {

  int hitFile;
  int hitRank;
  KhetPiece hitPiece;
  Rotation laserDir;

  int closest; //closest manhattan distance of any pt on laser path
              // to some target file and rank
  bool bounced; //true if laser was reflected at any point

} LaserHitInfo;


typedef struct KhetMove {
  KhetPiece piece;
  int fromFile;
  int fromRank;
  int fromRot;
  int toFile;
  int toRank;
  int toRot;
  KhetMove(KhetPiece p, int fFile, int fRank, int fRot, int tFile, int tRank,
      int tRot) {
    piece = p;
    fromFile = fFile;
    fromRank = fRank;
    fromRot = fRot;
    toFile = tFile;
    toRank = tRank;
    toRot = tRot;
  }
  void debugMove() {
    cout << "fromFile" << fromFile;
    cout << " fromRank" << fromRank;
    cout << " fromRot" << fromRot;
    cout << " toFile" << toFile;
    cout << " toRank" << toRank;
    cout << " toRot" << toRot << endl;
  }
} KhetMove;

inline
bool operator==(const KhetMove lhs, const KhetMove rhs)
{
    return lhs.fromFile == rhs.fromFile &&
            lhs.fromRank == rhs.fromRank &&
            lhs.fromRot == rhs.fromRot &&
            lhs.toFile == rhs.toFile &&
            lhs.toRank == rhs.toRank &&
     lhs.toRot == rhs.toRot;
}


typedef KhetPiece Board[FILE_COUNT][RANK_COUNT];


#endif
