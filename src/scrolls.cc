#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <string>
#include <vector>

using namespace std;

#include "error_handling.h"
#include "game.h"
#include "colors.h"
#include "coordinate.h"
#include "io.h"
#include "item.h"
#include "level.h"
#include "misc.h"
#include "monster.h"
#include "options.h"
#include "os.h"
#include "pack.h"
#include "player.h"
#include "potions.h"
#include "rings.h"
#include "rogue.h"
#include "things.h"
#include "wand.h"
#include "weapons.h"

#include "scrolls.h"

static vector<string> s_names(NSCROLLS);

vector<obj_info> scroll_info = {
    { "monster confusion",		 7, 140, "", false },
    { "magic mapping",			 4, 150, "", false },
    { "hold monster",			 2, 180, "", false },
    { "sleep",				 3,   5, "", false },
    { "enchant armor",			 7, 160, "", false },
    { "identify",			43, 115, "", false },
    { "scare monster",			 3, 200, "", false },
    { "food detection",			 2,  60, "", false },
    { "teleportation",			 5, 165, "", false },
    { "enchant weapon",			 8, 150, "", false },
    { "create monster",			 4,  75, "", false },
    { "remove curse",			 7, 105, "", false },
    { "aggravate monsters",		 3,  20, "", false },
    { "protect armor",			 2, 250, "", false },
};

void
scroll_init(void)
{
  char const* sylls[] = {
    "a", "ab", "ag", "aks", "ala", "an", "app", "arg", "arze", "ash",
    "bek", "bie", "bit", "bjor", "blu", "bot", "bu", "byt", "comp",
    "con", "cos", "cre", "dalf", "dan", "den", "do", "e", "eep", "el",
    "eng", "er", "ere", "erk", "esh", "evs", "fa", "fid", "fri", "fu",
    "gan", "gar", "glen", "gop", "gre", "ha", "hyd", "i", "ing", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur", "nej",
    "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od", "ood",
    "org", "orn", "ox", "oxy", "pay", "ple", "plu", "po", "pot",
    "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol", "sa",
    "san", "sat", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
    "sno", "so", "sol", "sri", "sta", "sun", "ta", "tab", "tem",
    "ther", "ti", "tox", "trol", "tue", "turs", "u", "ulk", "um", "un",
    "uni", "ur", "val", "viv", "vly", "vom", "wah", "wed", "werg",
    "wex", "whon", "wun", "xo", "y", "yot", "yu", "zant", "zeb", "zim",
    "zok", "zon", "zum",
  };

  int const MAXNAME = 40;
  for (size_t i = 0; i < NSCROLLS; i++)
  {
    char tmpbuf[MAXSTR*2];
    char* cp = tmpbuf;
    int nwords;

    for (nwords = os_rand_range(3)+2; nwords > 0; nwords--)
    {
      int nsyl = os_rand_range(3) + 1;

      while (nsyl--)
      {
        char const* sp = sylls[os_rand_range((sizeof(sylls)) / (sizeof(*sylls)))];
        if (&cp[strlen(sp)] > &tmpbuf[MAXNAME])
          break;
        while (*sp)
          *cp++ = *sp++;
      }

      *cp++ = ' ';
    }

    *--cp = '\0';
    s_names.at(i) = tmpbuf;
  }
}

void scroll_learn(enum scroll_t scroll)
{
  scroll_info.at(scroll).oi_know = true;
}

bool
scroll_is_known(enum scroll_t scroll)
{
  return scroll_info.at(scroll).oi_know;
}

size_t
scroll_value(enum scroll_t scroll)
{
  return scroll_info.at(scroll).oi_worth;
}

void scroll_set_name(enum scroll_t scroll, string const& new_name)
{
  scroll_info.at(scroll).oi_guess = new_name;
}

static bool
enchant_players_armor(void)
{
  Item* arm = pack_equipped_item(EQUIPMENT_ARMOR);
  if (arm == nullptr)
  {
    switch (os_rand_range(3))
    {
      case 0: io_msg("you are unsure if anything happened"); break;
      case 1: io_msg("you feel naked"); break;
      case 2: io_msg("you feel like something just touched you"); break;
    }
    return false;
  }

  arm->o_arm--;
  arm->o_flags &= ~ISCURSED;
  io_msg("your armor glows %s for a moment",
          player->is_hallucinating() ? color_random().c_str() : "silver");
  return true;
}

/* Stop all monsters within two spaces from chasing after the hero. */
static bool
hold_monsters(void)
{
  Coordinate const& player_pos = player->get_position();
  int monsters_affected = 0;
  Monster* held_monster = nullptr;

  for (int x = player_pos.x - 2; x <= player_pos.x + 2; x++)
    if (x >= 0 && x < NUMCOLS)
      for (int y = player_pos.y - 2; y <= player_pos.y + 2; y++)
        if (y >= 0 && y <= NUMLINES - 1)
        {
          Monster *monster = Game::level->get_monster(x, y);
          if (monster != nullptr)
          {
            monster->set_held();
            monsters_affected++;
            held_monster = monster;
          }
        }

  if (monsters_affected == 1)
  {
    io_msg("%s freezes", held_monster->get_name().c_str());
  }
  else if (monsters_affected > 1)
    io_msg("the monsters around you freeze");
  else /* monsters_affected == 0 */
    switch (os_rand_range(3))
    {
      case 0: io_msg("you are unsure if anything happened"); break;
      case 1: io_msg("you feel a strange sense of loss"); break;
      case 2: io_msg("you feel a powerful aura"); break;
    }

  return monsters_affected;
}

static bool
create_monster(void)
{
  Coordinate const& player_pos = player->get_position();
  Coordinate mp;
  int i = 0;

  for (int y = player_pos.y - 1; y <= player_pos.y + 1; y++)
    for (int x = player_pos.x - 1; x <= player_pos.x + 1; x++)
    {
      char ch = Game::level->get_type(x, y);

      /* No duplicates */
      if ((y == player_pos.y && x == player_pos.x)) {
        continue;
      }

      /* Cannot stand there */
      if (!step_ok(ch)) {
        continue;
      }

      /* Monsters cannot stand of scroll of stand monster */
      if (ch == SCROLL) {
        Item *item = Game::level->get_item(y, x);
        if (item == nullptr) {
          error("Should be an item here");
        }

        if (item->o_which == S_SCARE) {
          continue;
        }
      }

      /* RNGsus doesn't want a monster here */
      if (os_rand_range(++i) != 0) {
        continue;
      }

      mp.y = y;
      mp.x = x;
    }

  if (i == 0)
    switch (os_rand_range(3))
    {
      case 0: io_msg("you are unsure if anything happened"); break;
      case 1: io_msg("you hear a faint cry of anguish in the distance"); break;
      case 2: io_msg("you think you felt someone's presence"); break;
    }
  else
  {
    Monster *monster = new Monster(Monster::random_monster_type(), mp, Game::level->get_room(mp));
    Game::level->monsters.push_back(monster);
    Game::level->set_monster(mp, monster);
    io_msg("A %s appears out of thin air", monster->get_name().c_str());
  }

  return i;
}

static bool
food_detection(void)
{
  bool food_seen = false;
  wclear(hw);

  for (Item const* obj : Game::level->items) {
    if (obj->o_type == FOOD)
    {
      food_seen = true;
      mvwaddcch(hw, obj->get_y(), obj->get_x(), FOOD);
    }
  }

  if (food_seen)
    show_win("Your nose tingles and you smell food.--More--");
  else
    io_msg("your nose tingles");

  return food_seen;
}

static bool
player_enchant_weapon(void)
{
  Item* weapon = pack_equipped_item(EQUIPMENT_RHAND);
  if (weapon == nullptr)
  {
    switch (os_rand_range(2))
    {
      case 0: io_msg("you feel a strange sense of loss"); break;
      case 1: io_msg("you are unsure if anything happened"); break;
    }
    return false;
  }

  weapon->o_flags &= ~ISCURSED;
  if (os_rand_range(2) == 0)
    weapon->o_hplus++;
  else
    weapon->o_dplus++;
  io_msg("your %s glows %s for a moment",
         weapon_info[static_cast<size_t>(weapon->o_which)].oi_name.c_str(),
         player->is_hallucinating() ? color_random().c_str() : "blue");

  return true;
}

static void
remove_curse(void)
{
  for (int i = 0; i < NEQUIPMENT; ++i)
    if (pack_equipped_item(static_cast<equipment_pos>(i)) != nullptr)
      pack_uncurse_item(pack_equipped_item(static_cast<equipment_pos>(i)));

  io_msg(player->is_hallucinating()
      ? "you feel in touch with the Universal Onenes"
      : "you feel as if somebody is watching over you");
}

static bool
protect_armor(void)
{
  Item* arm = pack_equipped_item(EQUIPMENT_ARMOR);
  if (arm == nullptr)
  {
    switch (os_rand_range(2))
    {
      case 0: io_msg("you feel a strange sense of loss"); break;
      case 1: io_msg("you are unsure if anything happened"); break;
    }
    return false;
  }

  arm->o_flags |= ISPROT;
  io_msg("your armor is covered by a shimmering %s shield",
          player->is_hallucinating() ? color_random().c_str() : "gold");
  return true;
}

bool
scroll_read(void)
{
  Item* obj = pack_get_item("read", SCROLL);
  if (obj == nullptr)
    return false;

  if (obj->o_type != SCROLL)
  {
    io_msg("there is nothing on it to read");
    return false;
  }

  /* Get rid of the thing */
  bool discardit = obj->o_count == 1;
  pack_remove(obj, false, false);
  Item* orig_obj = obj;

  switch (obj->o_which)
  {
    case S_CONFUSE:
      player->set_confusing_attack();
      break;
    case S_ARMOR:
      if (enchant_players_armor())
        scroll_learn(S_ARMOR);
      break;
    case S_HOLD:
      if (hold_monsters())
        scroll_learn(S_HOLD);
      break;
    case S_SLEEP:
      scroll_learn(S_SLEEP);
      player->fall_asleep();
      break;
    case S_CREATE:
      if (create_monster())
        scroll_learn(S_CREATE);
      break;
    case S_ID:
      if (!scroll_is_known(S_ID))
        io_msg("this scroll is an %s scroll", scroll_info.at(static_cast<size_t>(obj->o_which)).oi_name.c_str());
      scroll_learn(S_ID);
      pack_identify_item();
      break;
    case S_MAP:
      scroll_learn(S_MAP);
      io_msg("this scroll has a map on it");
      Game::io->print_level_layout();
      break;
    case S_FDET:
      if (food_detection())
        scroll_learn(S_FDET);
      break;
    case S_TELEP:
      scroll_learn(S_TELEP);
      player->teleport(nullptr);
      break;
    case S_ENCH:
      player_enchant_weapon();
      break;
    case S_SCARE:
      /* Reading it is a mistake and produces laughter at her poor boo boo. */
      io_msg("you hear maniacal laughter in the distance");
      break;
    case S_REMOVE:
      remove_curse();
      break;
    case S_AGGR:
      /* This scroll aggravates all the monsters on the current
       * level and sets them running towards the hero */
      monster_aggravate_all();
      io_msg("you hear a high pitched humming noise");
      break;
    case S_PROTECT:
      protect_armor();
      break;
    default:
      io_msg("what a puzzling scroll!");
      return true;
  }
  obj = orig_obj;

  call_it("scroll", &scroll_info[static_cast<size_t>(obj->o_which)]);

  if (discardit)
    delete obj;

  return true;
}

void
scroll_description(Item const* item, char* buf)
{
  struct obj_info* op = &scroll_info[static_cast<size_t>(item_subtype(item))];
  char* ptr = buf;

  if (item_count(item) == 1)
    ptr += sprintf(ptr, "A scroll");
  else
    ptr += sprintf(ptr, "%d scrolls", item_count(item));

  if (op->oi_know)
    ptr += sprintf(ptr, " of %s", op->oi_name.c_str());
  else if (!op->oi_guess.empty())
    ptr += sprintf(ptr, " called %s", op->oi_guess.c_str());
  else
    ptr += sprintf(ptr, " titled '%s'", s_names.at(static_cast<size_t>(item_subtype(item))).c_str());
}

Item*
scroll_create(int which)
{
  Item* scroll = new Item();

  if (which == -1)
    which = static_cast<int>(pick_one(scroll_info, NSCROLLS));

  scroll->o_type  = SCROLL;
  scroll->o_count = 1;
  scroll->o_which = which;
  return scroll;
}

