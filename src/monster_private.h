#ifndef _MONSTER_PRIVATE_H_
#define _MONSTER_PRIVATE_H_

#include "coord.h"
#include "things.h"

/* Find proper destination for monster */
void monster_find_new_target(THING* tp);

void monster_start_chasing(THING* monster);
void monster_set_target(THING* mon, coord* target);

#endif /* _MONSTER_PRIVATE_H_ */