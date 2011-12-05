#ifndef GLOBALS_HDR
#define GLOBALS_HDR

#include <iostream>
#define FILE_COUNT 10
#define RANK_COUNT 8

typedef uint64_t u64;
using namespace std;
enum PieceType {
  EMPTY       = 26,
  SPHAROH     = 0,
  SSPHINX     = 1,
  SSCARAB1     = 2,
  SSCARAB2     = 3,
  SANUBIS1    = 4,
  SANUBIS2     = 5,
  SPYRAMID1     = 6,
  SPYRAMID2     = 7,
  SPYRAMID3     = 8,
  SPYRAMID4     = 9,
  SPYRAMID5     = 10,
  SPYRAMID6     = 11,
  SPYRAMID7     = 12,
  RPHAROH     = 13,
  RSPHINX     = 14,
  RSCARAB1     = 15,
  RSCARAB2     = 16,
  RANUBIS1     = 17,
  RANUBIS2     = 18,
  RPYRAMID1     = 19,
  RPYRAMID2     = 20,
  RPYRAMID3     = 21,
  RPYRAMID4     = 22,
  RPYRAMID5     = 23,
  RPYRAMID6     = 24,
  RPYRAMID7    = 25,

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

// typedef struct KhetPiece {
//   PieceType type;
//   Rotation rot;
//   PlayerColor color;
//   int id() {
//     return (((int)color))|(((int)rot)<<1)|(((int)type))<<3;
//   }
// } KhetPiece;

#define KhetPiece uint16_t
// if empty, i.e. piece is dead, = 0
// 9-13      type
// 7-8       rot
// 3-6       file
// 0-2       rank
inline
KhetPiece makeKhetPiece(int type, int rot, int file, int rank)
{
  return (((int)type)<<9)|(((int)rot)<<7)|(file<<3)|(rank);
}

inline 
KhetPiece moveKhetPiece(KhetPiece piece, int file, int rank)
{
  return (piece&0x3f80)|(file<<3)|rank;
}

inline 
KhetPiece rotateKhetPiece(KhetPiece piece, int rot)
{
  return (piece&0x3e7f)|(rot<<7);
}

inline
PieceType getType(KhetPiece p)
{
  return (PieceType)(p>>9);
}

inline
Rotation getRot(KhetPiece p)
{
  return (Rotation)((p>>7)&0x3);
}

inline
unsigned int getFile(KhetPiece p)
{
  return (p>>3)&0xf;
}

inline
unsigned int getRank(KhetPiece p)
{
  return (p&0x7);
}

inline
unsigned int getID(KhetPiece p)
{
  return p>>7;
}


typedef struct LaserHitInfo {

  int hitFile;
  int hitRank;
  KhetPiece hitPiece;
  Rotation laserDir;

  int closest; //closest manhattan distance of any pt on laser path
              // to some target file and rank
  bool bounced; //true if laser was reflected at any point

} LaserHitInfo;


// typedef struct KhetMove {
//   KhetPiece piece;
//   int fromFile;
//   int fromRank;
//   int fromRot;
//   int toFile;
//   int toRank;
//   int toRot;
//   KhetMove(KhetPiece p, int fFile, int fRank, int fRot, int tFile, int tRank,
//       int tRot) {
//     piece = p;
//     fromFile = fFile;
//     fromRank = fRank;
//     fromRot = fRot;
//     toFile = tFile;
//     toRank = tRank;
//     toRot = tRot;
//   }
//   void debugMove() {
//     cout << "fromFile" << fromFile;
//     cout << " fromRank" << fromRank;
//     cout << " fromRot" << fromRot;
//     cout << " toFile" << toFile;
//     cout << " toRank" << toRank;
//     cout << " toRot" << toRot << endl;
//   }
// } KhetMove;


#define KhetMove uint32_t
//typedef uint32_t KhetMove;
// bits      object
// 23-27     fPiece - piecetype
// 18-22     tPiece - piecetype
// 14-17     fFile
// 11-13     fRank
// 9-10      fRot
// 5-8       tFile
// 2-4       tRank
// 0-1       tRot
inline
KhetMove makeKhetMove(int fFile, int fRank, int fRot, int tFile, int tRank,int tRot, int fPiece, int tPiece) {
  return (uint32_t)((fPiece<<23)|(tPiece<<18)|(fFile<<14)|(fRank<<11)|(fRot<<9)|(tFile<<5)|(tRank<<2)|(tRot));
}

inline
unsigned int getFromPiece(KhetMove mv)
{
  return (mv>>23);
}

inline
unsigned int getToPiece(KhetMove mv)
{
  return (mv>>18)&0x1f;
}

inline
unsigned int getFromFile(KhetMove mv)
{
  return (mv>>14)&0xf;
}

inline
unsigned int getFromRank(KhetMove mv)
{
  return (mv>>11)&0x7;
}

inline
unsigned int getFromRot(KhetMove mv)
{
  return (mv>>9)&0x3;
}

inline
unsigned int getToFile(KhetMove mv)
{
  return (mv>>5)&0xf;
}

inline
unsigned int getToRank(KhetMove mv)
{
  return (mv>>2)&0x7;
}

inline
unsigned int getToRot(KhetMove mv)
{
  return (mv)&0x3;
}

// inline
// bool operator==(const KhetMove lhs, const KhetMove rhs)
// {
//     return lhs.fromFile == rhs.fromFile &&
//             lhs.fromRank == rhs.fromRank &&
//             lhs.fromRot == rhs.fromRot &&
//             lhs.toFile == rhs.toFile &&
//             lhs.toRank == rhs.toRank &&
//      lhs.toRot == rhs.toRot;
// }


// typedef KhetPiece Board[FILE_COUNT][RANK_COUNT];
typedef KhetPiece Board[26];


#endif
