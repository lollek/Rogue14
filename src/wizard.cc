#include <vector>

#include "food.h"
#include "gold.h"
#include "error_handling.h"
#include "game.h"
#include "coordinate.h"
#include "potions.h"
#include "scrolls.h"
#include "command.h"
#include "options.h"
#include "io.h"
#include "armor.h"
#include "pack.h"
#include "rings.h"
#include "misc.h"
#include "level.h"
#include "weapons.h"
#include "player.h"
#include "wand.h"
#include "traps.h"
#include "os.h"
#include "rogue.h"

#include "wizard.h"

using namespace std;

bool wizard = false;

static void
pr_spec(char type)
{
  WINDOW* printscr = dupwin(stdscr);

  Coordinate orig_pos;
  getyx(stdscr, orig_pos.y, orig_pos.x);

  size_t max;
  switch (type)
  {
    case IO::Potion: max = Potion::Type::NPOTIONS;  break;
    case IO::Scroll: max = Scroll::Type::NSCROLLS;  break;
    case IO::Ring:   max = Ring::Type::NRINGS;    break;
    case IO::Wand:   max = Wand::Type::NWANDS; break;
    case IO::Armor:  max = Armor::Type::NARMORS;   break;
    case IO::Weapon: max = Weapon::NWEAPONS; break;
    default: error("Unknown type in pr_spec");
  }

  char ch = '0';
  for (size_t i = 0; i < max; ++i)
  {
    string name;
    int prob;
    wmove(printscr, static_cast<int>(i) + 1, 1);

    switch (type) {
      case IO::Scroll: {
        name = Scroll::name(static_cast<Scroll::Type>(i));
        prob = Scroll::probability(static_cast<Scroll::Type>(i));
      } break;

      case IO::Armor: {
        name = Armor::name(static_cast<Armor::Type>(i));
        prob = Armor::probability(static_cast<Armor::Type>(i));
      } break;

      case IO::Potion: {
        name = Potion::name(static_cast<Potion::Type>(i));
        prob = Potion::probability(static_cast<Potion::Type>(i));
      } break;

      case IO::Wand: {
        name = Wand::name(static_cast<Wand::Type>(i));
        prob = Wand::probability(static_cast<Wand::Type>(i));
      } break;

      case IO::Ring: {
        name = Ring::name(static_cast<Ring::Type>(i));
        prob = Ring::probability(static_cast<Ring::Type>(i));
      } break;

      case IO::Weapon: {
        name = Weapon::name(static_cast<Weapon::Type>(i));
        prob = Weapon::probability(static_cast<Weapon::Type>(i));
      } break;

      default: error("Unknown type in pr_spec");
    }

    wprintw(printscr, "%c: %s (%d%%)", ch, name.c_str(), prob);
    ch = ch == '9' ? 'a' : (ch + 1);
  }

  wmove(stdscr, orig_pos.y, orig_pos.x);
  wrefresh(printscr);
  delwin(printscr);
}

static void
print_things(void)
{
  char index_to_char[] = { IO::Potion, IO::Scroll, IO::Food, IO::Weapon, IO::Armor,
                           IO::Ring, IO::Wand };
  WINDOW* tmp = dupwin(stdscr);

  Coordinate orig_pos;
  getyx(stdscr, orig_pos.y, orig_pos.x);

  for (int i = 0; i < static_cast<int>(Item::NITEMS); ++i)
  {
    wmove(tmp, i + 1, 1);
    wprintw(tmp, "%c %s", index_to_char[i], Item::name(static_cast<Item::Type>(i)).c_str());
  }

  wmove(stdscr, orig_pos.y, orig_pos.x);
  wrefresh(tmp);
  delwin(tmp);
}

int
wizard_list_items(void)
{
  Game::io->message("for what type of object do you want a list?");
  print_things();

  int ch = io_readchar(true);
  touchwin(stdscr);
  refresh();

  pr_spec(static_cast<char>(ch));
  Game::io->clear_message();
  Game::io->message("--Press any key to continue--");
  io_readchar(false);

  Game::io->clear_message();
  touchwin(stdscr);
  return 0;
}

void wizard_create_item(void) {

  // Get itemtype
  Game::io->message("type of item?");
  int type = io_readchar(true);
  Game::io->clear_message();

  switch (type) {

    // Supported types:
    case IO::Weapon: case IO::Armor: case IO::Ring: case IO::Wand:
    case IO::Gold: case IO::Potion: case IO::Scroll: case IO::Trap: case IO::Food: {
      break;
    }

    default: {
      Game::io->message("Bad pick");
      return;
    }
  }

  // Get item subtype
  Game::io->message("which " + string(1, type) + " do you want? (0-f)");
  char ch = io_readchar(true);
  int which = isdigit(ch) ? ch - '0' : ch - 'a' + 10;
  Game::io->clear_message();

  // Allocate item
  Item* obj = nullptr;
  switch (type) {
    case IO::Trap: {
      if (which < 0 || which >= Trap::NTRAPS) {
        Game::io->message("Bad trap id");

      } else {
        Game::level->set_not_real(player->get_position());
        Game::level->set_trap_type(player->get_position(), static_cast<Trap::Type>(which));
      }
    } return;

    case IO::Wand: {
      obj = new Wand(static_cast<Wand::Type>(which));
    } break;

    case IO::Scroll: {
      obj = new Scroll(static_cast<Scroll::Type>(which));
      obj->o_count = 10;
    } break;

    case IO::Potion: {
      obj = new Potion(static_cast<Potion::Type>(which));
      obj->o_count = 10;
    } break;

    case IO::Food: {
      obj = new Food(static_cast<Food::Type>(which));
    } break;

    case IO::Weapon: {
      obj = new Weapon(static_cast<Weapon::Type>(which), false);

      Game::io->message("blessing? (+,-,n)");
      char bless = io_readchar(true);
      Game::io->clear_message();

      if (bless == '-') {
        obj->set_cursed();
        obj->modify_hit_plus(-os_rand_range(3) + 1);

      } else if (bless == '+')
        obj->modify_hit_plus(os_rand_range(3) + 1);
    } break;

    case IO::Armor: {
      obj = new Armor(false);

      Game::io->message("blessing? (+,-,n)");
      char bless = io_readchar(true);
      Game::io->clear_message();
      if (bless == '-') {
        obj->set_cursed();
        obj->modify_armor(os_rand_range(3) + 1);

      } else if (bless == '+')
        obj->modify_armor(-os_rand_range(3) + 1);
    } break;

    case IO::Ring: {
      obj = new Ring(static_cast<Ring::Type>(which), false);

      switch (obj->o_which) {
        case Ring::Type::PROTECT: case Ring::Type::ADDSTR:
        case Ring::Type::ADDHIT:  case Ring::Type::ADDDAM: {
          Game::io->message("blessing? (+,-,n)");
          char bless = io_readchar(true);
          Game::io->clear_message();
          if (bless == '-')
            obj->set_cursed();
          obj->set_armor(bless == '-' ? -1 : os_rand_range(2) + 1);
        } break;
      }
    } break;

    case IO::Gold: {
      Game::io->message("how much?");
      int amount = stoi(Game::io->read_string());
      obj = new Gold(amount);
    } break;

    default: {
      error("Unimplemented item: " + to_string(which));
    }
  }

  if (obj == nullptr) {
    error("object was null");
  }
  pack_add(obj, false);
}

void wizard_show_map(void) {
  wclear(Game::io->extra_screen);

  for (int y = 1; y < NUMLINES - 1; y++)  {
    for (int x = 0; x < NUMCOLS; x++) {
      Tile::Type tile = Game::level->get_tile(x, y);
      chtype ch;
      switch (tile) {
        case Tile::Shadow: ch = IO::Shadow; break;
        case Tile::Floor:  ch = IO::Floor; break;
        case Tile::Wall:   ch = IO::Wall; break;
        case Tile::Door:   ch = IO::Door; break;
        case Tile::Trap:   ch = IO::Trap; break;
        case Tile::Stairs: ch = IO::Stairs; break;
      }

      if (!Game::level->is_real(x, y)) {
        ch |= A_STANDOUT;
      }

      mvwaddcch(Game::io->extra_screen, y, x, ch);
    }
  }
  Game::io->show_extra_screen("---More (level map)---");
}

void wizard_levels_and_gear(void) {
  player->raise_level(9);

  /* Give him a sword (+1,+1) */
  if (pack_equipped_item(EQUIPMENT_RHAND) == nullptr) {
    Weapon* weapon = new Weapon(Weapon::TWOSWORD, false);
    weapon->set_hit_plus(1);
    weapon->set_damage_plus(1);
    weapon->set_identified();
    pack_equip_item(weapon);
  }

  /* And his suit of armor */
  if (pack_equipped_item(EQUIPMENT_ARMOR) == nullptr) {
    Armor* armor = new Armor(Armor::Type::PLATE_MAIL, false);
    armor->set_armor(-5);
    armor->set_identified();
    pack_equip_item(armor);
  }
}
