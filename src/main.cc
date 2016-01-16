/*
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 *
 * @(#)main.c 4.22 (Berkeley) 02/05/99
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <inttypes.h>

#include "command.h"
#include "io.h"
#include "pack.h"
#include "init.h"
#include "score.h"
#include "misc.h"
#include "rooms.h"
#include "player.h"
#include "options.h"
#include "os.h"
#include "move.h"
#include "rogue.h"
#include "wizard.h"

/** parse_args
 * Parse command-line arguments
 */
static void
parse_args(int argc, char* const* argv)
{
  int option_index = 0;
  struct option const long_options[] = {
    {"no-colors", no_argument,       0, 'c'},
    {"escdelay",  optional_argument, 0, 'E'},
    {"flush",     no_argument,       0, 'f'},
    {"hide-floor",no_argument,       0, 'F'},
    {"no-jump",   no_argument,       0, 'j'},
    {"name",      required_argument, 0, 'n'},
    {"passgo",    no_argument,       0, 'p'},
    {"restore",   no_argument,       0, 'r'},
    {"score",     no_argument,       0, 's'},
    {"seed",      required_argument, 0, 'S'},
    {"wizard",    no_argument,       0, 'W'},
    {"help",      no_argument,       0, '0'},
    {"version",   no_argument,       0, '1'},
    {0,           0,                 0,  0 }
  };

  /* Global default options */
  ESCDELAY = 0;

  /* Set seed and dungeon number */
  os_rand_seed = static_cast<unsigned>(time(nullptr) + getpid());

  for (;;)
  {
    int c = getopt_long(argc, argv, "cE::fjn:prsS:W",
                        long_options, &option_index);
    if (c == -1)
      break;

    switch (c)
    {
      case 'c': use_colors = false; break;
      case 'E': ESCDELAY = optarg == nullptr ? 64 : atoi(optarg); break;
      case 'f': fight_flush = true; break;
      case 'j': jump = false; break;
      case 'n': if (strlen(optarg))
                  strucpy(whoami, optarg, strlen(optarg));
                break;
      case 'p': passgo = true; break;
      case 's': score_show_and_exit(0, -1, 0); /* does not return */
      case 'S': if (wizard)
                  sscanf(optarg, "%" SCNu32, &os_rand_seed);
                break;
      case 'W': wizard = true; break;
      case '0':
        printf("Usage: %s [OPTIONS] [FILE]\n"
               "Run Rogue14 with selected options or a savefile\n\n"
               "  -c, --no-colors      remove colors from the game\n"
               "  -E, --escdelay=[NUM] set escdelay in ms. Not setting this\n"
               "                       defaults to 0. If you do not give a NUM\n"
               "                       argument, it's set to 64 (old standard)\n"
               "  -f, --flush          flush typeahead during battle\n"
               "  -j, --no-jump        draw each player step separately\n"
               , argv[0]); printf(
               "  -n, --name=NAME      set highscore name\n"
               "  -p, --passgo         follow the turnings in passageways\n"
               "  -r, --restore        restore game instead of creating a new\n"
               "  -s, --score          display the highscore and exit\n"
               ); printf(
               "  -W, --wizard         run the game in debug-mode\n"
               "  -S, --seed=NUMBER    (wizard) set map seed to NUMBER\n"
               "      --help           display this help and exit\n"
               "      --version        display game version and exit\n\n"
               "%s\n"
               , GAME_VERSION);
        exit(0);
      case '1':
        puts(GAME_VERSION);
        exit(0);
      default:
        fprintf(stderr, "Try '%s --help' for more information\n",
                argv[0]);
        exit(1);
    }
  }

  if (optind < argc)
  {
    fprintf(stderr, "Try '%s --help' for more information\n",
            argv[0]);
    exit(1);
  }
}

/** main:
 * The main program, of course */
int
main(int argc, char** argv)
{
  /* Open scoreboard, so we can modify the score later */
  score_open();

  /* Parse args and then init new (or old) game */
  parse_args(argc, argv);
  if (init_new_game() == false)
    return 1;

  /* Try to crash cleanly, and autosave if possible
   * Unless we are debugging, since that messes with gdb */
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
  io_msg("Seed: #%u", os_rand_seed);
#endif

  move_pos_prev = *player_get_pos();
  room_prev = roomin(player_get_pos());

  for (;;) command();

  /* CODE NOT REACHED */;
}

