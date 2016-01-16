#pragma once

#include <list>

#include "monster.h"
#include "io.h"

/* Flags for level map */
#define F_PASS		0x80		/* is a passageway */
#define F_SEEN		0x40		/* have seen this spot before */
#define F_DROPPED	0x20		/* object was dropped here */
#define F_LOCKED	0x20		/* door is locked */
#define F_REAL		0x10		/* what you see is what you get */
#define F_PNUM		0x0f		/* passage number mask */
#define F_TMASK		0x07		/* trap number mask */

/* describe a place on the level map */
typedef struct {
    char     p_ch;
    char     p_flags;
    Monster* p_monst;
} PLACE;

class Level {
public:
  Level(int relative_level); /* +1 for stairs up, -1 if you go down */
  ~Level() = default;

  PLACE* get_place(int x, int y);
  PLACE* get_place(Coordinate const& coord);
  Monster* get_monster(int x, int y);
  Monster* get_monster(Coordinate const& coord);
  char get_flags(int x, int y);
  char get_flags(Coordinate const& coord);
  char get_ch(int x, int y);
  char get_ch(Coordinate const& coord);
  char get_type(int x, int y);
  char get_type(Coordinate const& coord);

  void set_monster(int x, int y, Monster* monster);
  void set_monster(Coordinate const& coord, Monster* monster);
  void set_flags(int x, int y, char flags);
  void set_flags(Coordinate const& coord, char flags);
  void set_ch(int x, int y, char ch);
  void set_ch(Coordinate const& coord, char ch);

  int static constexpr amulet_min_level = 26;
  int static           levels_without_food;
  int static           max_level_visited;
  int static           current_level;

private:
  void create_rooms();

  PLACE              level_places[MAXLINES*MAXCOLS];  /* level map */
};

extern Coordinate         level_stairs;                    /* Location of staircase */
extern std::list<Item*>   level_items;                     /* List of items on level */
