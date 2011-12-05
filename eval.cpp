#include "eval.h"

KhetPiece KhetState::evalboard[FILE_COUNT][RANK_COUNT];

int eval(Board b, PlayerColor ctm) {
  int eval_score = 0;
  int sPharaohRank = 0;
  int sPharaohFile = 0;
  int rPharaohRank = 0;
  int rPharaohFile = 0;

  int redScore = 0;
  int silverScore = 0;

  KhetPiece kp;
  unsigned int file;
  unsigned int rank;
  unsigned int type;

  memset(KhetState::evalboard,sizeof(KhetPiece)*80,0);

  for (int x=0; x<26; x++)
  {
    kp = b[x];
    if (kp==0) continue;
    file = getFile(kp);
    rank = getRank(kp);
    KhetState::evalboard[file][rank] = kp;
  }

  //piece score calc
  int rC = 0;
  int sC = 0;
  for(int x = 0; x < 26; x++) {
    kp = b[x];
    if (kp==0) continue;
    file = getFile(kp);
    rank = getRank(kp);
    int center_value = center[file][rank];
    int piece_value = 0; 
    int pst = 0;
    int pieceScore = 0;

    type = (unsigned int) getType(kp);

    if (type<13) // silver
    {
      switch(getType(kp)) {
      case SPHAROH : 
        pst = phPST;
        sPharaohRank = rank;
        sPharaohFile = file;
        break;
      case SSPHINX : 
        break;
      case SANUBIS1: 
      case SANUBIS2: 
        pst = anPST;
        piece_value = anVal;
        break;
      case SSCARAB1: 
      case SSCARAB2: 
        pst = scPST;
        if(isEdge(file,rank)) pieceScore += scEdge;
        break;
      case SPYRAMID1: 
      case SPYRAMID2: 
      case SPYRAMID3: 
      case SPYRAMID4: 
      case SPYRAMID5: 
      case SPYRAMID6: 
      case SPYRAMID7: 
        pst = pyPST;
        piece_value = pyVal;
        pieceScore += pyMob * adjacentEmptySquares(file, rank);
        if(isEdge(file,rank)) pieceScore += pyEdge;
        break;
      }
      silverScore += pieceScore + piece_value + (center_value * pst); 
    } else // red
    {
      switch(getType(kp)) {
      case RPHAROH : 
        pst = phPST;
        rPharaohRank = rank;
        rPharaohFile = file;
        break;
      case RSPHINX : 
        break;
      case RANUBIS1: 
      case RANUBIS2: 
        pst = anPST;
        piece_value = anVal;
        break;
      case RSCARAB1: 
      case RSCARAB2: 
        pst = scPST;
        if(isEdge(file,rank)) pieceScore += scEdge;
        break;
      case RPYRAMID1: 
      case RPYRAMID2: 
      case RPYRAMID3: 
      case RPYRAMID4: 
      case RPYRAMID5: 
      case RPYRAMID6: 
      case RPYRAMID7: 
        pst = pyPST;
        piece_value = pyVal;
        pieceScore += pyMob * adjacentEmptySquares(file, rank);
        if(isEdge(file,rank)) pieceScore += pyEdge;
        break;
      }
      redScore += pieceScore + piece_value + (center_value * pst); 
    }
  }
  int rSphRank = 7;
  int rSphFile = 0;
  int sSphRank = 0;
  int sSphFile = 9;
  LaserHitInfo lInfo;
  
  //exposure checks 
  for(int rot = RIGHT; rot != (DOWN + 1); ++rot) {
    lInfo = KhetState::fireLaser(rPharaohFile, rPharaohRank,
      (Rotation)rot, 0, 0);
    if(lInfo.bounced) {
      redScore += exposed;
      break;
    }
  }
  for(int rot = RIGHT; rot !=( DOWN + 1); ++rot) {
    lInfo = KhetState::fireLaser(sPharaohFile, sPharaohRank,
      (Rotation)rot, 0, 0);
    if(lInfo.bounced)  {
      silverScore += exposed;
      break;
    }
  }
//measure closest distance of laser to pharaohs
  Rotation sSphRot = getRot(b[SPHAROH]);

  lInfo = KhetState::fireLaser(sSphFile, sSphRank,
      sSphRot, sPharaohFile, sPharaohRank);

  silverScore += friendlyLaserDistance * lInfo.closest;
 
  lInfo = KhetState::fireLaser(sSphFile, sSphRank,
      sSphRot, rPharaohFile, rPharaohRank);

  silverScore += enemyLaserDistance * lInfo.closest;
//closest pharaoh for red
  Rotation rSphRot = getRot(b[RPHAROH]);

  lInfo = KhetState::fireLaser(rSphFile, rSphRank,
      rSphRot, rPharaohFile, rPharaohRank);

  redScore += friendlyLaserDistance * lInfo.closest;
  
  lInfo = KhetState::fireLaser(rSphFile, rSphRank,
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

int adjacentEmptySquares(int file, int rank) {
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
      if(KhetState::evalboard[toFile][toRank]==0) count++;
    }
  }
  return count;
}
