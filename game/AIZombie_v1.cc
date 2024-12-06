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
  // Direcciones posibles
  const vector<Dir> wdirs = {Up, Down, Left, Right};

  // Estructura para almacenar información del mago y libro
  struct LibWiz {
    int dist; // Distancia desde el mago al objetivo
    Pos p;    // Posición actual en el BFS
    Dir mov;  // Primer movimiento hacia el objetivo
    int id;   // ID del mago
  };

  // Mapa para almacenar la asignación de libros a magos
  map<Pos, LibWiz> LPosD; // Map de posición de libro a información del mago

  // Mapa para almacenar el movimiento asignado a cada mago
  map<int, Dir> wizard_moves; // Map de ID de mago a dirección de movimiento

  // Conjunto para rastrear los magos que ya se han movido
  set<int> wizards_moved;

  // Función para verificar si una celda es válida
  bool celdaValida(Pos p) {
    return pos_ok(p) && cell(p).type != Wall;
  }

  // BFS para encontrar el libro más cercano o explorar
  void BFS(Pos p, int wiz_id, queue<int>& bfs_queue, map<int, int>& requeue_count) {
    vector<vector<bool>> casVistas(board_rows(), vector<bool>(board_cols(), false));
    queue<LibWiz> pendientes;
    casVistas[p.i][p.j] = true;

    // Inicializar BFS con movimientos posibles desde la posición actual
    for (int i = 0; i < int(wdirs.size()); ++i) {
      Pos pm = p + wdirs[i];
      if (celdaValida(pm) && !casVistas[pm.i][pm.j]) {
        pendientes.push(LibWiz{1, pm, wdirs[i], wiz_id});
        casVistas[pm.i][pm.j] = true;
      }
    }

    bool found_book = false;
    LibWiz first_empty; // Para almacenar el primer movimiento si no hay libros
    bool first_move_assigned = false;

    while (!pendientes.empty()) {
      LibWiz front = pendientes.front();
      pendientes.pop();
      p = front.p;

      // Almacenar el primer movimiento válido si aún no lo hemos hecho
      if (!first_move_assigned) {
        first_empty = front;
        first_move_assigned = true;
      }

      if (cell(p).book) {
        found_book = true;
        if (LPosD.find(p) == LPosD.end()) {
          // El libro no ha sido reclamado, lo asignamos al mago actual
          LPosD[p] = front;
          wizard_moves[wiz_id] = front.mov;
        } else {
          LibWiz ant = LPosD[p];
          if (front.dist < ant.dist) {
            // El mago actual tiene una distancia menor, toma el libro
            LPosD[p] = front;
            wizard_moves[wiz_id] = front.mov;

            // El mago anterior debe buscar otro libro
            wizard_moves.erase(ant.id);
            bfs_queue.push(ant.id);
            requeue_count[ant.id]++;
          }
          // Si el mago actual tiene una distancia mayor o igual, no hace nada
        }
        // Terminamos el BFS para este mago ya que encontró un libro
        return;
      }

      // Continuamos el BFS explorando las celdas adyacentes
      for (int i = 0; i < int(wdirs.size()); ++i) {
        Pos pm = p + wdirs[i];
        if (celdaValida(pm) && !casVistas[pm.i][pm.j]) {
          pendientes.push(LibWiz{front.dist + 1, pm, front.mov, wiz_id});
          casVistas[pm.i][pm.j] = true;
        }
      }
    }

    // Si no se encontró un libro, asignamos el primer movimiento válido para explorar
    if (!found_book && first_move_assigned) {
      wizard_moves[wiz_id] = first_empty.mov;
    }
    // Si no hay movimientos posibles, el mago se queda en su lugar
  }

  // Función para atacar si hay enemigos cerca
  bool atacarcerca(Unit wiz) {
    Pos p = wiz.pos;
    for (int i = 0; i < int(wdirs.size()); ++i) {
      Pos adj = p + wdirs[i];
      if (celdaValida(adj) && cell(adj).id != -1) {
        int adj_id = cell(adj).id;
        Unit adj_unit = unit(adj_id);
        // Comprobar si es una unidad enemiga y puede ser atacada
        if (adj_unit.player != me()) {
          move(wiz.id, wdirs[i]);
          return true;
        }
      }
    }
    return false;
  }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play() {
    vector<int> wids = wizards(me());
    Pos posV = pos_voldemort();

    // Limpiar estructuras de datos para esta ronda
    LPosD.clear();
    wizard_moves.clear();
    wizards_moved.clear();

    queue<int> bfs_queue;
    map<int, int> requeue_count;

    // Inicializar cola de BFS y procesar magos
    for (int i = 0; i < int(wids.size()); ++i) {
      int wiz_id = wids[i];
      Unit wiz = unit(wiz_id);

      bool moved = false;

      // Huir si Voldemort está cerca
      if (abs(posV.i - wiz.pos.i) <= 5 && abs(posV.j - wiz.pos.j) <= 5) {
        Dir escapeDir = Up; // Valor por defecto
        bool escape = false;
        if (abs(posV.i - wiz.pos.i) > abs(posV.j - wiz.pos.j)) {
          if (posV.i > wiz.pos.i && celdaValida(wiz.pos + Up)) { escapeDir = Up; escape = true; }
          else if (posV.i < wiz.pos.i && celdaValida(wiz.pos + Down)) { escapeDir = Down; escape = true; }
        } else {
          if (posV.j > wiz.pos.j && celdaValida(wiz.pos + Left)) { escapeDir = Left; escape = true; }
          else if (posV.j < wiz.pos.j && celdaValida(wiz.pos + Right)) { escapeDir = Right; escape = true; }
        }
        if (escape) {
          move(wiz_id, escapeDir);
          wizards_moved.insert(wiz_id);
          moved = true;
        }
      }

      // Atacar si hay enemigos cerca
      if (!moved && atacarcerca(wiz)) {
        wizards_moved.insert(wiz_id);
        moved = true;
      }

      // Si el mago no se ha movido, lo agregamos a la cola de BFS
      if (!moved) {
        bfs_queue.push(wiz_id);
        requeue_count[wiz_id] = 0;
      }
    }

    const int MAX_REQUEUE_LIMIT = 3; // Límite para evitar bucles infinitos

    // Procesar BFS para los magos en la cola
    while (!bfs_queue.empty()) {
      int wiz_id = bfs_queue.front();
      bfs_queue.pop();

      // Si el mago ya se movió, continuamos
      if (wizards_moved.find(wiz_id) != wizards_moved.end()) continue;

      if (requeue_count[wiz_id] > MAX_REQUEUE_LIMIT) continue; // Evitar bucles infinitos

      Unit wiz = unit(wiz_id);
      BFS(wiz.pos, wiz_id, bfs_queue, requeue_count);
    }

    // Mover los magos hacia sus objetivos asignados
    for (auto it = wizard_moves.begin(); it != wizard_moves.end(); ++it) {
      int wiz_id = it->first;
      if (wizards_moved.find(wiz_id) == wizards_moved.end()) {
        Dir dir = it->second;
        move(wiz_id, dir);
        wizards_moved.insert(wiz_id);
      }
    }
  }
};

/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

