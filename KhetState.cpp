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
    return newstate;
  //   uint64_t key = newstate->key;
  // if (KhetState::khet_cache.count(key))
  // {
  //   assert(newstate->getBoardStr().compare(KhetState::khet_cache[key]->getBoardStr())==0);
  //   delete newstate;
  //   return KhetState::khet_cache[key];
  // } else
  // {
  //   KhetState::khet_cache[key] = newstate;
  //   return newstate;
  // }
}

KhetState* KhetState::getKhetState(KhetState* s, int mvi)
{
    KhetState* newstate = new KhetState(s,mvi);
    return newstate;
  //   uint64_t key = newstate->key;
  // if (KhetState::khet_cache.count(key))
  // {
  //   assert(newstate->getBoardStr().compare(KhetState::khet_cache[key]->getBoardStr())==0);
  //   delete newstate;
  //   return KhetState::khet_cache[key];
  // } else
  // {
  //   KhetState::khet_cache[key] = newstate;
  //   return newstate;
  // }
}

KhetState* KhetState::getKhetState(string b)
{
    KhetState* newstate = new KhetState(b);
    return newstate;
  //   uint64_t key = newstate->key;

  // if (KhetState::khet_cache.count(key))
  // {
  //   assert(newstate->getBoardStr().compare(KhetState::khet_cache[key]->getBoardStr())==0);
  //   delete newstate;
  //   return KhetState::khet_cache[key];
  // } else
  // {
  //   KhetState::khet_cache[key] = newstate;
  //   return newstate;
  // }
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
  // gen();
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
  // gen();
  // key = hashBoard();
}

KhetState::KhetState(string strBoard) : moves_init(false) {
  initBoard(strBoard);
  //his = NULL;
  //first move
  // gen();
  key = hashBoard();
}

string KhetState::getMoveStr(int i) {
  return alg(moves[i]);
}

KhetMove KhetState::getMove(int i) {
  return moves[i];
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
  return eval(ctm);
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

bool KhetState::isValidMove(KhetMove mv) {
  const unsigned int fromFile = getFromFile(mv);
  const unsigned int fromRank = getFromRank(mv); 
  const unsigned int fromRot = getFromRot(mv); 
  const unsigned int toFile = getToFile(mv); 
  const unsigned int toRank = getToRank(mv); 
  const unsigned int toRot = getToRot(mv); 
  KhetPiece origPiece = board[fromFile][fromRank];
  KhetPiece targetPiece = board[toFile][toRank];
  if (origPiece.type == EMPTY) return false;
  if (targetPiece.type != EMPTY)
  {
    if (origPiece.type!=SCARAB) return false;
    if (targetPiece.type == PYRAMID || targetPiece.type == ANUBIS) return true;
    return false;
  } else
  {
    if (fromRot==toRot)
    {
      if (fromFile==toFile && fromRank==toRank) return true;
      return false;
    } else
    {
      if (fromFile==toFile && fromRank==toRank) return false;
      return true;
    }
  }
}

uint64_t KhetState::hashBoard() {
  uint64_t hashKey = 0;
  for(int file = 0; file < FILE_COUNT; file++) {
    for (int rank = 0; rank < RANK_COUNT; rank++) {
      uint64_t zobVal = KhetState::zob[file][rank][board[file][rank].id()];
      hashKey ^= zobVal;
    }
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
    KhetPiece origPiece = board[fromFile][fromRank];
    KhetPiece targetPiece = board[toFile][toRank];
    
    if(targetPiece.type != EMPTY &&
            fromRot == toRot) {//if its rotation target wont be empty
        assert(origPiece.type == SCARAB);
        assert(targetPiece.type == ANUBIS || targetPiece.type == PYRAMID);
        //scarab swap
    key ^= KhetState::zob[fromFile][fromRank][origPiece.id()];
    key ^= KhetState::zob[toFile][toRank][targetPiece.id()];
        board[fromFile][fromRank] = targetPiece;
        board[toFile][toRank] = origPiece;
    key ^= KhetState::zob[fromFile][fromRank][board[fromFile][fromRank].id()];
    key ^= KhetState::zob[toFile][toRank][board[toFile][toRank].id()];
    }
    else {
    if (fromRot != toRot)
    {
      key ^= KhetState::zob[fromFile][fromRank][origPiece.id()];
      board[toFile][toRank].rot = (Rotation)toRot;//mv.piece is original piece
      key ^= KhetState::zob[fromFile][fromRank][board[fromFile][fromRank].id()];
    } else
    {
      key ^= KhetState::zob[fromFile][fromRank][origPiece.id()];
      key ^= KhetState::zob[toFile][toRank][targetPiece.id()];
      board[fromFile][fromRank].type = EMPTY;
      board[toFile][toRank] = origPiece;  
      board[toFile][toRank].rot = (Rotation)toRot;//mv.piece is original piece
      key ^= KhetState::zob[fromFile][fromRank][board[fromFile][fromRank].id()];
      key ^= KhetState::zob[toFile][toRank][board[toFile][toRank].id()];
    }
        
  
    }
  assert(key==hashBoard());

   
    //shoot laser
    KhetPiece sph;
    int tFile;
    int tRank;
    //set inital laser location
    if(ctm == SILVER) {
        sph = board[9][0];
        tFile = 9;
        tRank = 0;
    } else {
        sph = board[0][7];
        tFile = 0;
        tRank = 7;
    }

    assert(board[tFile][tRank].type == SPHINX && board[tFile][tRank].color == ctm);

    Rotation laserDir = sph.rot;

    LaserHitInfo laserHitInfo = fireLaser(tFile, tRank, laserDir, 0, 0);


    //piece is to be removed from board
    //handle it approrpiately
    targetPiece = laserHitInfo.hitPiece;
    tFile = laserHitInfo.hitFile;
    tRank = laserHitInfo.hitRank;

    switch(targetPiece.type) {
    case ANUBIS:
        //anubis can take hit on front
        if(!isOppositeDirections(laserHitInfo.laserDir, targetPiece.rot)) {
            key ^= KhetState::zob[tFile][tRank][board[tFile][tRank].id()];
            board[tFile][tRank].type = EMPTY;
            key ^= KhetState::zob[tFile][tRank][board[tFile][tRank].id()];
        }
        break;
    case PHAROAH:
        key ^= KhetState::zob[tFile][tRank][board[tFile][tRank].id()];
        board[tFile][tRank].type = EMPTY;
        key ^= KhetState::zob[tFile][tRank][board[tFile][tRank].id()];
        gameOver = true;
        winner = (targetPiece.color == RED) ? SILVER : RED;
        break;
    case PYRAMID:
        key ^= KhetState::zob[tFile][tRank][board[tFile][tRank].id()];
        board[tFile][tRank].type = EMPTY;
        key ^= KhetState::zob[tFile][tRank][board[tFile][tRank].id()];
        break;
    case SCARAB:
        cout << "ERROR: scarab being removed" << endl;
        break;
    case SPHINX://sphinx cant be affected
        break;
    case EMPTY:
        break;
    default:
        cout << "Unknown laser target " << targetPiece.type << endl;
    }
    //change player
    ctm = (ctm == RED) ? SILVER : RED;
	key ^= 1;
    assert(key==hashBoard());
}

LaserHitInfo KhetState::fireLaser(int tFile, int tRank, Rotation laserDir,
    int closestToFile, int closestToRank) {
  KhetPiece targetPiece;
  targetPiece.type = EMPTY;

  LaserHitInfo lInfo;
  lInfo.bounced = false;
  lInfo.hitPiece.type = EMPTY;
  lInfo.closest = 999;

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
    targetPiece = board[tFile][tRank];

    if(targetPiece.type == EMPTY) continue; //laser goes through empty sq

    //a piece was hit

    //check for reflections
    if(targetPiece.type == SCARAB) {
      switch(laserDir) {
        case UP:
          //scarab has two mirrors
          if(targetPiece.rot == UP || targetPiece.rot == DOWN){
            laserDir = RIGHT;
          }
          else {
            laserDir = LEFT;
          }
          break;
        case DOWN:  
          if(targetPiece.rot == UP || targetPiece.rot == DOWN){
            laserDir = LEFT;
          }
          else {
            laserDir = RIGHT;
          }
          break;
        case LEFT:
          if(targetPiece.rot == UP || targetPiece.rot == DOWN){
            laserDir = DOWN;
          }
          else {
            laserDir = UP;
          }
          break;
        case RIGHT:
          if(targetPiece.rot == UP || targetPiece.rot == DOWN){
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
    }
    if(targetPiece.type == PYRAMID) {
      switch(laserDir) {
        case UP:
          if(targetPiece.rot == DOWN) {
            laserDir = RIGHT;
            lInfo.bounced = true;
            continue;
          }
          else if(targetPiece.rot == LEFT) {
            laserDir = LEFT;
            lInfo.bounced = true;
            continue;
          }
          break;
        case DOWN:
          if(targetPiece.rot == UP) {
            laserDir = LEFT;
            lInfo.bounced = true;
            continue;
          }
          else if(targetPiece.rot == RIGHT) {
            laserDir = RIGHT;
            lInfo.bounced = true;
            continue;
          }
          break;
        case LEFT:
          if(targetPiece.rot == RIGHT) {
            laserDir = UP;
            lInfo.bounced = true;
            continue;
          }
          else if(targetPiece.rot == DOWN) {
            laserDir = DOWN;
            lInfo.bounced = true;
            continue;
          }
          break;
        case RIGHT:
          if(targetPiece.rot == LEFT) {
            laserDir = DOWN;
            lInfo.bounced = true;
            continue;
          }
          else if(targetPiece.rot == UP) {
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
    for(int i = 0; i < 80; i++) {
        int file = i % 10;
        int rank = 8 - (i / 10);//rank goes from 1 to 8 not 0 indexed
        rank--;
				board[file][rank].type = EMPTY; 
        string sq = "";
        sq += strBoard[i * 2];
        sq += strBoard[i * 2 + 1];
				board[file][rank] = strToPiece(sq);
    }
    if(strBoard[strBoard.length() - 1] == 'w') {
      ctm = SILVER;
    }
    else {
      ctm = RED;
    }
    // gen();
	key = hashBoard();
}

// bool KhetState::isOppositeDirections(Rotation dir1, Rotation dir2) {
//   switch (dir1) {
//     case LEFT:
//       return dir2 == RIGHT;
//       break;
//     case RIGHT:
//       return dir2 == LEFT;
//       break;
//     case UP:
//       return dir2 == DOWN;
//       break;
//     case DOWN:
//       return dir2 == UP;
//       break;
//     default:
//       cout << "Err in opp dir" << endl;
//   }
//   return true;
// }

//generates all moves and returns the number of moves 
long KhetState::gen() 
{
  if (moves_init) return moves.size();
  moves_init = true;
  PlayerColor fctm = ctm;
  moves.clear();

  int rank, file;

  for (int afile = 0; afile < 5; afile++) {
    for (int arank = 0; arank < 4; arank++) {
      for (int bbb=0; bbb<4; bbb++) {
        switch (bbb)
        {
          case 0: rank = arank; file = afile; break;
          case 1: rank = arank; file = 9-afile; break;
          case 2: rank = 7-arank; file = afile; break;
          case 3: rank = 7-arank; file = 9-afile; break;
        }


        KhetPiece piece = board[file][rank];	
        if(( piece.type == EMPTY) || (piece.color !=fctm)) continue;
        int rot1 = (piece.rot + 1) % 4;
        int rot2 = (piece.rot + 3) % 4;
        //cout << file << " " << rank << endl;
        switch (piece.type) {
          case SPHINX: 
            //rotations only
            if(fctm == SILVER) {
              if(rot1 == UP || rot1 == LEFT) {
                 moves.push_back(makeKhetMove( file, rank, piece.rot,
                                                file, rank, rot1));
              }
              if(rot2 == UP || rot2 == LEFT) {
                moves.push_back(makeKhetMove(file, rank, piece.rot,
                                                file, rank, rot2));
              }
            }
            else {
              if(rot1 == DOWN || rot1 == RIGHT) {
                moves.push_back(makeKhetMove(file, rank, piece.rot,
                                                file, rank, rot1));
              }
              if(rot2 == DOWN || rot2 == RIGHT) {
                moves.push_back(makeKhetMove(file, rank, piece.rot,
                                                file, rank, rot2));
              }
            }
            break;
          case ANUBIS: 
          case SCARAB:
          case PHAROAH: 
          case PYRAMID: 
            //moveso
            for (int toFileOffset = -1; toFileOffset < 2; toFileOffset++ ) {
              for (int toRankOffset = -1; toRankOffset < 2; toRankOffset++ ) {
                if(toRankOffset == 0 && toFileOffset == 0) continue;//must move

                int toFile = file + toFileOffset;
                int toRank = rank + toRankOffset;
                if(toFile > 9 || toFile < 0) continue;//offboard
                if(toRank > 7 || toRank < 0) continue;//offboard

                //certain squares are forbidden on board
                if(piece.color == RED) {
                  if(toFile == 9) continue;
                  if(toFile == 1 && (toRank == 0 || toRank == 7)) continue;
                }
                if(piece.color == SILVER) {
                  if(toFile == 0) continue;
                  if(toFile == 8 && (toRank == 0 || toRank == 7)) continue;
                }

                //is the target location already occuppied
                if(board[toFile][toRank].type != EMPTY) {
                  if(piece.type != SCARAB) continue;//scarabs can swap
                  KhetPiece otherPiece = board[toFile][toRank];
                    
                  //dont swap the other piece into an illegal square
                  if(otherPiece.color == RED) {
                    if(file == 9) continue;
                    if(file == 1 && (rank == 0 || rank == 7)) continue;
                  }
                  if(otherPiece.color == SILVER) {
                    if(file == 0) continue;
                    if(file == 8 && (rank == 0 || rank == 7)) continue;
                  }

                  if(otherPiece.type == PYRAMID || otherPiece.type == ANUBIS) {
                    //valid swap move
                    moves.push_back(makeKhetMove(file, rank, piece.rot, 
                                                    toFile, toRank, piece.rot));
                  }
                  else {
                    continue;
                  }
                }
                else {
                  //valid move
                  moves.push_back(makeKhetMove(file, rank, piece.rot, 
                                                  toFile, toRank, piece.rot));
                }
              }
            }
            //rotations 
            moves.push_back(makeKhetMove(file, rank, piece.rot, file, rank, rot1));
            //if(piece.type != SCARAB) {
            moves.push_back(makeKhetMove(file, rank, piece.rot, file, rank, rot2));
            //}
            break;
          default:
            cout << "unknown piece in gen: " << piece.type  << endl;
        }

  	  }
    }	
  }
  return moves.size();
}

string KhetState::getBoardStr() {
  stringstream bd;
  for(int rank = 7; rank >= 0; rank--) {
    for(int file = 0; file < 10; file++) {
      KhetPiece piece = board[file][rank];
      if(piece.type == EMPTY) {
        bd << "--";
        continue;
      }

      if(piece.color == RED) {
        bd << redLetters[piece.type];
      }
      else {
        bd << silverLetters[piece.type];
      }
      bd << rotationLetters[piece.rot]; 
    }

  }
  return bd.str();
}

string KhetState::getBoardPrettyStr() {
  stringstream bd;
  for(int rank = 7; rank >= 0; rank--) {
    for(int file = 0; file < 10; file++) {
      KhetPiece piece = board[file][rank];
      if(piece.type == EMPTY) {
        bd << "--";
        continue;
      }

      if(piece.color == RED) {
        bd << redLetters[piece.type];
      }
      else {
        bd << silverLetters[piece.type];
      }
      bd << rotationLetters[piece.rot]; 
    }
    if(rank != 0){
      bd << "\n";
    }
  }
  bd << ((ctm == SILVER) ? " w" : " b");
  bd << "\n";
  return bd.str();

}

KhetPiece KhetState::strToPiece(string sq) {
  KhetPiece pc;
  //determine type
  if(sq[0] == 'a' || sq[0] == 'A') {
    pc.type = ANUBIS;
  }
  else if (sq[0] == 'h' || sq[0] == 'H') {
    pc.type = SPHINX;
  }
  else if (sq[0] == 'p' || sq[0] == 'P') {
    pc.type = PHAROAH;
  }
  else if (sq[0] == 'y' || sq[0] == 'Y') {
    pc.type = PYRAMID;
  }
  else if (sq[0] == 's' || sq[0] == 'S') {
    pc.type = SCARAB;
  }
  else {
    pc.type = EMPTY;
  }
  //determine color
  if(isupper(sq[0], loc)) {
    pc.color = SILVER;
  }
  else {
    pc.color = RED;
  }
  //determine rotation
  if(sq[1] == 'u') {
    pc.rot = UP;
  }
  else if(sq[1] == 'd') {
    pc.rot = DOWN;
  }
  else if(sq[1] == 'l') {
    pc.rot = LEFT;
  }
  else if(sq[1] == 'r') {
    pc.rot = RIGHT;
  }
  else {
    pc.rot = UP;
  }
  return pc;
}

int KhetState::eval(PlayerColor ctm) {
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
      KhetPiece piece = board[file][rank]; 
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
          pieceScore += pyMob * adjacentEmptySquares(file, rank);
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
  Rotation sSphRot = board[sSphFile][sSphRank].rot;

  lInfo = KhetState::fireLaser(sSphFile, sSphRank,
      sSphRot, sPharaohFile, sPharaohRank);

  silverScore += friendlyLaserDistance * lInfo.closest;
 
  lInfo = KhetState::fireLaser(sSphFile, sSphRank,
      sSphRot, rPharaohFile, rPharaohRank);

  silverScore += enemyLaserDistance * lInfo.closest;
//closest pharaoh for red
  Rotation rSphRot = board[rSphFile][rSphRank].rot;

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

bool KhetState::isEdge(int file, int rank) {
  if (file == 9 || file == 0) return true;
  if (rank == 7 || rank == 0) return true;
  return false;
} 

int KhetState::adjacentEmptySquares(int file, int rank) {
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
      if(board[toFile][toRank].type == EMPTY) count++;
    }
  }
  return count;
}

