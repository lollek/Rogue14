#include <exception>

using namespace std;

#include "gold.h"
#include "error_handling.h"
#include "game.h"
#include "coordinate.h"
#include "io.h"
#include "pack.h"
#include "monster.h"
#include "misc.h"
#include "level.h"
#include "player.h"
#include "os.h"
#include "rogue.h"

#include "level_rooms.h"

/* position matrix for maze positions */
struct spot {
  int        nexits;
  Coordinate exits[4];
  int        used;
};

static spot maze[NUMLINES/3+1][NUMCOLS/3+1];

/* Called to illuminate a room.
 * If it is dark, remove anything that might move.  */
static void
room_open_door(struct room* rp) {
  if (rp == nullptr || rp->r_flags & ISGONE) {
    return;
  }

  for (int y = rp->r_pos.y; y < rp->r_pos.y + rp->r_max.y; y++) {
    for (int x = rp->r_pos.x; x < rp->r_pos.x + rp->r_max.x; x++) {
      Game::level->set_discovered(x, y);
      Monster* monster = Game::level->get_monster(x, y);
      if (monster != nullptr) {
        monster_notice_player(y, x);
      }
    }
  }
}

/* Account for maze exits */
static void
room_accnt_maze(int y, int x, int ny, int nx) {
  spot* sp = &maze[y][x];
  Coordinate* cp;

  for (cp = sp->exits; cp < &sp->exits[sp->nexits]; cp++) {
    if (cp->y == ny && cp->x == nx) {
      return;
    }
  }

  cp->y = ny;
  cp->x = nx;
}


/* Dig out from around where we are now, if possible */
void
Level::draw_maze_recursive(int y, int x, int starty, int startx, int maxy, int maxx) {
  int nexty = 0;
  int nextx = 0;

  for (;;) {
    Coordinate const del[4] = { {2, 0}, {-2, 0}, {0, 2}, {0, -2} };
    int cnt = 0;
    for (unsigned i = 0; i < sizeof(del)/sizeof(*del); ++i) {
      int newy = y + del[i].y;
      int newx = x + del[i].x;

      if (newy < 0 || newy > maxy || newx < 0 || newx > maxx
          || is_passage(newx + startx, newy + starty)) {
        continue;
      }

      if (os_rand_range(++cnt) == 0) {
        nexty = newy;
        nextx = newx;
      }
    }

    if (cnt == 0) {
      return;
    }

    room_accnt_maze(y, x, nexty, nextx);
    room_accnt_maze(nexty, nextx, y, x);

    Coordinate pos;
    if (nexty == y) {
      pos.y = y + starty;
      if (nextx - x < 0) {
        pos.x = nextx + startx + 1;
      } else {
        pos.x = nextx + startx - 1;
      }
    } else {
      pos.x = x + startx;
      if (nexty - y < 0) {
        pos.y = nexty + starty + 1;
      } else {
        pos.y = nexty + starty - 1;
      }
    }
    place_passage(&pos);

    pos.y = nexty + starty;
    pos.x = nextx + startx;
    place_passage(&pos);

    draw_maze_recursive(nexty, nextx, starty, startx, maxy, maxx);
  }
}

/* Dig a maze */
void
Level::draw_maze(room const& rp) {
  for (spot* sp = &maze[0][0]; sp <= &maze[NUMLINES / 3][NUMCOLS / 3]; sp++) {
    sp->used = false;
    sp->nexits = 0;
  }

  int y = (os_rand_range(rp.r_max.y) / 2) * 2;
  int x = (os_rand_range(rp.r_max.x) / 2) * 2;

  // TODO: Is this a typo/incorrect? maybe x + rp->r_pos.x
  Coordinate pos(y + rp.r_pos.x, y + rp.r_pos.y);
  place_passage(&pos);

  draw_maze_recursive(y, x, rp.r_pos.y, rp.r_pos.x, rp.r_max.y, rp.r_max.x);
}

/* Draw a box around a room and lay down the floor for normal
 * rooms; for maze rooms, draw maze. */
void
Level::draw_room(room const& rp) {

  /* Draw left + right side */
  for (int y = rp.r_pos.y + 1; y <= rp.r_max.y + rp.r_pos.y - 1; y++) {
    set_tile(rp.r_pos.x, y, Tile::Wall);
    set_tile(rp.r_pos.x + rp.r_max.x - 1, y, Tile::Wall);
  }

  /* Draw top + bottom side */
  for (int x = rp.r_pos.x; x <= rp.r_pos.x + rp.r_max.x - 1; x++) {
    set_tile(x, rp.r_pos.y, Tile::Wall);
    set_tile(x, rp.r_pos.y + rp.r_max.y - 1, Tile::Wall);
  }

  /* Put the floor down */
  for (int y = rp.r_pos.y + 1; y < rp.r_pos.y + rp.r_max.y - 1; y++) {
    for (int x = rp.r_pos.x + 1; x < rp.r_pos.x + rp.r_max.x - 1; x++) {
      Tile& t = tile(x, y);
      t.type = Tile::Floor;
      t.is_dark = rp.r_flags & ISDARK;
    }
  }
}

static void
room_place_gone_room(Coordinate const* max_size, Coordinate const* top, room* room) {
  /** Place a gone room.  Make certain that there is a blank line
   * for passage drawing.  */
  do {
    room->r_pos.x = top->x + os_rand_range(max_size->x - 3) + 1;
    room->r_pos.y = top->y + os_rand_range(max_size->y - 2) + 1;
    room->r_max.x = -NUMCOLS;
    room->r_max.y = -NUMLINES;
  } while (!(room->r_pos.y > 0 && room->r_pos.y < NUMLINES-1 &&
             room->r_pos.x > 0 && room->r_pos.x < NUMCOLS -1));
}

void
Level::create_rooms() {

  /* Clear things for a new level */
  for (struct room& room : rooms) {
    room.r_goldval = 0;
    room.r_nexits = 0;
    room.r_flags = 0;
  }

  /* Put the gone rooms, if any, on the level */
  int left_out = os_rand_range(4);
  for (int i = 0; i < left_out; i++) {
    get_random_room()->r_flags |= ISGONE;
  }

  /* dig and populate all the rooms on the level */
  if (rooms.size() != 9) {
    error("This functions expects there to be exacly 9 rooms"
          " but currently there are " + to_string(rooms.size()));
  }

  /* maximum room size */
  Coordinate const bsze(NUMCOLS / 3, NUMLINES / 3);

  for (int i = 0; i < static_cast<int>(rooms.size()); i++) {
    room& room = rooms.at(static_cast<size_t>(i));

    /* Find upper left corner of box that this room goes in */
    Coordinate const top((i % 3) * bsze.x + 1, (i / 3) * bsze.y);

    if (room.r_flags & ISGONE) {
      room_place_gone_room(&bsze, &top, &room);
      continue;
    }

    /* set room type */
    if (os_rand_range(10) < Game::current_level - 1) {
      room.r_flags |= ISDARK;  /* dark room */
      if (os_rand_range(15) == 0) {
        room.r_flags = ISMAZE; /* maze room */
      }
    }

    /* Find a place and size for a random room */
    if (room.r_flags & ISMAZE) {
      room.r_max.x = bsze.x - 1;
      room.r_max.y = bsze.y - 1;
      room.r_pos.x = top.x;
      room.r_pos.y = top.y;
      if (room.r_pos.y == 0) {
        room.r_pos.y++;
        room.r_max.y--;
      } else if (room.r_pos.x == 0) {
        room.r_pos.x++;
        room.r_max.x--;
      }

    } else {
      do {
        room.r_max.x = os_rand_range(bsze.x - 4) + 4;
        room.r_max.y = os_rand_range(bsze.y - 4) + 4;
        room.r_pos.x = top.x + os_rand_range(bsze.x - room.r_max.x);
        room.r_pos.y = top.y + os_rand_range(bsze.y - room.r_max.y);
      } while (room.r_pos.y == 0 || room.r_pos.x == 0);
    }

    if (room.r_flags & ISMAZE) {
      draw_maze(room);
    } else {
      draw_room(room);
    }

    /* Put the gold in */
    if (os_rand_range(2) == 0 &&
        (!pack_contains_amulet() || Game::current_level >= Game::max_level_visited)) {
      get_random_room_coord(&room, &room.r_gold, 0, false);
      Gold *gold = new Gold();
      gold->set_position(room.r_gold);
      room.r_goldval = gold->get_amount();
      items.push_back(gold);
    }

    /* Put the monster in */
    if (os_rand_range(100) < (room.r_goldval > 0 ? 80 : 25)) {
      Coordinate mp;
      get_random_room_coord(&room, &mp, 0, true);
      Monster* monster = new Monster(Monster::random_monster_type(), mp, &room);
      monsters.push_back(monster);
      monster_give_pack(monster);
      set_monster(mp, monster);
    }
  }
}

bool
Level::get_random_room_coord(room* room, Coordinate* coord, int tries, bool monst) {
  bool limited_tries = tries > 0;
  char compchar = Tile::Floor;
  bool pickroom = room == nullptr;

  for (;;) {
    if (limited_tries && tries-- == 0) {
      return false;
    }

    if (pickroom) {
      room = get_random_room();
    }

    /* Pick a random position */
    coord->x = room->r_pos.x + os_rand_range(room->r_max.x - 2) + 1;
    coord->y = room->r_pos.y + os_rand_range(room->r_max.y - 2) + 1;

    Tile::Type ch = get_tile(*coord);
    if (monst) {
      if (get_monster(*coord) == nullptr &&
          can_step(*coord)) {
        return true;
      }
    } else if (ch == compchar) {
      return true;
    }
  }
}

void
room_enter(Coordinate const& cp) {

  struct room* rp = Game::level->get_room(cp);
  player->set_room(rp);
  room_open_door(rp);
  Game::io->print_room(rp);
}

/** room_leave:
 * Code for when we exit a room */
void
room_leave(Coordinate const& cp)
{
  (void)cp;
  struct room* rp = player->get_room();

  if (rp->r_flags & ISMAZE) {
    return;
  }

  player->set_room(nullptr);

  // If we leave dark rooms, we want to hide everything inside of it
  if (rp->r_flags & ISDARK) {
    Game::io->hide_room(rp);

  // If we leave a light room, we only hide monsters
  } else {
    for (int y = rp->r_pos.y; y < rp->r_max.y + rp->r_pos.y; y++) {
      for (int x = rp->r_pos.x; x < rp->r_max.x + rp->r_pos.x; x++) {

        // Reprint monsters (which usually hides them)
        Monster* mon = Game::level->get_monster(x, y);
        if (mon != nullptr) {
          Game::io->print_tile(x, y);
        }
      }
    }
  }

  room_open_door(rp);
}

room* Level::get_random_room() {
  for (;;) {
    room* room = &rooms.at(static_cast<size_t>(os_rand_range(rooms.size())));
    if (room->r_flags & ISGONE) {
      continue;
    }

    return room;
  }
}
