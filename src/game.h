#pragma once

#include "level.h"

class Game {
public:
  Game();
  Game(Game const&) = delete;

  ~Game() = default;

  Game& operator=(Game const&) = delete;


  static void new_level(int dungeon_level);

  static Level*        level;
  static int           current_level;
  static int constexpr amulet_min_level = 26;
  static int           levels_without_food;
  static int           max_level_visited;

private:
  int init_graphics();
};
