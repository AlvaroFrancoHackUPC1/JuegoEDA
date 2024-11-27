#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Zombie_v1

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
  Dir BFS(Pos &p, bool Vol) {
    vector<vector<bool>> casVistas(board_rows(), vector<bool>(board_cols(), false));
    queue<pair<Pos,Dir>> pendientes;
    casVistas[p.i][p.j] = true;

    for(int i = 0; i < int(wdirs.size()); ++i) {
      Pos pm = p+wdirs[i];
      if (celdaValida(pm) && !casVistas[pm.i][pm.j]) {
        pendientes.push(pair(pm, wdirs[i]));
        casVistas[pm.i][pm.j] = true;
      }
    }

    while(!pendientes.empty()) {
      //Futuras posibles posiciones
      pair<Pos,Dir> front = pendientes.front();
      p = front.first;
      for(int i = 0; i < int(wdirs.size()); ++i) {
        Pos pm = p+wdirs[i];
        if (celdaValida(pm) && !casVistas[pm.i][pm.j]) {
          pendientes.push(pair(pm, front.second));
          casVistas[pm.i][pm.j] = true;
        }
      }
      if (!Vol && cell(p).book) return front.second;
      else if (Vol && cell(p).id != -1 && setWiz.find(cell(p).id) == setWiz.end()) return front.second;
      pendientes.pop();
    }
    
    return Up; // Placeholder return value
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
      for(int k = 0; k < int(wids.size()); ++k) setWiz.insert(wids[i]);
      atacarcerca(wiz);

      //BFS
      Pos lPos = wiz.pos;
      Dir mov = Up;
      move(wids[i], BFS(lPos, false));
      
    }
  }
};

/*
  bool BFS(Pos &p, Dir mov) {
    vector<vector<bool>> casVistas(board_rows(), vector<bool>(board_cols(), false));
    queue<pair<Pos,Dir>> pendientes;
    if (celdaValida(p.i+1, p.j) && !casVistas[p.i+1][p.j]) {
      pendientes.push(pair(p+Down, Down));
      casVistas[p.i+1][p.j] = true;
    }
    if (celdaValida(p.i-1, p.j) && !casVistas[p.i-1][p.j]) {
      pendientes.push(pair(p+Up, Up));
      casVistas[p.i-1][p.j] = true;
    }
    if (celdaValida(p.i, p.j+1) && !casVistas[p.i][p.j+1]) {
      pendientes.push(pair(p+Right, Right));
      casVistas[p.i][p.j+1] = true;
    }
    if (celdaValida(p.i, p.j-1) && !casVistas[p.i][p.j-1]) {
      pendientes.push(pair(p+Left, Left));
      casVistas[p.i][p.j-1] = true;
    }
    while(!pendientes.size() == 0) {
      //Futuras posibles posiciones
      pair<Pos,Dir> front = pendientes.front();
      p = front.first;
      mov = front.second;

      if (celdaValida(p.i+1, p.j) && !casVistas[p.i+1][p.j]) {
        pendientes.push(pair(p+Down, mov));
        casVistas[p.i+1][p.j] = true;
      }
      if (celdaValida(p.i-1, p.j) && !casVistas[p.i-1][p.j]) {
        pendientes.push(pair(p+Up, mov));
        casVistas[p.i-1][p.j] = true;
      }
      if (celdaValida(p.i, p.j+1) && !casVistas[p.i][p.j+1]) {
        pendientes.push(pair(p+Right, mov));
        casVistas[p.i][p.j+1] = true;
      }
      if (celdaValida(p.i, p.j-1) && !casVistas[p.i][p.j-1]) {
        pendientes.push(pair(p+Left, mov));
        casVistas[p.i][p.j-1] = true;
      }
      //cerr << p.i << ' ' << p.j << endl;
      //cerr << "mov: " << mov << endl;
      Cell act = cell(p.i, p.j);
      if (act.book) return true;
      pendientes.pop();
    }
    return false;
  }
*/
/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
