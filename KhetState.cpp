#include "KhetState.h"

// Iterates through all keys of cache and verifies that the hashed board
// is hashed to the correct key
int KhetState::checkKhetCache() {
    map<uint64_t,KhetState*>::iterator it;

    int errors = 0;
    int total = 0;

    for (it=khet_cache.begin(); it!=khet_cache.end(); it++) {
        total++;
        if ((*it).second->hashBoard() != (*it).first)
        {
          printf("key: %lu expected %lu\n",(*it).second->hashBoard(),(*it).first);
                errors++;
        }
    }

    assert(errors==0);
    printf("%u errors in cache out of %u total\n",errors,total);
  return errors;
}

map<uint64_t,KhetState*> KhetState::khet_cache;

KhetState* KhetState::getKhetState(uint64_t key)
{
  if (KhetState::khet_cache.count(key))
  {
    return KhetState::khet_cache[key];
  } else
  {
    return (KhetState*)0;
  }
}

KhetState* KhetState::getKhetState(KhetState* s, KhetMove mv)
{
    KhetState* newstate = new KhetState(s,mv);
    uint64_t key = newstate->key;
  if (KhetState::khet_cache.count(key))
  {
    assert(newstate->getBoardStr().compare(KhetState::khet_cache[key]->getBoardStr())==0);
    delete newstate;
    return KhetState::khet_cache[key];
  } else
  {
    KhetState::khet_cache[key] = newstate;
    return newstate;
  }
}

KhetState* KhetState::getKhetState(KhetState* s, int mvi)
{
    KhetState* newstate = new KhetState(s,mvi);
    uint64_t key = newstate->key;
  if (KhetState::khet_cache.count(key))
  {
    assert(newstate->getBoardStr().compare(KhetState::khet_cache[key]->getBoardStr())==0);
    delete newstate;
    return KhetState::khet_cache[key];
  } else
  {
    KhetState::khet_cache[key] = newstate;
    return newstate;
  }
}

KhetState* KhetState::getKhetState(string b)
{
    KhetState* newstate = new KhetState(b);
    uint64_t key = newstate->key;

  if (KhetState::khet_cache.count(key))
  {
    assert(newstate->getBoardStr().compare(KhetState::khet_cache[key]->getBoardStr())==0);
    delete newstate;
    return KhetState::khet_cache[key];
  } else
  {
    KhetState::khet_cache[key] = newstate;
    return newstate;
  }
}


KhetState::KhetState() : moves_init(false){
}

KhetState::KhetState(KhetState* s, KhetMove mv) : moves_init(false) {
  ctm = s->ctm;
  memcpy(board, s->board, sizeof(board));
  gameOver = s->gameOver;

  //maintain history pointer for repeition checking
  //his = s;
  //move should be valid
  //string result*/
  key = s->key;
  imake(mv);
  //assert(result.compare("") != 0);
  gen();
  // key = hashBoard();
}

KhetState::KhetState(KhetState* s, int mvi) : moves_init(false) {
  ctm = s->ctm;
  memcpy(board, s->board, sizeof(board));
  gameOver = s->gameOver;

  //maintain history pointer for repeition checking
  //his = s;
  //move should be valid
  //string result*/
  key = s->key;
  imake(s->moves[mvi]);
  //assert(result.compare("") != 0);
  gen();
  // key = hashBoard();
}

KhetState::KhetState(string strBoard) : moves_init(false) {
  initBoard(strBoard);
  //his = NULL;
  //first move
  gen();
  key = hashBoard();
}
string KhetState::getMove(int i) {
  return alg(moves[i]);
}

string KhetState::getCtmStr() {
  return (ctm == SILVER) ? "Silver" : "Crimson";
}

bool KhetState::isWon() {
  return gameOver;
}
    
void KhetState::getPossibleStates(std::vector<KhetState*> &v) {
  if(gameOver) return;
  gen();
  for(int i = 0; i < moves.size(); i++) {
    v.push_back(getKhetState(this, moves[i]));
  }
  return;
}

void KhetState::getPossibleMoves(std::vector<KhetMove> &v) {
  if(gameOver) return;
  gen();
  for(int i = 0; i < moves.size(); i++) {
    v.push_back(moves[i]);
  }
  return;
}

unsigned int KhetState::getNumPossibleMoves() {
  if(gameOver) return 0;
  gen();
  return moves.size();
}

uint64_t KhetState::perft(int depth) {
  uint64_t nodec = 0;
  if (depth == 0) return 1;

  gen();
  KhetState localState = *this;
  if(depth == 1) return moves.size();
  for(int i = 0; i < moves.size(); i++) {
    KhetState next = localState;

    next.imake(moves[i]);
    if(next.isWon()){
      nodec += 1;
    }
    else {
      long num = next.perft(depth - 1);
      nodec += num;
    }
  }
  return nodec;
}

void KhetState::debugMoves() {
  gen();
  for(int i = 0; i < moves.size(); i++) {
    cout << alg(moves[i]) << "\t";
	
  }
  cout << endl;
}
int KhetState::init(string strBoard) {
  if(strBoard.compare("classic") == 0){
    initBoard(classicOpen);
    return 0;
  }
  else if(strBoard.compare("dynasty") == 0){
    initBoard(dynastyOpen);
    return 0;
  }
  else if(strBoard.compare("imhotep") == 0){
    initBoard(imhotepOpen);
    return 0;
  }
  else {
    return 1;
  }
}
int KhetState::evaluate() {
  if(gameOver) {
    //ABSearch regards 32000 as mate value
    return (ctm == winner) ? 32000 : -32000;
  }
  return eval(board, ctm);
}

//converts move to string notation
string KhetState::alg(KhetMove mv) {
  string s = "";
  //used for integer to str conversion
  stringstream mvstream;

  mvstream << fileLetters[getFromFile(mv)];
  mvstream << getFromRank(mv) + 1;//0 indexing vs 1 indexing in rank
  if(getToRot(mv) != getFromRot(mv)){
    if(((getFromRot(mv) + 1) & 3) == (getToRot(mv) & 3) ) {
      mvstream << "L";
    }
    else {
      mvstream << "R";

    }
  }
  else {
    mvstream << fileLetters[getToFile(mv)];
    mvstream << getToRank(mv) + 1;
  }
  return mvstream.str();
}

//attenots to make move in the from of "a8R" or a8a7 in algstr
//returns empty str if move is invalid, returns algrstr otherwise
int KhetState::makeMove(string algstr) {
  //moves_init = false;
  gen();
  for(int i = 0; i < moves.size(); i++) {
    if(alg(moves[i]).compare(algstr) == 0) {
      //move is in list, should be valid
      //imake(moves[i]);
      //moves_init = false;
      return i;
    }
  }
  return -1;
}

KhetState* KhetState::makeMove(KhetMove mv) {
  return getKhetState(this,mv);
}

KhetState* KhetState::makeMove(int index) {
  return getKhetState(this,moves[index]);
}

uint64_t KhetState::hashBoard() {
  uint64_t hashKey = 0;
  KhetPiece kp;
  for (int x=0; x<26; x++)
  {
    kp = board[x];
    hashKey ^= KhetState::zob[getFile(kp)][getRank(kp)][getID(kp)];
  }
  return hashKey^=(int)ctm;
}

//performs move on this state, assumes move is valid
void KhetState::imake(KhetMove mv) {
    //move piece
    assert(key==hashBoard());
    const unsigned int fromFile = getFromFile(mv);
    const unsigned int fromRank = getFromRank(mv); 
    const unsigned int fromRot = getFromRot(mv); 
    const unsigned int toFile = getToFile(mv); 
    const unsigned int toRank = getToRank(mv); 
    const unsigned int toRot = getToRot(mv); 
    PieceType origPiece = (PieceType)getFromPiece(mv);
    PieceType targetPiece = (PieceType)getToPiece(mv);
    
    if(targetPiece != EMPTY && fromRot == toRot) {//if its rotation target wont be empty
      assert(((origPiece)%13)/2 == 1);
      assert(((targetPiece)%13)>3);
      //scarab swap
      key ^= KhetState::zob[fromFile][fromRank][getID(board[origPiece])];
      key ^= KhetState::zob[toFile][toRank][getID(board[targetPiece])];

      board[origPiece] = moveKhetPiece(board[origPiece],toFile,toRank);
      board[targetPiece] = moveKhetPiece(board[targetPiece],fromFile,fromRank);

      key ^= KhetState::zob[fromFile][fromRank][getID(board[targetPiece])];
      key ^= KhetState::zob[toFile][toRank][getID(board[origPiece])];
    }
    else {
      if (fromRot != toRot)
      {
        key ^= KhetState::zob[fromFile][fromRank][getID(board[origPiece])];
        board[origPiece] = rotateKhetPiece(board[origPiece],toRot);
        key ^= KhetState::zob[fromFile][fromRank][getID(board[origPiece])];
      } else
      {
        key ^= KhetState::zob[fromFile][fromRank][getID(board[origPiece])];
        // key ^= KhetState::zob[toFile][toRank][104];
        board[origPiece] = moveKhetPiece(board[origPiece],toFile,toRank);
        // key ^= KhetState::zob[fromFile][fromRank][104];
        key ^= KhetState::zob[toFile][toRank][getID(board[origPiece])];
      }
    }
    assert(key==hashBoard());

   
    //shoot laser
    KhetPiece sph;
    int tFile;
    int tRank;
    //set inital laser location
    if(ctm == SILVER) {
        sph = board[SSPHINX];
        tFile = 9;
        tRank = 0;
    } else {
        sph = board[RSPHINX];
        tFile = 0;
        tRank = 7;
    }

    // assert(board[tFile][tRank].type == SPHINX && board[tFile][tRank].color == ctm);

    Rotation laserDir = getRot(sph);

    memset(KhetState::evalboard,sizeof(KhetPiece)*80,0);

    for (int x=0; x<26; x++)
    {
      KhetPiece kp = board[x];
      if (kp==0) continue;
      unsigned int file = getFile(kp);
      unsigned int rank = getRank(kp);
      evalboard[file][rank] = kp;
    }

    LaserHitInfo laserHitInfo = fireLaser(tFile, tRank, laserDir, 0, 0);


    //piece is to be removed from board
    //handle it approrpiately
    KhetPiece tPiece = laserHitInfo.hitPiece;
    tFile = laserHitInfo.hitFile;
    tRank = laserHitInfo.hitRank;
    if (tFile>=0)
    {
      unsigned int hittype = getType(tPiece);

      switch(hittype) {
      case SANUBIS1: case SANUBIS2: case RANUBIS1: case RANUBIS2:
          //anubis can take hit on front
          Rotation rot = getRot(tPiece);
          if(!isOppositeDirections(laserHitInfo.laserDir, rot)) {
              key ^= KhetState::zob[tFile][tRank][getID(tPiece)];
              board[hittype] = 0;
              // key ^= KhetState::zob[tFile][tRank][104];
          }
          break;
      case SPHAROH: case RPHAROH:
          key ^= KhetState::zob[tFile][tRank][getID(tPiece)];
          board[hittype] = 0;
          // key ^= KhetState::zob[tFile][tRank][104];
          gameOver = true;
          winner = (hittype/13) ? SILVER : RED;
          break;
      case SPYRAMID1: case SPYRAMID4: case SPYRAMID7: case RPYRAMID3: case RPYRAMID6:
      case SPYRAMID2: case SPYRAMID5: case RPYRAMID1: case RPYRAMID4: case RPYRAMID7:
      case SPYRAMID3: case SPYRAMID6: case RPYRAMID2: case RPYRAMID5:
          key ^= KhetState::zob[tFile][tRank][getID(tPiece)];
          board[hittype] = 0;
          // key ^= KhetState::zob[tFile][tRank][104];
          break;
      case SSCARAB1: case SSCARAB2: case RSCARAB1: case RSCARAB2: 
          cout << "ERROR: scarab being removed" << endl;
          break;
      case SSPHINX: case RSPHINX://sphinx cant be affected
          break;
      case EMPTY:
          break;
      default:
          cout << "Unknown laser target " << (tPiece>>7) << endl;
      }
    }
    
    //change player
    ctm = (PlayerColor)(((int)ctm)^1);
	  key ^= 1;
    assert(key==hashBoard());
}

LaserHitInfo KhetState::fireLaser(int tFile, int tRank, Rotation laserDir,
                                  int closestToFile, int closestToRank) {
  KhetPiece targetPiece;
  LaserHitInfo lInfo;
  lInfo.bounced = false;
  lInfo.hitPiece = 0;
  lInfo.closest = 999;
  lInfo.hitPiece = 0;
  lInfo.hitFile = -1;
  lInfo.hitRank = -1;
  lInfo.laserDir = 0;

  unsigned int type;
  Rotation rot;

  while(true) {
    switch(laserDir) {
      case LEFT:
        tFile--;
        break;
      case RIGHT:
        tFile++;
        break;
      case UP:
        tRank++;
        break;
      case DOWN:
        tRank--;
        break;
      default:
        cout << "Unkown laserDirection " << laserDir << endl;
    }
    int distanceFromLaser = abs(tFile - closestToFile) + abs(tRank - closestToRank); 
    if(distanceFromLaser < lInfo.closest) {
      lInfo.closest = distanceFromLaser;
    }
    if(tFile > 9 || tFile < 0) break; //off board, clean miss
    if(tRank > 7 || tRank < 0) break;

    //what piece is hit by laser?
    targetPiece = evalboard[tFile][tRank];

    if(targetPiece==0) continue; //laser goes through empty sq

    type = getType(targetPiece);
    rot = getRot(targetPiece);

    //a piece was hit

    //check for reflections

    switch (type)
    {
      case SSCARAB1: case SSCARAB2: case RSCARAB1: case RSCARAB2: 
        switch(laserDir) {
        case UP:
          //scarab has two mirrors
          if(rot == UP || rot == DOWN){
            laserDir = RIGHT;
          }
          else {
            laserDir = LEFT;
          }
          break;
        case DOWN:  
          if(rot == UP || rot == DOWN){
            laserDir = LEFT;
          }
          else {
            laserDir = RIGHT;
          }
          break;
        case LEFT:
          if(rot == UP || rot == DOWN){
            laserDir = DOWN;
          }
          else {
            laserDir = UP;
          }
          break;
        case RIGHT:
          if(rot == UP || rot == DOWN){
            laserDir = UP;
          }
          else {
            laserDir = DOWN;
          }
          break;
        default:
          cout << "bad bounce" << endl;
      }
      lInfo.bounced = true;
      continue;//scarabs always bounec laser, cant be removed
    case SPYRAMID1: case SPYRAMID2: case SPYRAMID3: case SPYRAMID4: 
    case SPYRAMID5: case SPYRAMID6: case SPYRAMID7: case RPYRAMID1: 
    case RPYRAMID2: case RPYRAMID3: case RPYRAMID4: case RPYRAMID5: 
    case RPYRAMID6: case RPYRAMID7:
      switch(laserDir) {
        case UP:
          if(rot == DOWN) {
            laserDir = RIGHT;
            lInfo.bounced = true;
            continue;
          }
          else if(rot == LEFT) {
            laserDir = LEFT;
            lInfo.bounced = true;
            continue;
          }
          break;
        case DOWN:
          if(rot == UP) {
            laserDir = LEFT;
            lInfo.bounced = true;
            continue;
          }
          else if(rot == RIGHT) {
            laserDir = RIGHT;
            lInfo.bounced = true;
            continue;
          }
          break;
        case LEFT:
          if(rot == RIGHT) {
            laserDir = UP;
            lInfo.bounced = true;
            continue;
          }
          else if(rot == DOWN) {
            laserDir = DOWN;
            lInfo.bounced = true;
            continue;
          }
          break;
        case RIGHT:
          if(rot == LEFT) {
            laserDir = DOWN;
            lInfo.bounced = true;
            continue;
          }
          else if(rot == UP) {
            laserDir = UP;
            lInfo.bounced = true;
            continue;
          }
          break;
      }
    }

    lInfo.hitPiece = targetPiece;
    lInfo.hitFile = tFile;
    lInfo.hitRank = tRank;
    lInfo.laserDir = laserDir;
    break;
  }
  //piece got hit
  return lInfo;
}

void KhetState::initBoard(string strBoard) {
  gameOver = false;
  memset(board,sizeof(KhetPiece)*26,0);

  for(int i = 0; i < 80; i++) {
      int file = i % 10;
      int rank = 7 - (i / 10);//rank goes from 1 to 8 yes 0 indexed
      if (strBoard[i*2]=='-') continue;
      
			// board[file][rank].type = EMPTY;
      int type = (int)(strBoard[i*2]-'a');
      int rot = (int)(strBoard[i*2+1]-'0');
      
      board[type] = makeKhetPiece(type,rot,file,rank);
  }
  if(strBoard[strBoard.length() - 1] == 'w') {
    ctm = SILVER;
  }
  else {
    ctm = RED;
  }
  gen();
   key = hashBoard();
}

//generates all moves and returns the number of moves 
long KhetState::gen() 
{
  if (moves_init) return moves.size();
  moves_init = true;
  PlayerColor fctm = ctm;
  moves.clear();

  KhetPiece kp;
  unsigned int file;
  unsigned int rank;
  unsigned int type;
  Rotation rot;

  memset(KhetState::evalboard,sizeof(KhetPiece)*80,0);

  for (int x=0; x<26; x++)
  {
    kp = board[x];
    if (kp==0) continue;
    file = getFile(kp);
    rank = getRank(kp);
    evalboard[file][rank] = kp;
  }

  if (ctm == SILVER)
  {
    for (int x=0; x<13; x++)
    {
      kp = board[x];
      if (kp==0) continue;
      file = getFile(kp);
      rank = getRank(kp);
      rot = getRot(kp);
      type = getType(kp);

      int rot1 = (rot + 1) % 4;
      int rot2 = (rot + 3) % 4;

      switch (x)
      {
        case SSPHINX: 
          //rotations only
          if(rot1 == UP || rot1 == LEFT) {
             moves.push_back(makeKhetMove( file, rank, rot,
                                            file, rank, rot1,
                                            x, x));
          } else
          {
            moves.push_back(makeKhetMove(file, rank, rot,
                                            file, rank, rot2,
                                            x, x));
          }
          break;
        case SANUBIS1: case SANUBIS2: 
        case SSCARAB1: case SSCARAB2:
        case SPHAROH: 
        case SPYRAMID1: case SPYRAMID2: case SPYRAMID3: case SPYRAMID4: 
        case SPYRAMID5: case SPYRAMID6: case SPYRAMID7: 
          //moveso
          for (int toFileOffset = -1; toFileOffset < 2; toFileOffset++ ) {
            for (int toRankOffset = -1; toRankOffset < 2; toRankOffset++ ) {
              if(toRankOffset == 0 && toFileOffset == 0) continue;//must move

              int toFile = file + toFileOffset;
              int toRank = rank + toRankOffset;
              if(toFile > 9 || toFile < 0) continue;//offboard
              if(toRank > 7 || toRank < 0) continue;//offboard

              //certain squares are forbidden on board

              if(toFile == 0) continue;
              if(toFile == 8 && (toRank == 0 || toRank == 7)) continue;
              
              PieceType otherPiece = getType(evalboard[toFile][toRank]);
              //is the target location already occuppied
              if(evalboard[toFile][toRank] != 0) {
                if(!(type==SSCARAB1||type==SSCARAB2)) continue;//scarabs can swap
                
                //dont swap the other piece into an illegal square
                if(otherPiece/13) {
                  if(file == 9) continue;
                  if(file == 1 && (rank == 0 || rank == 7)) continue;
                } else {
                  if(file == 0) continue;
                  if(file == 8 && (rank == 0 || rank == 7)) continue;
                }


                if((otherPiece%13)>3) {
                  //valid swap move
                  moves.push_back(makeKhetMove(file, rank, rot, 
                                                  toFile, toRank, rot,
                                                  x, otherPiece));
                } else {
                  continue;
                }
              } else {
                //valid move
                moves.push_back(makeKhetMove(file, rank, rot, 
                                                toFile, toRank, rot,
                                                x, EMPTY));
              }
            }
          }
          //rotations 
          moves.push_back(makeKhetMove(file, rank, rot, file, rank, rot1, x, x));
          //if(piece.type != SCARAB) {
          moves.push_back(makeKhetMove(file, rank, rot, file, rank, rot2, x, x));
          //}
          break;
        default:
          cout << "unknown piece in gen: " << x << " " << kp  << endl;
      }
    }
  } else
  {
    for (int x=13; x<26; x++)
    {
      if (kp==0) continue;
      kp = board[x];
      file = getFile(kp);
      rank = getRank(kp);
      rot = getRot(kp);
      type = getType(kp);

      int rot1 = (rot + 1) % 4;
      int rot2 = (rot + 3) % 4;

      switch (x)
      {
        case RSPHINX: 
          //rotations only
          if(rot1 == DOWN || rot1 == RIGHT) {
             moves.push_back(makeKhetMove( file, rank, rot,
                                            file, rank, rot1,
                                            x, x));
          } else
          {
            moves.push_back(makeKhetMove(file, rank, rot,
                                            file, rank, rot2,
                                            x, x));
          }
          break;
        case RANUBIS1: case RANUBIS2: 
        case RSCARAB1: case RSCARAB2:
        case RPHAROH: 
        case RPYRAMID1: case RPYRAMID2: case RPYRAMID3: case RPYRAMID4: 
        case RPYRAMID5: case RPYRAMID6: case RPYRAMID7: 
          //moveso
          for (int toFileOffset = -1; toFileOffset < 2; toFileOffset++ ) {
            for (int toRankOffset = -1; toRankOffset < 2; toRankOffset++ ) {
              if(toRankOffset == 0 && toFileOffset == 0) continue;//must move

              int toFile = file + toFileOffset;
              int toRank = rank + toRankOffset;
              if(toFile > 9 || toFile < 0) continue;//offboard
                        if(toRank > 7 || toRank < 0) continue;//offboard

              //certain squares are forbidden on board
              if(toFile == 9) continue;
              if(toFile == 1 && (toRank == 0 || toRank == 7)) continue;
              

              PieceType otherPiece = getType(evalboard[toFile][toRank]);
              //is the target location already occuppied
              if(evalboard[toFile][toRank] != 0) {
                if(!(type==RSCARAB1||type==RSCARAB2)) continue;//scarabs can swap
                
                //dont swap the other piece into an illegal square
                if(otherPiece/13) {
                  if(file == 9) continue;
                  if(file == 1 && (rank == 0 || rank == 7)) continue;
                } else {
                  if(file == 0) continue;
                  if(file == 8 && (rank == 0 || rank == 7)) continue;
                }

                if((otherPiece%13)>3) {
                  //valid swap move
                  moves.push_back(makeKhetMove(file, rank, rot, 
                                                  toFile, toRank, rot,
                                                  x, otherPiece));
                } else {
                  continue;
                }
              }
              else {
                //valid move
                moves.push_back(makeKhetMove(file, rank, rot, 
                                                toFile, toRank, rot,
                                                  x, EMPTY));
              }
            }
          }
          //rotations 
          moves.push_back(makeKhetMove(file, rank, rot, file, rank, rot1, x, x));
          //if(piece.type != SCARAB) {
          moves.push_back(makeKhetMove(file, rank, rot, file, rank, rot2, x, x));
          //}
          break;
        default:
          cout << "unknown piece in gen: " << x << " " << kp  << endl;
      }
    }
  }
  return moves.size();
}

string KhetState::getBoardStr() {
  // return "";

  KhetPiece kp;
  unsigned int file;
  unsigned int rank;
  unsigned int type;

  memset(KhetState::evalboard,sizeof(KhetPiece)*80,0);

  for (int x=0; x<26; x++)
  {
    kp = board[x];
    if (kp==0) continue;
    file = getFile(kp);
    rank = getRank(kp);
    evalboard[file][rank] = kp;
  }

  stringstream bd;
  for(int rank = 7; rank >= 0; rank--) {
    for(int file = 0; file < 10; file++) {
      kp = evalboard[file][rank];
      if(kp == 0) {
        bd << "--";
        continue;
      }
      type = getType(kp);

      bd << pieceLetters[type];
      bd << rotationLetters[getRot(kp)];  
    }
  }
  return bd.str();
}

string KhetState::getBoardPrettyStr() {
  // return "";

  KhetPiece kp;
  unsigned int file;
  unsigned int rank;
  unsigned int type;

  memset(KhetState::evalboard,sizeof(KhetPiece)*80,0);

  for (int x=0; x<26; x++)
  {
    kp = board[x];
    if (kp==0) continue;
    file = getFile(kp);
    rank = getRank(kp);
    evalboard[file][rank] = kp;
  }

  stringstream bd;
  for(int rank = 7; rank >= 0; rank--) {
    for(int file = 0; file < 10; file++) {
      kp = evalboard[file][rank];
      if(kp == 0) {
        bd << "--";
        continue;
      }
      type = getType(kp);

      bd << pieceLetters[type];
      bd << rotationLetters[getRot(kp)]; 
    }
    if(rank != 0){
      bd << "\n";
    }
  }

  bd << ((ctm == SILVER) ? " w" : " b");
  bd << "\n";
  return bd.str();
}

// KhetPiece KhetState::strToPiece(string sq) {
//   KhetPiece pc;
//   //determine type
//   if(sq[0] == 'a' || sq[0] == 'A') {
//     pc.type = ANUBIS;
//   }
//   else if (sq[0] == 'h' || sq[0] == 'H') {
//     pc.type = SPHINX;
//   }
//   else if (sq[0] == 'p' || sq[0] == 'P') {
//     pc.type = PHAROAH;
//   }
//   else if (sq[0] == 'y' || sq[0] == 'Y') {
//     pc.type = PYRAMID;
//   }
//   else if (sq[0] == 's' || sq[0] == 'S') {
//     pc.type = SCARAB;
//   }
//   else {
//     pc.type = EMPTY;
//   }
//   //determine color
//   if(isupper(sq[0], loc)) {
//     pc.color = SILVER;
//   }
//   else {
//     pc.color = RED;
//   }
//   //determine rotation
//   if(sq[1] == 'u') {
//     pc.rot = UP;
//   }
//   else if(sq[1] == 'd') {
//     pc.rot = DOWN;
//   }
//   else if(sq[1] == 'l') {
//     pc.rot = LEFT;
//   }
//   else if(sq[1] == 'r') {
//     pc.rot = RIGHT;
//   }
//   else {
//     pc.rot = UP;
//   }
//   return pc;
// }
