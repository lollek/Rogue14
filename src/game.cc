#include <string>
#include <iostream>
#include <fstream>

#include "command.h"
#include "error_handling.h"
#include "os.h"
#include "potions.h"
#include "scrolls.h"
#include "io.h"
#include "armor.h"
#include "daemons.h"
#include "colors.h"
#include "level.h"
#include "rings.h"
#include "misc.h"
#include "player.h"
#include "weapons.h"
#include "wand.h"
#include "options.h"
#include "rogue.h"
#include "traps.h"

#include "Game.h"

using namespace std;

Game*   Game::game_ptr = nullptr;
IO*     Game::io = nullptr;
Level*  Game::level = nullptr;
string* Game::whoami = nullptr;
int     Game::current_level = 1;
int     Game::levels_without_food = 0;
int     Game::max_level_visited = 1;

void Game::exit() {
  if (game_ptr != nullptr) {
    delete game_ptr;
  }
  ::exit(0);
}

void Game::new_level(int dungeon_level) {

  /* Set max level we've been to */
  Game::current_level = dungeon_level;
  if (Game::current_level > Game::max_level_visited) {
    Game::max_level_visited = Game::current_level;
  }

  if (Game::level != nullptr) {
    delete Game::level;
  }

  Game::level = new Level();

  // Drop player in the new dungeon level
  if (player == nullptr) {
    player = new Player();
  }

  Coordinate new_player_pos = player->get_position();
  Game::level->get_random_room_coord(nullptr, &new_player_pos, 0, true);
  player->set_position(new_player_pos);
  player->set_room(Game::level->get_room(new_player_pos));
  Game::io->print_color(new_player_pos.x, new_player_pos.y, player->get_type());

  // Unhold player just in case
  player->set_not_held();
}

int Game::run() {

  // Try to crash cleanly, and autosave if possible
  // Unless we are debugging, since that messes with gdb/lldb
#ifdef NDEBUG
  signal(SIGHUP, save_auto);
  signal(SIGQUIT, command_signal_endit);
  signal(SIGILL, save_auto);
  signal(SIGTRAP, save_auto);
  signal(SIGIOT, save_auto);
  signal(SIGFPE, save_auto);
  signal(SIGBUS, save_auto);
  signal(SIGSEGV, save_auto);
  signal(SIGSYS, save_auto);
  signal(SIGTERM, save_auto);
  signal(SIGINT, command_signal_quit);
#else
  Game::io->message("Seed: #" + to_string(starting_seed));
#endif

  player->set_previous_room(Game::level->get_room(player->get_position()));

#ifdef NDEBUG
  try {
    for (;;) command();
  } catch (const std::runtime_error &ex) {
    endwin();
    cout << ex.what() << endl;
    return 1;
  }
#else
  for (;;) command();
#endif

  // CODE NOT REACHED
}

Game::Game(string const& whoami_)
  : starting_seed(os_rand_seed) {
    Game::whoami = new string(whoami_);

  if (game_ptr != nullptr) {
    error("Game is a singleton class");
  }
  game_ptr = this;

  cout << "Hello " << *whoami << ", just a moment while I dig the dungeon..." << flush;

  // Init stuff
  Game::io = new IO();                  // Graphics
  Scroll::init_scrolls();               // Names of scrolls
  Color::init_colors();                 // Colors for potions and stuff
  Potion::init_potions();               // Colors of potions
  Ring::init_rings();                   // Stone settings of rings
  Wand::init_wands();                   // Materials of wands
  Daemons::init_daemons();              // Over-time-effects
  Monster::init_monsters();             // Monster types
  Trap::init_traps();                   // Trap types
  Game::new_level(Game::current_level); // Level (and player)

  // Start up daemons and fuses
  Daemons::daemon_start(Daemons::runners_move, AFTER);
  Daemons::daemon_start(Daemons::doctor, AFTER);
  Daemons::daemon_start(Daemons::ring_abilities, AFTER);
}

Game::~Game() {
  Trap::free_traps();
  Monster::free_monsters();
  Daemons::free_daemons();
  Wand::free_wands();
  Ring::free_rings();
  Potion::free_potions();
  Color::free_colors();
  Scroll::free_scrolls();

  delete Game::io;
  Game::io = nullptr;

  delete Game::whoami;
  Game::whoami = nullptr;

  cerr << "\nOK\n";
}

Game::Game(ifstream& savefile) {
  Game::io = new IO();

  Scroll::load_scrolls(savefile);
#if 0
  Scroll::init_scrolls();
  Color::init_colors();
  Potion::init_potions();
  Ring::init_rings();
  Wand::init_wands();
  Daemons::init_daemons();
  Monster::init_monsters();
  Trap::init_traps();
  Game::new_level(Game::current_level);
#endif
}

void Game::save(std::ofstream& savefile) {
  Scroll::save_scrolls(savefile);
#if 0
#endif
}
