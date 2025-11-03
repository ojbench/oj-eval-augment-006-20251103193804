#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <queue>
#include <string>

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to use global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */
int rows;         // The count of rows of the game map. You MUST NOT modify its name.
int columns;      // The count of columns of the game map. You MUST NOT modify its name.
int total_mines;  // The count of mines of the game map. You MUST NOT modify its name. You should initialize this
                  // variable in function InitMap. It will be used in the advanced task.
int game_state;  // The state of the game, 0 for continuing, 1 for winning, -1 for losing. You MUST NOT modify its name.

// Internal state
static bool is_mine[35][35];
static bool visited_cell[35][35];
static bool marked_correct[35][35];
static bool wrong_mark[35][35];  // true if a non-mine was marked (only meaningful when game over)
static int adj_mines[35][35];
static int visited_nonmine_count;  // number of visited non-mine cells
static int marked_correct_count;   // number of correctly marked mines

static const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
static const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

inline bool in_bounds(int r, int c) { return r >= 0 && r < rows && c >= 0 && c < columns; }

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (1, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map
 * would be initialized, with all the blocks unvisited.
 */
void InitMap() {
  std::cin >> rows >> columns;
  total_mines = 0;
  game_state = 0;
  visited_nonmine_count = 0;
  marked_correct_count = 0;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      is_mine[i][j] = false;
      visited_cell[i][j] = false;
      marked_correct[i][j] = false;
      wrong_mark[i][j] = false;
      adj_mines[i][j] = 0;
    }
  }
  std::string line;
  for (int i = 0; i < rows; ++i) {
    std::cin >> line;
    for (int j = 0; j < columns && j < static_cast<int>(line.size()); ++j) {
      if (line[j] == 'X') {
        is_mine[i][j] = true;
        ++total_mines;
      }
    }
  }
  // compute adjacent mine counts
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int cnt = 0;
      for (int d = 0; d < 8; ++d) {
        int ni = i + dr[d], nj = j + dc[d];
        if (in_bounds(ni, nj) && is_mine[ni][nj]) ++cnt;
      }
      adj_mines[i][j] = cnt;
    }
  }
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), the return value would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, the return value would be -1 (game ends and the players loses) , and the
 * game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, the return value of the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void VisitBlock(int r, int c) {
  if (game_state != 0) return;
  if (!in_bounds(r, c)) return;  // invalid
  if (visited_cell[r][c]) return; // already visited
  if (marked_correct[r][c]) return; // marked as mine, ignore

  if (is_mine[r][c]) {
    visited_cell[r][c] = true;
    game_state = -1;
    return;
  }

  // BFS flood fill for zeros; also visit border numbers
  std::queue<std::pair<int, int>> q;
  q.push({r, c});
  visited_cell[r][c] = true;
  ++visited_nonmine_count;
  while (!q.empty()) {
    auto [cr, cc] = q.front();
    q.pop();
    if (adj_mines[cr][cc] != 0) continue;  // only expand from zeros
    for (int d = 0; d < 8; ++d) {
      int nr = cr + dr[d], nc = cc + dc[d];
      if (!in_bounds(nr, nc)) continue;
      if (visited_cell[nr][nc]) continue;
      if (marked_correct[nr][nc]) continue;  // do not visit marked
      if (is_mine[nr][nc]) continue;         // never visit mines here
      visited_cell[nr][nc] = true;
      ++visited_nonmine_count;
      if (adj_mines[nr][nc] == 0) {
        q.push({nr, nc});
      }
    }
  }

  if (visited_nonmine_count == rows * columns - total_mines) {
    game_state = 1;
  }
}

/**
 * @brief The definition of function MarkMine(int, int)
 *
 * @details This function is designed to mark a mine in the game map.
 * If the block being marked is a mine, show it as "@".
 * If the block being marked isn't a mine, END THE GAME immediately. (NOTE: This is not the same rule as the real
 * game) And you don't need to
 *
 * For example, if we use the same map as before, and the current state is:
 *     1?1
 *     ???
 *     ???
 * If you call MarkMine(0, 1), you marked the right mine. Then the resulting game map is:
 *     1@1
 *     ???
 *     ???
 * If you call MarkMine(1, 0), you marked the wrong mine(There's no mine in grid (1, 0)).
 * The game_state would be -1 and game ends immediately. The game map would be:
 *     1?1
 *     X??
 *     ???
 * This is different from the Minesweeper you've played. You should beware of that.
 *
 * @param r The row coordinate (0-based) of the block to be marked.
 * @param c The column coordinate (0-based) of the block to be marked.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void MarkMine(int r, int c) {
  if (game_state != 0) return;
  if (!in_bounds(r, c)) return;
  if (visited_cell[r][c]) return;  // already visited, ignore
  if (marked_correct[r][c]) return;  // already marked

  if (is_mine[r][c]) {
    marked_correct[r][c] = true;
    ++marked_correct_count;
  } else {
    wrong_mark[r][c] = true;
    game_state = -1;
  }
}

/**
 * @brief The definition of function AutoExplore(int, int)
 *
 * @details This function is designed to auto-visit adjacent blocks of a certain block.
 * See README.md for more information
 *
 * For example, if we use the same map as before, and the current map is:
 *     ?@?
 *     ?2?
 *     ??@
 * Then auto explore is available only for block (1, 1). If you call AutoExplore(1, 1), the resulting map will be:
 *     1@1
 *     122
 *     01@
 * And the game ends (and player wins).
 */
void AutoExplore(int r, int c) {
  if (game_state != 0) return;
  if (!in_bounds(r, c)) return;
  if (!visited_cell[r][c]) return;  // only on visited non-mine numbers
  if (is_mine[r][c]) return;

  int need = adj_mines[r][c];
  int marked = 0;
  for (int d = 0; d < 8; ++d) {
    int nr = r + dr[d], nc = c + dc[d];
    if (in_bounds(nr, nc) && marked_correct[nr][nc]) ++marked;
  }
  if (marked != need) return;  // no effect

  // Visit all non-mine neighbors that are not yet visited
  for (int d = 0; d < 8; ++d) {
    int nr = r + dr[d], nc = c + dc[d];
    if (!in_bounds(nr, nc)) continue;
    if (marked_correct[nr][nc]) continue;
    if (visited_cell[nr][nc]) continue;
    if (is_mine[nr][nc]) continue;
    VisitBlock(nr, nc);
    if (game_state != 0) return;  // game ended
  }
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game.
 * It outputs a line according to the result, and a line of two integers, visit_count and marked_mine_count,
 * representing the number of blocks visited and the number of marked mines taken respectively.
 *
 * @note If the player wins, we consider that ALL mines are correctly marked.
 */
void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
    std::cout << visited_nonmine_count << " " << total_mines << std::endl;
  } else if (game_state == -1) {
    std::cout << "GAME OVER!" << std::endl;
    std::cout << visited_nonmine_count << " " << marked_correct_count << std::endl;
  }
  exit(0);  // Exit the game immediately
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch = '?';
      if (game_state == 1) {
        // victory: show all mines as '@', others numbers
        if (is_mine[i][j]) ch = '@';
        else ch = static_cast<char>('0' + adj_mines[i][j]);
      } else {
        if (wrong_mark[i][j]) ch = 'X';
        else if (visited_cell[i][j]) ch = is_mine[i][j] ? 'X' : static_cast<char>('0' + adj_mines[i][j]);
        else if (marked_correct[i][j]) ch = '@';
        else ch = '?';
      }
      std::cout << ch;
    }
    std::cout << std::endl;
  }
}

#endif
