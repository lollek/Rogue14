#pragma once

#include <list>
#include <vector>
#include <string>

#include "monster.h"
#include "item.h"
#include "io.h"

struct place {
  place()
    : p_ch(SHADOW), is_passage(false), is_discovered(false), is_real(true),
      passage_number(0), trap_type(0), p_monst(nullptr)
  {}

  char     p_ch;
  bool     is_passage;
  bool     is_discovered;
  bool     is_real;
  size_t   passage_number;
  size_t   trap_type;
  Monster* p_monst;
};

class Level {
public:
  Level();
  ~Level();

  // Getters
  Monster* get_monster(int x, int y);
  Monster* get_monster(Coordinate const& coord);
  Item* get_item(int x, int y);
  Item* get_item(Coordinate const& coord);
  bool is_passage(int x, int y);
  bool is_passage(Coordinate const& coord);
  bool is_discovered(int x, int y);
  bool is_discovered(Coordinate const& coord);
  bool is_real(int x, int y);
  bool is_real(Coordinate const& coord);
  char get_ch(int x, int y);
  char get_ch(Coordinate const& coord);
  size_t get_trap_type(int x, int y);
  size_t get_trap_type(Coordinate const& coord);
  size_t get_passage_number(int x, int y);
  size_t get_passage_number(Coordinate const& coord);
  char get_type(int x, int y);
  char get_type(Coordinate const& coord);
  bool get_random_room_coord(room* room, Coordinate* coord, int tries, bool monster);
  room* get_room(Coordinate const& coord);
  room* get_passage(Coordinate const& coord);
  Coordinate const& get_stairs_pos() const;
  int get_stairs_x() const;
  int get_stairs_y() const;

  // Setters
  void set_monster(int x, int y, Monster* monster);
  void set_monster(Coordinate const& coord, Monster* monster);
  void set_passage(int x, int y);
  void set_passage(Coordinate const& coord);
  void set_discovered(int x, int y);
  void set_discovered(Coordinate const& coord);
  void set_real(int x, int y);
  void set_real(Coordinate const& coord);
  void set_not_real(int x, int y);
  void set_not_real(Coordinate const& coord);
  void set_ch(int x, int y, char ch);
  void set_ch(Coordinate const& coord, char ch);
  void set_trap_type(int x, int y, size_t type);
  void set_trap_type(Coordinate const& coord, size_t type);
  void set_passage_number(int x, int y, size_t number);
  void set_passage_number(Coordinate const& coord, size_t number);

  // Misc
  void wizard_show_passages();

  // Variables
  std::list<Item*> items;   // List of items on level

private:

  // Parts of constructor
  static int constexpr max_items = 9;
  static int constexpr max_monsters = 10;
  static int constexpr max_traps = 10;
  static int constexpr treasure_room_chance = 5;
  static int constexpr treasure_room_max_items = 10;
  static int constexpr treasure_room_min_items = 2;

  void create_rooms();
  void create_passages();
  void create_loot();
  void create_traps();
  void create_stairs();

  // Part of create_rooms()
  void create_treasure_room();
  void draw_room(room const& room);
  void draw_maze(room const& room);
  void draw_maze_recursive(int y, int x, int starty, int startx, int maxy, int maxx);

  // Part of create_passages()
  void place_door(room* room, Coordinate* coord);
  void place_passage(Coordinate* coord);
  void connect_passages(int r1, int r2);
  void number_passage(int x, int y);

  // Misc
  place& get_place(int x, int y);

  // Variables
  std::vector<place> places;        // level map
  std::vector<room>  passages;      // Passages between rooms
  Coordinate         stairs_coord;  // Where the stairs are
};
