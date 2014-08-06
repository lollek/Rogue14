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

#include "rogue.h"
#include "command.h"
#include "io.h"
#include "chase.h"

enum game_mode_t
{
  LOAD_GAME = 0,
  NEW_GAME = 1
};

static bool playit();
static enum game_mode_t parse_args(int argc, char * const *argv);
static void endit(int sig);

int open_score_and_drop_setuid_setgid(); /* src/save.c */
void auto_save(int);                      /* src/save.c */

bool init_new_game();                     /* src/init.c */
bool init_old_game();                     /* src/init.c */

/** main:
 * The main program, of course */
int
main(int argc, char **argv)
{
  bool retval = false;

  /* Open scoreboard and drop setuid/getgid, so we can modify the score later */
  if (open_score_and_drop_setuid_setgid() != 0)
    return 1;

  /* Parse args and then init new (or old) game */
  switch (parse_args(argc, argv))
  {
    case LOAD_GAME: retval = init_old_game();
    when NEW_GAME:  retval = init_new_game();
    otherwise:
      printf("Error: Failed while parsing arguments\n");
      return 1;
  }

  if (retval == false)
    return 1;

  /* Play game! */
  return playit();
}

/** endit:
 * Exit the program abnormally.  */
void
endit(int sig)
{
  (void)sig;
  endwin();
  puts("Okay, bye bye!\n");
  exit(0);
}

/** playit:
 * The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */
bool
playit(void)
{
  /* Try to crash cleanly, and autosave if possible */
  signal(SIGHUP, auto_save);
  signal(SIGQUIT, endit);
  signal(SIGILL, auto_save);
  signal(SIGTRAP, auto_save);
  signal(SIGIOT, auto_save);
  signal(SIGFPE, auto_save);
  signal(SIGBUS, auto_save);
  signal(SIGSEGV, auto_save);
  signal(SIGSYS, auto_save);
  signal(SIGTERM, auto_save);
  signal(SIGINT, quit);

  oldpos = hero;
  oldrp = roomin(&hero);
  while (command() != 1)
    /* Main Loop */;
  endit(0);
  return 0;
}

/** quit:
 * Have player make certain, then exit.
 */
void
quit(int sig)
{
  int oy, ox;
  (void) sig;

  /* Reset the signal in case we got here via an interrupt */
  getyx(curscr, oy, ox);
  msg("");
  msg("really quit? ");

  if (getch() == 'y')
  {
    signal(SIGINT, leave);
    evaluate_players_inventory();
    score(purse, 1, 0);
    exit(0);
  }
  else
  {
    status();
    msg("");
    move(oy, ox);
    refresh();
    stop_counting(true);
  }
}

/** leave:
 * Leave quickly, but curteously
 */
void
leave(int sig)
{
  static char buf[BUFSIZ];
  (void)sig;

  setbuf(stdout, buf);	/* throw away pending output */

  if (!isendwin())
  {
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
  }

  putchar('\n');
  exit(0);
}

/** shell:
 * Let them escape for a while
 */
void
shell(void)
{
  /* Set the terminal back to original mode */
  move(LINES-1, 0);
  refresh();
  endwin();
  putchar('\n');
  fflush(stdout);

  /* Return to shell */
  kill(getpid(), SIGSTOP);

  /* Set the terminal to gaming mode */
  fflush(stdout);
  noecho();
  raw();
  clearok(stdscr, true);
}

/** parse_args
 * Parse command-line arguments
 */
enum game_mode_t
parse_args(int argc, char * const *argv)
{
  enum game_mode_t game_mode = NEW_GAME;
  int option_index = 0;
  struct option long_options[] = {
    {"no-colors", no_argument,       0, 'c'},
    {"escdelay",  optional_argument, 0, 'E'},
    {"flush",     no_argument,       0, 'f'},
    {"hide-floor",no_argument,       0, 'F'},
    {"jump",      no_argument,       0, 'j'},
    {"name",      required_argument, 0, 'n'},
    {"passgo",    no_argument,       0, 'p'},
    {"restore",   no_argument,       0, 'r'},
    {"score",     no_argument,       0, 's'},
    {"seed",      required_argument, 0, 'S'},
    {"terse",     no_argument,       0, 't'},
    {"hide-tomb", no_argument,       0, 'T'},
    {"quick",     no_argument,       0, 'Q'},
    {"wizard",    no_argument,       0, 'W'},
    {"help",      no_argument,       0, '0'},
    {"version",   no_argument,       0, '1'},
    {0,           0,                 0,  0 }
  };

  /* Global default options */
  ESCDELAY = 0;                 /* Set the delay before ESC cancels */
  terse = false;                /* Terse output */
  fight_flush = false;          /* Flush typeahead during battle */
  jump = false;                 /* Show running as a series of jumps */
  see_floor = true;             /* Show the lamp-illuminated floor */
  passgo = false;               /* Follow the turnings in passageways */
  tombstone = true;             /* Print out tombstone when killed */
  use_colors = true;            /* Use ncurses colors */
  game_type = DEFAULT;          /* Play a normal game or rogue */

  /* Default file name for save file */
  strcpy(file_name, get_homedir());
  strncat(file_name, ".rogue14_save", MAXSTR - strlen(get_homedir()) -1);

  /* Set seed and dungeon number */
  seed = time(NULL) + getpid();

  for (;;)
  {
    int c = getopt_long(argc, argv, "cE::fFjn:prsS:tTQW",
                        long_options, &option_index);
    if (c == -1)
      break;

    switch (c)
    {
      case 'c': use_colors = false;
      when 'E': ESCDELAY = optarg == NULL ? 64 : atoi(optarg);
      when 'f': fight_flush = true;
      when 'F': see_floor = false;
      when 'j': jump = true;
      when 'n': if (strlen(optarg))
                  strucpy(whoami, optarg, strlen(optarg));
      when 'p': passgo = true;
      when 'r': game_mode = LOAD_GAME;
      when 's': potential_wizard = true; score(0, -1, 0); exit(0);
      when 'S': seed = atoi(optarg);
      when 't': terse = true;
      when 'T': tombstone = false;
      when 'Q': game_type = QUICK;
      when 'W': potential_wizard = wizard = true;
                player.t_flags |= SEEMONST;
      when '0':
        printf("Usage: %s [OPTIONS] [FILE]\n"
               "Run Rogue14 with selected options or a savefile\n\n"
               "  -c, --no-colors      remove colors from the game\n"
               "  -E, --escdelay=[NUM] set escdelay in ms. Not setting this\n"
               "                       defaults to 0. If you do not give a NUM\n"
               "                       argument, it's set to 64 (old standard)\n"
               "  -f, --flush          flush typeahead during battle\n"
               "  -F, --hide-floor     hide the lamp-illuminated floor\n"
               "  -j, --jump           show running as a series of jumps\n"
               , argv[0]); printf(
               "  -n, --name=NAME      set highscore name\n"
               "  -p, --passgo         Follow the turnings in passageways\n"
               "  -r, --restore        restore game instead of creating a new\n"
               "  -s, --score          display the highscore and exit\n"
               "  -S, --seed=NUMBER    set map seed to NUMBER\n"
               "  -t, --terse          terse output\n"
               "  -T, --hide-tomb      don't print out tombstone when killed\n"
               "  -Q, --quick          Change some rules to make the game take\n"
               "                       less time to play\n"
               ); printf(
               "  -W, --wizard         run the game in debug-mode\n"
               "      --help           display this help and exit\n"
               "      --version        display game version and exit\n\n"
               "%s\n"
               , game_version);
        exit(0);
      when '1': puts(game_version); exit(0);
      otherwise:
        fprintf(stderr, "Try '%s --help' for more information\n",
                argv[0]);
        exit(1);
    }
  }

  /* If we run ./game -r ~/saved_game we should restore it
   * otherwise, we should create a new game with ~/saved_game as file_name */
  if (optind < argc)
  {
    strncpy(file_name, argv[optind], MAXSTR);
    file_name[MAXSTR -1] = '\0';
  }

  return game_mode;
}

