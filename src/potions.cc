#include <vector>
#include <string>

using namespace std;

#include "io.h"
#include "pack.h"
#include "level.h"
#include "misc.h"
#include "player.h"
#include "monster.h"
#include "colors.h"
#include "os.h"
#include "rogue.h"
#include "item.h"
#include "game.h"

#include "potions.h"

/* Colors of the potions */
static vector<string const*> p_colors;

vector<obj_info> potion_info = {
  /* io_name,      oi_prob, oi_worth, oi_guess, oi_know */
  { "confusion",         7,        5,       "", false },
  { "hallucination",     8,        5,       "", false },
  { "poison",            8,        5,       "", false },
  { "gain strength",    13,      150,       "", false },
  { "see invisible",     3,      100,       "", false },
  { "healing",          13,      130,       "", false },
  { "monster detection", 6,      130,       "", false },
  { "magic detection",   6,      105,       "", false },
  { "raise level",       2,      250,       "", false },
  { "extra healing",     5,      200,       "", false },
  { "haste self",        5,      190,       "", false },
  { "restore strength", 13,      130,       "", false },
  { "blindness",         5,        5,       "", false },
  { "levitation",        6,       75,       "", false },
};

void
potions_init(void)
{
  /* Pick a unique color for each potion */
  for (int i = 0; i < NPOTIONS; i++)
    for (;;)
    {
      size_t color = os_rand_range(color_max());

      if (find(p_colors.cbegin(), p_colors.cend(), &color_get(color)) !=
          p_colors.cend()) {
        continue;
      }

      p_colors.push_back(&color_get(color));
      break;
    }
}

/** is_quaffable
 * Can we dring thing? */
static bool
is_quaffable(Item const* item)
{
  if (item == nullptr)
    return false;
  else if (item_type(item) != POTION)
  {
    io_msg("that's undrinkable");
    return false;
  }
  else
    return true;
}

/** potion_learn
 * Hero learn what a potion does */
static void
potion_learn(enum potion_t potion)
{
  potion_info[potion].oi_know = true;
}

bool
potion_quaff_something(void)
{
  Item* obj = pack_get_item("quaff", POTION);

  /* Make certain that it is somethings that we want to drink */
  if (!is_quaffable(obj))
    return false;

  /* Calculate the effect it has on the poor guy. */
  bool discardit = obj->o_count == 1;
  pack_remove(obj, false, false);
  switch (obj->o_which)
  {
    case P_CONFUSE:
      if (!player->is_hallucinating())
        potion_learn(P_CONFUSE);
      player->set_confused();
      break;

    case P_POISON:
      potion_learn(P_POISON);
      player->become_poisoned();
      break;

    case P_HEALING:
      potion_learn(P_HEALING);
      player->restore_health(roll(player->get_level(), 4), true);
      player->set_not_blind();
      io_msg("you begin to feel better");
      break;

    case P_STRENGTH:
      potion_learn(P_STRENGTH);
      player->modify_strength(1);
      io_msg("you feel stronger, now.  What bulging muscles!");
      break;

    case P_MFIND:
      potion_learn(P_MFIND);
      player->set_sense_monsters();
      break;

    case P_TFIND:
    {
      /* Potion of magic detection.  Show the potions and scrolls */
      bool show = false;
      if (!Game::level->items.empty())
      {
        wclear(hw);
        for (Item* item : Game::level->items) {
          if (item->is_magic())
          {
            show = true;
            mvwaddcch(hw, item->get_y(), item->get_x(), MAGIC);
            potion_learn(P_TFIND);
          }
        }

        if (monster_show_if_magic_inventory())
          show = true;
     }

      if (show)
      {
        potion_learn(P_TFIND);
        show_win("You sense the presence of magic on this level.--More--");

      }
      else
        io_msg("you have a strange feeling for a moment, then it passes");
    }
    break;

    case P_LSD:
      potion_learn(P_LSD);
      player->set_hallucinating();
      break;

    case P_SEEINVIS:
      player->set_true_sight();
      break;

    case P_RAISE:
      potion_learn(P_RAISE);
      player->raise_level(1);
      break;

    case P_XHEAL:
      potion_learn(P_XHEAL);
      player->restore_health(roll(player->get_level(), 8), true);
      player->set_not_blind();
      player->set_not_hallucinating();
      io_msg("you begin to feel much better");
      break;

    case P_HASTE:
      potion_learn(P_HASTE);
      player->increase_speed();
      break;

    case P_RESTORE:
      player->restore_strength();
      break;

    case P_BLIND:
      potion_learn(P_BLIND);
      player->set_blind();
      break;

    case P_LEVIT:
      potion_learn(P_LEVIT);
      player->set_levitating();
      break;

    default:
      io_msg("what an odd tasting potion!");
      return true;
  }
  io_refresh_statusline();

  /* Throw the item away */
  call_it("potion", &potion_info.at(static_cast<size_t>(obj->o_which)));

  if (discardit)
    delete obj;
  return true;
}

void
potion_description(Item const* item, char buf[])
{
  struct obj_info* op = &potion_info.at(static_cast<size_t>(item_subtype(item)));
  if (op->oi_know)
  {
    if (item_count(item) == 1)
      buf += sprintf(buf, "A potion of %s", op->oi_name.c_str());
    else
      buf += sprintf(buf, "%d potions of %s", item_count(item), op->oi_name.c_str());
  }
  else
  {
    string color = *p_colors.at(static_cast<size_t>(item_subtype(item)));

    if (item_count(item) == 1)
      buf += sprintf(buf, "A%s %s potion", vowelstr(color).c_str(), color.c_str());
    else
      buf += sprintf(buf, "%d %s potions", item_count(item), color.c_str());

    if (!op->oi_guess.empty())
      sprintf(buf, " called %s", op->oi_guess.c_str());
  }
}

Item*
potion_create(int which)
{
  Item* pot = new Item();

  if (which == -1)
    which = static_cast<int>(pick_one(potion_info, NPOTIONS));

  pot->o_type       = POTION;
  pot->o_which      = which;
  pot->o_count      = 1;
  pot->o_damage     = {1, 2};
  pot->o_hurldmg    = {1, 2};

  return pot;
}

