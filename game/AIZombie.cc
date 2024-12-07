#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Zombie_v3

struct PLAYER_NAME : public Player {
  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory() { return new PLAYER_NAME; }

  /**
   * Types and attributes for your player can be defined here.
   */
  // Cosas auxiliares
  const vector<Dir> wdirs = {Up, Down, Left, Right};
  const vector<Dir> fdirs = {DR, RU, UL, LD, Down, Right, Up, Left};
  set<int> setWiz;

  bool celdaValida(int i, int j) {
    if (pos_ok(i, j) && cell(i, j).type != Wall) return true;
    return false;
  }
  bool celdaValida(Pos p) {
    if (pos_ok(p) && cell(p).type != Wall) return true;
    return false;
  }

  // Funciones uitiles
  vector<int> asignar_grupos() {
    vector<int> ingredientes = spell_ingredients();
    int n = ingredientes.size(); // Debería ser 15
    int num_grupos = 5;
    int tam_grupo = 3;

    vector<int> asignaciones(n, -1);
    vector<int> sumas_grupos(num_grupos, 0);
    vector<int> conteos_grupos(num_grupos, 0);

    // Calcular la suma objetivo
    int suma_total = 0;
    for (int i = 0; i < n; ++i) {
      suma_total += ingredientes[i];
    }
    int suma_objetivo = suma_total / num_grupos;

    vector<pair<int, int>> ingredientes_ordenados;
    for (int i = 0; i < n; ++i) {
      ingredientes_ordenados.push_back(make_pair(ingredientes[i], i));
    }
    sort(ingredientes_ordenados.rbegin(), ingredientes_ordenados.rend());

    for (int i = 0; i < n; ++i) {
      int valor = ingredientes_ordenados[i].first;
      int indice = ingredientes_ordenados[i].second;
      for (int g = 0; g < num_grupos; ++g) {
        if (conteos_grupos[g] < tam_grupo && sumas_grupos[g] + valor <= suma_objetivo) {
          asignaciones[indice] = g;
          sumas_grupos[g] += valor;
          conteos_grupos[g] += 1;
          break; // Pasar al siguiente ingrediente
        }
      }
    }
    return asignaciones;
  }

  struct LibWiz {
    int dist;  // Distancia en recorrido entre wizard y libro
    Pos p;     // Posicion del libro
    Dir mov;   // Proximo movimiento del wizard hasta el libro
    int id;    // id Wizard
  };

  map<Pos, LibWiz> LPosD;

  void BFS(int wiz_id, queue<int>& bfsqueue, bool fantasma) {
    vector<vector<bool>> casVistas(board_rows(), vector<bool>(board_cols(), false));
    queue<LibWiz> pendientes;
    Pos p = unit(wiz_id).pos;
    casVistas[p.i][p.j] = true;
    vector<Dir> movement = fantasma? fdirs : wdirs;

    for (int i = 0; i < int(movement.size()); ++i) {
      Pos pm = p + movement[i];
      if (celdaValida(pm) && !casVistas[pm.i][pm.j]) {
        pendientes.push(LibWiz{1, pm, movement[i], wiz_id});
        casVistas[pm.i][pm.j] = true;
      }
    }
    LibWiz front = {0, p, Up, wiz_id};
    while (!pendientes.empty()) {
      // Futuras posibles posiciones
      front = pendientes.front();
      p = front.p;

      for (int i = 0; i < int(movement.size()); ++i) {
        Pos pm = p + movement[i];
        if (celdaValida(pm) && !casVistas[pm.i][pm.j]) {
          pendientes.push(LibWiz{front.dist + 1, pm, front.mov, wiz_id});
          casVistas[pm.i][pm.j] = true;
        }
      }
      //! Revisar
      if (cell(p).book || 
      (cell(p).id != -1 && cell(p).owner == me() && unit(cell(p).id).is_in_conversion_process() && unit(cell(p).id).rounds_for_converting() > front.dist) ||
      (cell(p).id != -1 && cell(p).owner != me() && (magic_strength(me()) > 2*magic_strength(cell(p).owner) || (round() >= 100 && 2*magic_strength(me()) > magic_strength(cell(p).owner)) ) ) ) {
        if (LPosD.find(p) == LPosD.end() || LPosD[p].dist > front.dist) {
          if (LPosD.find(p) != LPosD.end()) bfsqueue.push(LPosD[p].id);
          LPosD[p] = front;
          return;
        }
      }
      pendientes.pop();
    }
  }

    //( ), , , 


  void atacarcerca(Unit wiz) {
    Pos p = wiz.pos;
    vector<int> idWizs = wizards(me());

    for (int i = 0; i < int(wdirs.size()); ++i) {
      if (celdaValida(p + wdirs[i]) && cell(p + wdirs[i]).id != -1) {
        Unit pwiz = unit(cell(p + wdirs[i]).id);
        if (setWiz.find(pwiz.id) == setWiz.end() || pwiz.is_in_conversion_process()) {
          move(wiz.id, wdirs[i]);
          // cerr << "soy: " << wiz.id  << " estoy: " << p << " ataco: " << p+wdirs[i] << endl;
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
    int ghostid = ghost(me());
    bfsqueue.push(ghostid);

    // Inicializa un set con todos
    for (int k = 0; k < int(wids.size()); ++k) setWiz.insert(wids[k]);
    setWiz.insert(ghostid);  //meter al fantasma
    if (round() > 50 && unit(ghostid).rounds_pending == 0) spell(ghostid, asignar_grupos());

    for (int i = 0; i < int(wids.size()); ++i) {
      Unit wiz = unit(wids[i]);
      // Huir
      if (abs(posVi - wiz.pos.i) <= 5 && abs(posVj - wiz.pos.j) <= 5) {
        if (abs(posVi - wiz.pos.i) > abs(posVj - wiz.pos.j)) {
          if (posVi > wiz.pos.i && celdaValida(wiz.pos.i - 1, wiz.pos.j))
            move(wids[i], Up);
          else if (posVi < wiz.pos.i && celdaValida(wiz.pos.i + 1, wiz.pos.j))
            move(wids[i], Down);
        } else {
          if (posVj > wiz.pos.j && celdaValida(wiz.pos.i, wiz.pos.j - 1))
            move(wids[i], Left);
          else if (posVj < wiz.pos.j && celdaValida(wiz.pos.i, wiz.pos.j + 1))
            move(wids[i], Right);
        }
      }

      // Atacar si esta cerca
      atacarcerca(wiz);

      // BFS
      bfsqueue.push(wiz.id);
      while (!bfsqueue.empty()) {
        if (ghostid == bfsqueue.front()) BFS(bfsqueue.front(), bfsqueue, true);
        BFS(bfsqueue.front(), bfsqueue, false);
        bfsqueue.pop();
      }
    }


    map<Pos, LibWiz>::iterator it = LPosD.begin();
    while (it != LPosD.end()) {
      //cerr << it->second.id << ' ' << it->second.dist << endl;
      move(it->second.id, it->second.mov);
      ++it;
    }
  }
};
/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);