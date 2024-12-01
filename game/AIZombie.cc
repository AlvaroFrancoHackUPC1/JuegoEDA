#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Zombie_v2

struct PLAYER_NAME : public Player {
  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory() { return new PLAYER_NAME; }

  /**
   * Types and attributes for your player can be defined here.
   */
  //Cosas auxiliares
  const vector<Dir> wdirs = {Up, Down, Left, Right};
  set<int> setWiz;

  bool celdaValida(int i, int j) {
    if (pos_ok(i,j) && cell(i,j).type != Wall) return true;
    return false;
  }
  bool celdaValida(Pos p) {
    if (pos_ok(p) && cell(p).type != Wall) return true;
    return false;
  }


  //
  struct LibWiz {
    int dist; //Distancia en recorrido entre wizard y libro
    Pos p; //Posicion del libro
    Dir mov; //Proximo movimiento del wizard hasta el libro
    int id; //id Wizard
  };

  map<Pos, LibWiz> LPosD; 

  void BFS(int wiz_id, queue<int> &bfsqueue) {
    vector<vector<bool>> casVistas(board_rows(), vector<bool>(board_cols(), false));
    queue<LibWiz> pendientes;
    Pos p = unit(wiz_id).pos;
    casVistas[p.i][p.j] = true;
    for(int i = 0; i < int(wdirs.size()); ++i) {
      Pos pm = p+wdirs[i];
      if (celdaValida(pm) && !casVistas[pm.i][pm.j]) {
        pendientes.push(LibWiz{1, pm, wdirs[i], wiz_id});
        casVistas[pm.i][pm.j] = true;
      }
    }
    LibWiz front = {0, p, Up, wiz_id};
    while(!pendientes.empty()) {
      //Futuras posibles posiciones
      front = pendientes.front();
      p = front.p;

      for(int i = 0; i < int(wdirs.size()); ++i) {
        Pos pm = p+wdirs[i];
        if (celdaValida(pm) && !casVistas[pm.i][pm.j]) {
          pendientes.push(LibWiz{front.dist + 1, pm, front.mov, wiz_id});
          casVistas[pm.i][pm.j] = true;
        }
      }
      if (cell(p).book) {
        if (LPosD.find(p) == LPosD.end() || LPosD[p].dist > front.dist) {
          if (LPosD.find(p) != LPosD.end()) bfsqueue.push(LPosD[p].id);
          LPosD[p] = front;
          return;
        }
      }
      pendientes.pop();
    }
  }

  void atacarcerca(Unit wiz) {
    Pos p = wiz.pos; 
    vector<int> idWizs = wizards(me());
    
    for(int i = 0; i < int(wdirs.size()); ++i) {
      if (celdaValida(p+wdirs[i]) && cell(p+wdirs[i]).id != -1) {
        Unit pwiz = unit(cell(p+wdirs[i]).id);
        if (setWiz.find(pwiz.id) == setWiz.end() || pwiz.is_in_conversion_process()) {
          move(wiz.id, wdirs[i]);
          //cerr << "soy: " << wiz.id  << " estoy: " << p << " ataco: " << p+wdirs[i] << endl;
          return;
        }
      }
    }
  }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play() {
    vector<int> wids = wizards(me());
    Pos posV = pos_voldemort();
    int posVi = posV.i;
    int posVj = posV.j;
    LPosD.clear();
    queue<int> bfsqueue;

    //Inicializa un set con todos
    for(int k = 0; k < int(wids.size()); ++k) setWiz.insert(wids[k]);

    for (int i = 0; i < int(wids.size()); ++i) {
      Unit wiz = unit(wids[i]);
      //Huir
      if (abs(posVi - wiz.pos.i) <= 5 && abs(posVj - wiz.pos.j) <= 5) {
        if (abs(posVi - wiz.pos.i) > abs(posVj - wiz.pos.j)) {
          if (posVi > wiz.pos.i && celdaValida(wiz.pos.i - 1, wiz.pos.j)) move(wids[i], Up);
          else if (posVi < wiz.pos.i && celdaValida(wiz.pos.i + 1, wiz.pos.j)) move(wids[i], Down);
        } else {
          if (posVj > wiz.pos.j && celdaValida(wiz.pos.i, wiz.pos.j - 1)) move(wids[i], Left);
          else if (posVj < wiz.pos.j && celdaValida(wiz.pos.i, wiz.pos.j + 1)) move(wids[i], Right);
        }
      }

      //Atacar si esta cerca
      atacarcerca(wiz);
      

      //BFS
      bfsqueue.push(wiz.id);
      while(!bfsqueue.empty()) {
        BFS(bfsqueue.front(), bfsqueue);
        int id = bfsqueue.front();
        bfsqueue.pop();
      } 
    }
    map<Pos,LibWiz>::iterator it = LPosD.begin();
    while(it != LPosD.end()) {
      cerr << it->second.id << ' ' <<  it->second.dist << endl;
      move(it->second.id, it->second.mov);
      ++it;
    }
  }
};
/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);