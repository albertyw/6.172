#include "eval.h"

int eval(Board b, PlayerColor ctm) {
  int eval_score = 0;
  int sPharaohRank = 0;
  int sPharaohFile = 0;
  int rPharaohRank = 0;
  int rPharaohFile = 0;

  int redScore = 0;
  int silverScore = 0;

  //piece score calc
  int rC = 0;
  int sC = 0;
  for(int file = 0; file < FILE_COUNT; file++) {
    for(int rank = 0; rank < RANK_COUNT; rank++) {
      int center_value = center[file][rank];
      int piece_value = 0; 
      int pst = 0;
      int pieceScore = 0;
      KhetPiece piece = b[file][rank]; 
      switch(piece.type) {
        case SCARAB:
          pst = scPST;
          if(isEdge(file,rank)) pieceScore += scEdge;
          break;
        case SPHINX:
          break;
        case PHAROAH:
          pst = phPST;
          if(piece.color == SILVER) {
            sPharaohRank = rank;
            sPharaohFile = file;
          }
          else {
            rPharaohRank = rank;
            rPharaohFile = file;
          }
          break;
        case PYRAMID:
          pst = pyPST;
          piece_value = pyVal;
          pieceScore += pyMob * adjacentEmptySquares(b, file, rank);
          if(isEdge(file,rank)) pieceScore += pyEdge;
          break;
        case ANUBIS:
          pst = anPST;
          piece_value = anVal;
          break;
        case EMPTY:
          continue;
          break;
      }
      
      pieceScore += piece_value + (center_value * pst); 
      
      if (piece.color == SILVER) {
        silverScore += pieceScore;
      }
      else {
        redScore += pieceScore;
      }
    }
  }
  int rSphRank = 7;
  int rSphFile = 0;
  int sSphRank = 0;
  int sSphFile = 9;
  LaserHitInfo lInfo;
  
  //exposure checks 
  for(int rot = RIGHT; rot != (DOWN + 1); ++rot) {
    lInfo = KhetState::fireLaser(b, rPharaohFile, rPharaohRank,
      (Rotation)rot, 0, 0);
    if(lInfo.bounced) {
      redScore += exposed;
      break;
    }
  }
  for(int rot = RIGHT; rot !=( DOWN + 1); ++rot) {
    lInfo = KhetState::fireLaser(b, sPharaohFile, sPharaohRank,
      (Rotation)rot, 0, 0);
    if(lInfo.bounced)  {
      silverScore += exposed;
      break;
    }
  }
//measure closest distance of laser to pharaohs
  Rotation sSphRot = b[sSphFile][sSphRank].rot;

  lInfo = KhetState::fireLaser(b, sSphFile, sSphRank,
      sSphRot, sPharaohFile, sPharaohRank);

  silverScore += friendlyLaserDistance * lInfo.closest;
 
  lInfo = KhetState::fireLaser(b, sSphFile, sSphRank,
      sSphRot, rPharaohFile, rPharaohRank);

  silverScore += enemyLaserDistance * lInfo.closest;
//closest pharaoh for red
  Rotation rSphRot = b[rSphFile][rSphRank].rot;

  lInfo = KhetState::fireLaser(b, rSphFile, rSphRank,
      rSphRot, rPharaohFile, rPharaohRank);

  redScore += friendlyLaserDistance * lInfo.closest;
  
  lInfo = KhetState::fireLaser(b, rSphFile, rSphRank,
      rSphRot, sPharaohFile, sPharaohRank);

  redScore += enemyLaserDistance * lInfo.closest;
  
  //weight for right side
  if(ctm == RED) {
    return redScore - silverScore;
  }
  else  {
    return silverScore - redScore;
  }
}

bool isEdge(int file, int rank) {
  if (file == 9 || file == 0) return true;
  if (rank == 7 || rank == 0) return true;
  return false;
} 

int adjacentEmptySquares(Board b, int file, int rank) {
  int count = 0;
  for(int fileOffset = -1; fileOffset < 2; fileOffset++) {
    for(int rankOffset = -1; rankOffset < 2; rankOffset++) {
      int toFile = fileOffset + file;
      int toRank = rank + rankOffset;
      if(toFile > 9 || toFile < 0) {
        continue;
      }
      if(toRank > 7 || toRank < 0) { 
        continue;
      }
      if(rankOffset == 0 && fileOffset == 0) continue;
      if(b[toFile][toRank].type == EMPTY) count++;
    }
  }
  return count;
}
