#include <list>

using namespace std;

#include "Coordinate.h"
#include "things.h"
#include "player.h"
#include "level.h"
#include "scrolls.h"
#include "misc.h"
#include "io.h"
#include "os.h"

#include "monster.h"
#include "monster_private.h"

list<monster*> monster_list;

void
monster_find_new_target(monster* monster)
{
  int prob = monsters[monster->t_type - 'A'].m_carry;
  if (prob <= 0 || monster->t_room == player_get_room()
      || monster_seen_by_player(monster)) {
    monster_set_target(monster, player_get_pos());
    return;
  }

  for (item* obj : level_items) {
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
      continue;

    if (roomin(&obj->o_pos) == monster->t_room && os_rand_range(100) < prob)
    {
      auto result = find_if(monster_list.cbegin(), monster_list.cend(),
          [&] (struct monster const* m) {
          return m->t_dest == &obj->o_pos;
      });

      if (result == monster_list.cend()) {
        monster_set_target(monster, &obj->o_pos);
        return;
      }
    }
  }

  monster_set_target(monster, player_get_pos());
}

void
monster_start_chasing(monster* mon)
{
  mon->t_flags |= ISRUN;
}

void
monster_set_target(monster* mon, Coordinate* target)
{
  mon->t_dest = target;
}