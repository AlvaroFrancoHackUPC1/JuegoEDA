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

  /**
   * Play method, invoked once per each round.
   */
  virtual void play() {
    if (round() % 2 == 0 and rounds_spell_resting_ghost() == 0) {
      int ghostid = ghost(me());
      vector<int> res = encantamiento(spell_ingredients());
      spell(ghostid, res);
    }
    vector<int> wids = wizards(me());
    for (int i = 0; i < int(wids.size()); ++i) {
      bool libro = false;
      Unit wiz = unit(wids[i]);
      Pos posrel(0, 0);
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

      Dir mov = Up;  // Default
      if (libro) {
        if (posrel.i == 1)
          mov = Down;
        else if (posrel.j == 1)
          mov = Right;
        else if (posrel.j == -1)
          mov = Left;
        move(wids[i], mov);
      } else {
        Dir mov = dirs[random(0, dirs.size() - 1)];
        Pos new_pos = unit(wids[i]).pos + mov;
        if (pos_ok(new_pos) and cell(new_pos.i, new_pos.j).type != Wall)
          move(wids[i], mov);
      }
    }
  }
};

/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
