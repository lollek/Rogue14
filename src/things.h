#pragma once

#include <list>
#include <vector>
#include <string>

#include "Coordinate.h"
#include "damage.h"
#include "item.h"

/* Stuff about objects */
struct obj_info {
  std::string oi_name;
  size_t      oi_prob;
  size_t      oi_worth;
  std::string oi_guess;
  bool        oi_know;
};

extern std::vector<obj_info> things;

/* Return the name of something as it would appear in an inventory. */
std::string inv_name(Item const* item, bool drop);

/* Put something down */
bool drop(void);

/* Return a new thing */
Item* new_thing(void);
Item* new_amulet(void);
Item* new_food(int which);

/* Pick an item out of a list of nitems possible objects */
size_t pick_one(std::vector<obj_info>& start, size_t nitems);
