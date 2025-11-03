#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <limits>


extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).

 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

// Client-side observed map and helpers
static char observed_map[35][35];  // '?', '@', '0'..'8'
static const int dr8[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
static const int dc8[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
inline bool inb(int r, int c) { return r >= 0 && r < rows && c >= 0 && c < columns; }

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize observed map
  for (int i = 0; i < 35; ++i) {
    for (int j = 0; j < 35; ++j) observed_map[i][j] = '?';
  }
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  for (int i = 0; i < rows; ++i) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns && j < (int)line.size(); ++j) {
      observed_map[i][j] = line[j];
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // 1) Deterministic logic first
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      char ch = observed_map[r][c];
      if (ch >= '0' && ch <= '8') {
        int k = ch - '0';
        int unknown_cnt = 0, marked_cnt = 0;
        int last_ur = -1, last_uc = -1;
        for (int d = 0; d < 8; ++d) {
          int nr = r + dr8[d], nc = c + dc8[d];
          if (!inb(nr, nc)) continue;
          if (observed_map[nr][nc] == '?') {
            ++unknown_cnt;
            last_ur = nr; last_uc = nc;
          } else if (observed_map[nr][nc] == '@') {
            ++marked_cnt;
          }
        }
        if (marked_cnt == k && unknown_cnt > 0) {
          // All unknown neighbors must be safe -> auto explore
          Execute(r, c, 2);
          return;
        }
        if (marked_cnt + unknown_cnt == k && unknown_cnt > 0) {
          // All unknown neighbors are mines -> mark one of them
          Execute(last_ur, last_uc, 1);
          return;
        }
      }
    }
  }

  // 2) Heuristic guess: choose cell with minimal naive risk from adjacent equations
  double best_risk = std::numeric_limits<double>::infinity();
  int best_r = -1, best_c = -1;
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      if (observed_map[r][c] != '?') continue;
      double risk = 0.0;
      bool constrained = false;
      for (int d = 0; d < 8; ++d) {
        int nr = r + dr8[d], nc = c + dc8[d];
        if (!inb(nr, nc)) continue;
        char ch = observed_map[nr][nc];
        if (ch >= '0' && ch <= '8') {
          constrained = true;
          int k = ch - '0';
          int marked_cnt = 0, unknown_cnt = 0;
          for (int d2 = 0; d2 < 8; ++d2) {
            int ar = nr + dr8[d2], ac = nc + dc8[d2];
            if (!inb(ar, ac)) continue;
            if (observed_map[ar][ac] == '@') ++marked_cnt;
            else if (observed_map[ar][ac] == '?') ++unknown_cnt;
          }
          int need = k - marked_cnt;
          if (need < 0) need = 0;
          if (unknown_cnt > 0) risk += (double)need / (double)unknown_cnt;
        }
      }
      if (risk < best_risk || (best_r == -1)) {
        best_risk = risk;
        best_r = r; best_c = c;
      }
    }
  }
  if (best_r != -1) {
    Execute(best_r, best_c, 0);
    return;
  }

  // Fallback (should not happen): visit (0,0)
  Execute(0, 0, 0);
}

#endif