/*
 * This file contains misc functions for dealing with armor
 * @(#)armor.c	4.14 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "pack.h"
#include "rings.h"
#include "misc.h"
#include "wizard.h"
#include "things.h"
#include "options.h"
#include "os.h"
#include "rogue.h"

#include "armor.h"

static struct armor_info_t armors[NARMORS] = {
 /* name                   ac  prob value known */
 { "leather armor",         8, 20,   20,  false },
 { "ring mail",             7, 15,   25,  false },
 { "studded leather armor", 7, 15,   20,  false },
 { "scale mail",            6, 13,   30,  false },
 { "chain mail",            5, 12,   75,  false },
 { "splint mail",           4, 10,   80,  false },
 { "banded mail",           4, 10,   90,  false },
 { "plate mail",            3,  5,  150,  false },
};

char const* armor_name(enum armor_t i)  { return armors[i].name; }
int armor_ac(enum armor_t i)            { return armors[i].ac; }
int armor_value(enum armor_t i)         { return armors[i].value; }
int armor_probability(enum armor_t i)   { return armors[i].prob; }

int
armor_for_thing(THING* thing)
{
  return 20 - thing->t_stats.s_arm;
}

bool
armor_command_wear(void)
{
  THING* obj = pack_get_item("wear", ARMOR);

  if (obj == NULL)
    return false;

  if (obj->o_type != ARMOR)
  {
    msg("you can't wear that");
    return armor_command_wear();
  }

  if (pack_equipped_item(EQUIPMENT_ARMOR) != NULL)
    if (!pack_unequip(EQUIPMENT_ARMOR, false))
      return true;

  waste_time(1);
  pack_remove(obj, false, true);
  pack_equip_item(obj);

  char buf[MAXSTR];
  msg("now wearing %s", inv_name(buf, obj, true));
  return true;
}

enum armor_t
armor_type_random(void)
{
  int value = rnd(100);
  for (enum armor_t i = 0; i < NARMORS; ++i)
  {
    if (value < armors[i].prob)
      return i;
    else
      value -= armors[i].prob;
  }

  /* Error! Sum of probs was not 100 */
  msg("Error! Sum of probabilities is not 100%");
  pr_spec(ARMOR);
  readchar(false);
  endwin();
  exit(1);
}

void
armor_rust(void)
{
  THING* arm = pack_equipped_item(EQUIPMENT_ARMOR);
  if (arm == NULL || arm->o_type != ARMOR || arm->o_which == LEATHER ||
      arm->o_arm >= 9)
    return;

  if ((arm->o_flags & ISPROT) || player_has_ring_with_ability(R_SUSTARM))
  {
    if (!to_death)
      msg("the rust vanishes instantly");
  }
  else
  {
    arm->o_arm++;
    msg("your armor weakens");
  }
}

void
armor_description(THING* obj, char* buf)
{
  char *ptr = buf;
  char const* obj_name = armor_name((enum armor_t)obj->o_which);
  int bonus_ac = armor_ac((enum armor_t)obj->o_which) - obj->o_arm;
  int base_ac = 10 - obj->o_arm - bonus_ac;

  ptr += sprintf(ptr, "A%s %s [%d]", vowelstr(obj_name), obj_name, base_ac);

  if (obj->o_flags & ISKNOW)
  {
    ptr -= 1;
    ptr += sprintf(ptr, bonus_ac < 0 ? ",%d]" : ",+%d]", bonus_ac);
  }

  if (obj->o_label != NULL)
    ptr += sprintf(ptr, " called %s", obj->o_label);
}

THING*
armor_create(int which, int random_stats)
{
  THING* armor = allocate_new_item();
  memset(armor, 0, sizeof(*armor));

  if (which == -1)
    which = armor_type_random();

  armor->o_type = ARMOR;
  armor->o_which = which;
  armor->o_arm = armor_ac((enum armor_t)armor->o_which);

  if (random_stats)
  {
    int rand = rnd(100);
    if (rand < 20)
    {
      armor->o_flags |= ISCURSED;
      armor->o_arm += rnd(3) + 1;
    }
    else if (rand < 28)
      armor->o_arm -= rnd(3) + 1;
  }

  return armor;
}
