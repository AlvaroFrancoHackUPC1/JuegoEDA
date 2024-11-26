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
  const vector<Dir> dirs = {Up, Down, Left, Right};

  bool BFS(int &i, int &j, vector<vector<bool>> casVistas, queue<pair<int,int>> pendientes) {
    pendientes.push(pair(i,j));
    while(!pendientes.size() == 0) {
      //Futuras posibles posiciones
      pair<int,int> pos = pendientes.front();
      i = pos.first;
      j = pos.second;
      pendientes.pop();
      if (pos_ok(i+1,j) && !casVistas[i+1][j] && cell(i+1,j).type != Wall) {
        pendientes.push(pair(i+1,j));
        casVistas[i+1][j] = true;
      }
      if (pos_ok(i-1,j) && !casVistas[i-1][j] && cell(i-1,j).type != Wall) {
        pendientes.push(pair(i-1,j));
        casVistas[i-1][j] = true;
      }
      if (pos_ok(i,j+1) && !casVistas[i][j+1] && cell(i,j+1).type != Wall) {
        pendientes.push(pair(i,j+1));
        casVistas[i][j+1] = true;
      }
      if (pos_ok(i,j-1) && !casVistas[i][j-1] && cell(i,j-1).type != Wall) {
        pendientes.push(pair(i,j-1));
        casVistas[i][j-1] = true;
      }
      Cell act = cell(i,j);
      if (act.book) return true;
      casVistas[i][j] = true;
    }
    return false;
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
          if (posVi > wiz.pos.i && pos_ok(wiz.pos.i - 1, wiz.pos.j) && cell(wiz.pos.i - 1, wiz.pos.j).type != Wall) move(wids[i], Up);
          else if (posVi < wiz.pos.i && pos_ok(wiz.pos.i + 1, wiz.pos.j) && cell(wiz.pos.i + 1, wiz.pos.j).type != Wall) move(wids[i], Down);
        } else {
          if (posVj > wiz.pos.j && pos_ok(wiz.pos.i, wiz.pos.j - 1) && cell(wiz.pos.i, wiz.pos.j - 1).type != Wall) move(wids[i], Left);
          else if (posVj < wiz.pos.j && pos_ok(wiz.pos.i, wiz.pos.j + 1) && cell(wiz.pos.i, wiz.pos.j + 1).type != Wall) move(wids[i], Right);
        }
      }

      //BFS
      int lPosi = wiz.pos.i, lPosj = wiz.pos.j;
      vector<vector<bool>> casVistas(board_rows(), vector<bool>(board_cols(), false));
      bool camino = BFS(lPosi, lPosj, casVistas, queue<pair<int,int>>());

      if (camino) {
        if (abs(lPosi - wiz.pos.i) > abs(lPosj - wiz.pos.j)) {
          if (lPosi > wiz.pos.i && cell(wiz.pos.i+1, wiz.pos.j).type != Wall) move(wids[i], Down);
          else if (lPosi < wiz.pos.i && cell(wiz.pos.i-1, wiz.pos.j).type != Wall) move(wids[i], Up);
          {
            if (random(0,1) == 0) move(wids[i], Right);
            else move(wids[i], Left);
          }
        } 
        else {
          if (lPosj > wiz.pos.j && cell(wiz.pos.i, wiz.pos.j+1).type != Wall) move(wids[i], Right);
          else if (lPosj < wiz.pos.j && cell(wiz.pos.i, wiz.pos.j-1).type != Wall) move(wids[i], Left);
          else {
            if (random(0,1) == 0) move(wids[i], Up);
            else move(wids[i], Left);
          }
        }
      }
    }
  }
};





//!Sección de codigo relegado, pero con posibilidad de ser util
      /*
      for (int j = 0; j < 2 and !libro; ++j) {
        for (int k = 0; k < 2 and !libro; ++k) {
          if (pos_ok(wiz.pos.i + k - 1, wiz.pos.j + j - 1)) {
            Cell posible = cell(wiz.pos.i + k - 1, wiz.pos.j + j - 1);
            if (posible.book) {
              libro = true;
              posrel.i = k - 1;
              posrel.j = j - 1;
            }
          }
        }
      }
      */
  /*
  // Helper function to recursively find valid groups
  bool find_groups(vector<int>& ingredients, vector<vector<int>>& res, vector<int>& current, int idx, int target_sum, vector<bool>& used) {
    if (current.size() == 3) {
      int sum = 0;
      for (int val : current) sum += val;
      if (sum == target_sum) {
        res.push_back(current);
        return true;
      }
      return false;
    }
    for (int i = idx; i < ingredients.size(); ++i) {
      if (!used[i]) {
        used[i] = true;
        current.push_back(ingredients[i]);
        if (find_groups(ingredients, res, current, i + 1, target_sum, used)) return true;
        current.pop_back();
        used[i] = false;
      }
    }
    return false;
  }

  vector<int> agrupacion(vector<int>& ingredients) {
    vector<int> res(15, -1);
    int total_sum = 0;
    for (int i = 0; i < int(ingredients.size()); ++i) total_sum += ingredients[i];
    int target_sum = total_sum / 5;

    vector<vector<int>> grupos;
    vector<bool> used(ingredients.size(), false);
    for (int i = 0; i < 5; ++i) {
      vector<int> current;
      if (!find_groups(ingredients, grupos, current, 0, target_sum, used)) {
        return res; // Return empty result if no valid group found
      }
    }

    // Assign group indices to result vector
    for (int i = 0; i < grupos.size(); ++i) {
      for (int j = 0; j < grupos[i].size(); ++j) {
        int val = grupos[i][j];
        for (int k = 0; k < ingredients.size(); ++k) {
          if (ingredients[k] == val) {
            res[k] = i;
            ingredients[k] = -1; // Mark as used
            break;
          }
        }
      }
    }
    return res;
  }

  vector<int> encantamiento(vector<int> ingredients) {
    return agrupacion(ingredients);
  }
  */
    /*
    if (round() % 2 == 0 and round() > 50) {
      int ghostid = ghost(me());
      Unit ghost = unit(ghostid); 
      if (ghost.rounds_pending == 0) {
        vector<int> res = encantamiento(spell_ingredients());
        for(int i = 0; i < res.size(); ++i) cerr << spell_ingredients()[i];
        cerr << endl;
        for(int i = 0; i < res.size(); ++i) cerr << res[i];
        cerr << endl;
        exit(0);
        spell(ghostid, res);
      }
    }
    */



/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
