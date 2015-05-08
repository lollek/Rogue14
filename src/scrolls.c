/*
 * Read a scroll and let it happen
 *
 * @(#)scrolls.c	4.44 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>

#include "scrolls.h"
#include "io.h"
#include "pack.h"
#include "list.h"
#include "monster.h"
#include "misc.h"
#include "level.h"
#include "player.h"
#include "potions.h"
#include "rings.h"
#include "weapons.h"
#include "wand.h"
#include "rogue.h"

char *s_names[NSCROLLS];
struct obj_info scr_info[NSCROLLS] = {
    { "monster confusion",		 7, 140, NULL, false },
    { "magic mapping",			 4, 150, NULL, false },
    { "hold monster",			 2, 180, NULL, false },
    { "sleep",				 3,   5, NULL, false },
    { "enchant armor",			 7, 160, NULL, false },
    { "identify",			43, 115, NULL, false },
    { "scare monster",			 3, 200, NULL, false },
    { "food detection",			 2,  60, NULL, false },
    { "teleportation",			 5, 165, NULL, false },
    { "enchant weapon",			 8, 150, NULL, false },
    { "create monster",			 4,  75, NULL, false },
    { "remove curse",			 7, 105, NULL, false },
    { "aggravate monsters",		 3,  20, NULL, false },
    { "protect armor",			 2, 250, NULL, false },
};

static void
set_know(THING *obj, struct obj_info *info)
{
  char **guess;

  info[obj->o_which].oi_know = true;
  obj->o_flags |= ISKNOW;
  guess = &info[obj->o_which].oi_guess;
  if (*guess)
  {
    free(*guess);
    *guess = NULL;
  }
}

static bool
enchant_players_armor(void)
{
  THING *arm = pack_equipped_item(EQUIPMENT_ARMOR);
  if (arm == NULL)
  {
    switch (rnd(3))
    {
      case 0: msg("you are unsure if anything happened"); break;
      case 1: msg("you feel naked"); break;
      case 2: msg("you feel like something just touched you"); break;
    }
    return false;
  }

  arm->o_arm--;
  arm->o_flags &= ~ISCURSED;
  msg("your armor glows %s for a moment", pick_color("silver"));
  return true;
}

/* Stop all monsters within two spaces from chasing after the hero. */
static bool
hold_monsters(void)
{
  int x, y;
  int monsters_affected = 0;
  coord *player_pos = player_get_pos();
  THING *monster;

  for (x = player_pos->x - 2; x <= player_pos->x + 2; x++)
    if (x >= 0 && x < NUMCOLS)
      for (y = player_pos->y - 2; y <= player_pos->y + 2; y++)
        if (y >= 0 && y <= NUMLINES - 1)
        {
          monster = moat(y, x);
          if (monster != NULL && on(*monster, ISRUN))
          {
            monster_become_held(monster);
            monsters_affected++;
          }
        }

  if (monsters_affected == 1)
    msg("the monster freezes");
  else if (monsters_affected > 1)
    msg("the monsters around you freeze");
  else /* monsters_affected == 0 */
    switch (rnd(3))
    {
      case 0: msg("you are unsure if anything happened"); break;
      case 1: msg("you feel a strange sense of loss"); break;
      case 2: msg("you feel a powerful aura"); break;
    }

  return monsters_affected;
}

static bool
create_monster(void)
{
  const coord *player_pos = player_get_pos();
  THING *obj;
  coord mp;
  int x, y;
  int i = 0;

  for (y = player_pos->y - 1; y <= player_pos->y + 1; y++)
    for (x = player_pos->x - 1; x <= player_pos->x + 1; x++)
    {
      char ch = winat(y, x);

      if ((y == player_pos->y && x == player_pos->x)
          ||(!step_ok(ch))
          ||(ch == SCROLL && find_obj(y, x)->o_which == S_SCARE)
          ||(rnd(++i) != 0))
        continue;

      mp.y = y;
      mp.x = x;
    }

  if (i == 0)
    switch (rnd(3))
    {
      case 0: msg("you are unsure if anything happened"); break;
      case 1: msg("you hear a faint cry of anguish in the distance"); break;
      case 2: msg("you think you felt someone's presence"); break;
    }
  else
  {
    obj = new_item();
    monster_new(obj, monster_random(false), &mp);
    msg("A %s appears out of thin air", monsters[obj->t_type - 'A'].m_name);
  }

  return i;
}

static void
magic_mapping(void)
{
  int x, y;
  char ch;

  /* take all the things we want to keep hidden out of the window */
  for (y = 1; y < NUMLINES - 1; y++)
    for (x = 0; x < NUMCOLS; x++)
    {
      PLACE *pp = INDEX(y, x);
      switch (ch = pp->p_ch)
      {
        case DOOR: case STAIRS: break;

        case HWALL: case VWALL:
          if (!(pp->p_flags & F_REAL))
          {
            ch = pp->p_ch = DOOR;
            pp->p_flags |= F_REAL;
          }
          break;

        case SHADOW:
          if (pp->p_flags & F_REAL)
            goto def;
          pp->p_flags |= F_REAL;
          ch = pp->p_ch = PASSAGE;
          /* FALLTHROUGH */

        case PASSAGE:
pass:
          if (!(pp->p_flags & F_REAL))
            pp->p_ch = PASSAGE;
          pp->p_flags |= (F_SEEN|F_REAL);
          ch = PASSAGE;
          break;

        case FLOOR:
          if (pp->p_flags & F_REAL)
            ch = SHADOW;
          else
          {
            ch = TRAP;
            pp->p_ch = TRAP;
            pp->p_flags |= (F_SEEN|F_REAL);
          }
          break;

        default:
def:
          if (pp->p_flags & F_PASS)
            goto pass;
          ch = SHADOW;
          break;
      }

      if (ch != SHADOW)
      {
        THING *obj = pp->p_monst;
        if (obj != NULL)
          obj->t_oldch = ch;
        if (obj == NULL || !player_can_sense_monsters())
          mvaddcch(y, x, ch);
      }
    }
}

static bool
food_detection(void)
{
  bool food_seen = false;
  THING *obj;

  wclear(hw);

  for (obj = lvl_obj; obj != NULL; obj = obj->l_next)
    if (obj->o_type == FOOD)
    {
      food_seen = true;
      mvwaddcch(hw, obj->o_pos.y, obj->o_pos.x, FOOD);
    }

  if (food_seen)
    show_win("Your nose tingles and you smell food.--More--");
  else
    msg("your nose tingles");

  return food_seen;
}

static bool
player_enchant_weapon(void)
{
  THING *weapon = pack_equipped_item(EQUIPMENT_RHAND);
  if (weapon == NULL)
  {
    switch (rnd(2))
    {
      case 0: msg("you feel a strange sense of loss"); break;
      case 1: msg("you are unsure if anything happened"); break;
    }
    return false;
  }

  weapon->o_flags &= ~ISCURSED;
  if (rnd(2) == 0)
    weapon->o_hplus++;
  else
    weapon->o_dplus++;
  msg("your %s glows %s for a moment",
      weap_info[weapon->o_which].oi_name, pick_color("blue"));

  return true;
}

static void
remove_curse(void)
{
  int i;
  for (i = 0; i < NEQUIPMENT; ++i)
    if (pack_equipped_item(i) != NULL)
      pack_uncurse_item(pack_equipped_item(i));

  msg(player_is_hallucinating()
      ? "you feel in touch with the Universal Onenes"
      : "you feel as if somebody is watching over you");
}

static bool
protect_armor(void)
{
  THING *arm = pack_equipped_item(EQUIPMENT_ARMOR);
  if (arm == NULL)
  {
    switch (rnd(2))
    {
      case 0: msg("you feel a strange sense of loss"); break;
      case 1: msg("you are unsure if anything happened"); break;
    }
    return false;
  }

  arm->o_flags |= ISPROT;
  msg("your armor is covered by a shimmering %s shield", pick_color("gold"));
  return true;
}

void
identify(void)
{
  THING *obj;

  if (pack_is_empty())
  {
    msg("you don't have anything in your pack to identify");
    return;
  }

  obj = pack_get_item("identify", 0);
  if (obj == NULL)
    return;

  switch (obj->o_type)
  {
    case SCROLL: set_know(obj, scr_info);  break;
    case POTION: set_know(obj, pot_info);  break;
    case STICK:  set_know(obj, __wands_ptr());   break;
    case RING:   set_know(obj, ring_info); break;
    case WEAPON: case ARMOR: obj->o_flags |= ISKNOW; break;
  }
  msg(inv_name(obj, false));
}

bool
read_scroll(void)
{
  THING *obj = pack_get_item("read", SCROLL);
  THING *orig_obj;
  bool discardit = false;

  if (obj == NULL)
    return false;

  if (obj->o_type != SCROLL)
  {
    msg(terse
      ? "nothing to read"
      : "there is nothing on it to read");
    return false;
  }

  /* Get rid of the thing */
  discardit = (bool)(obj->o_count == 1);
  pack_remove(obj, false, false);
  orig_obj = obj;

  switch (obj->o_which)
  {
    case S_CONFUSE:
      player_set_confusing_attack();
      break;
    case S_ARMOR:
      if (enchant_players_armor())
        learn_scroll(S_ARMOR);
      break;
    case S_HOLD:
      if (hold_monsters())
        learn_scroll(S_HOLD);
      break;
    case S_SLEEP:
      learn_scroll(S_SLEEP);
      player_fall_asleep();
      break;
    case S_CREATE:
      if (create_monster())
        learn_scroll(S_CREATE);
      break;
    case S_ID:
      learn_scroll(obj->o_which);
      msg("this scroll is an %s scroll", scr_info[obj->o_which].oi_name);
      identify();
      break;
    case S_MAP:
      learn_scroll(S_MAP);
      msg("this scroll has a map on it");
      magic_mapping();
      break;
    case S_FDET:
      if (food_detection())
        learn_scroll(S_FDET);
      break;
    case S_TELEP:
      learn_scroll(S_TELEP);
      player_teleport(NULL);
      break;
    case S_ENCH:
      player_enchant_weapon();
      break;
    case S_SCARE:
      /* Reading it is a mistake and produces laughter at her poor boo boo. */
      msg("you hear maniacal laughter in the distance");
      break;
    case S_REMOVE:
      remove_curse();
      break;
    case S_AGGR:
      /* This scroll aggravates all the monsters on the current
       * level and sets them running towards the hero */
      aggravate();
      msg("you hear a high pitched humming noise");
      break;
    case S_PROTECT:
      protect_armor();
      break;
    default:
      msg("what a puzzling scroll!");
      return true;
  }
  obj = orig_obj;
  look(true);	/* put the result of the scroll on the screen */
  status();

  call_it("scroll", &scr_info[obj->o_which]);

  if (discardit)
    discard(obj);

  return true;
}


