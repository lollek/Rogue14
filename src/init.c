/*
 * global variable initializaton
 *
 * @(#)init.c	4.31 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "rogue.h"
#include "potions.h"
#include "status_effects.h"
#include "scrolls.h"
#include "io.h"
#include "chase.h"
#include "armor.h"
#include "pack.h"


static void sumprobs(char ch);

/* init_new_game
 * Set up everything so we can start playing already */
bool
init_new_game(void)
{
  /* Parse environment opts */
  if (whoami[0] == '\0')
  {
    const struct passwd *pw = getpwuid(getuid());
    const char *name = pw ? pw->pw_name : "nobody";

    strucpy(whoami, name, strlen(name));
  }

  printf("Hello %s, just a moment while I dig the dungeon...", whoami);
  fflush(stdout);

  /* Init Graphics */
  if (init_graphics() != 0)
    return false;
  idlok(stdscr, true);
  idlok(hw, true);

  /* Init stuff */
  init_probs();                         /* Set up prob tables for objects */
  init_player();                        /* Set up initial player stats */
  init_names();                         /* Set up names of scrolls */
  init_colors();                        /* Set up colors of potions */
  init_stones();                        /* Set up stone settings of rings */
  init_materials();                     /* Set up materials of wands */

  new_level();                          /* Draw current level */

  /* Start up daemons and fuses */
  start_daemon(runners, 0, AFTER);
  start_daemon(doctor, 0, AFTER);
  fuse(swander, 0, WANDERTIME, AFTER);
  start_daemon(stomach, 0, AFTER);

  return true;
}

/** init_old_game:
 * Restore a saved game from a file with elaborate checks for file
 * integrity from cheaters */
bool
init_old_game(void)
{
    FILE *inf = fopen(file_name, "r");
    char buf[MAXSTR];

    if (inf == NULL)
    {
        perror(file_name);
        return false;
    }

    fflush(stdout);
    encread(buf, strlen(game_version) + 1, inf);
    if (strcmp(buf, game_version))
    {
      printf("Sorry, saved game is out of date.\n");
      return false;
    }

    encread(buf, 80, inf);

    if (init_graphics() != 0)
      return false;

    if (rs_restore_file(inf) != 0)
    {
      endwin();
      printf(": Corrupted save game\n");
      return false;
    }
    /*
     * we do not close the file so that we will have a hold of the
     * inode for as long as possible
     */

    if (unlink(file_name) < 0)
    {
        endwin();
        printf("Cannot unlink file\n");
        return false;
    }
    mpos = 0;
    clearok(stdscr,true);

    if (pstats.s_hpt <= 0)
    {
        endwin();
        printf("\n\"He's dead, Jim\"\n");
        return false;
    }

    clearok(curscr, true);
    msg("file name: %s", file_name);
    return true;
}

/*
 * init_graphics:
 * 	get curses running
 */
int
init_graphics(void)
{
  initscr();  /* Start up cursor package */

  /* Ncurses colors */
  if (use_colors)
  {
    if (start_color() == ERR)
    {
      endwin();
      fprintf(stderr, "Error: Failed to start colors. "
                      "Try restarting without colors enabled\n");
      return 1;
    }

    /* Because ncurses has defined COLOR_BLACK to 0 and COLOR_WHITE to 7,
     * and then decided that init_pair cannot change number 0 (COLOR_BLACK)
     * I use COLOR_WHITE for black text and COLOR_BLACK for white text */

    assume_default_colors(0, -1); /* Default is white text and any background */
    init_pair(COLOR_RED, COLOR_RED, -1);
    init_pair(COLOR_GREEN, COLOR_GREEN, -1);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, -1);
    init_pair(COLOR_BLUE, COLOR_BLUE, -1);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, -1);
    init_pair(COLOR_CYAN, COLOR_CYAN, -1);
    init_pair(COLOR_WHITE, COLOR_BLACK, -1);
  }

  if (LINES < NUMLINES || COLS < NUMCOLS)
  {
    endwin();
    printf("\nSorry, the screen must be at least %dx%d\n", NUMLINES, NUMCOLS);
    return 1;
  }

  raw();     /* Raw mode */
  noecho();  /* Echo off */
  hw = newwin(LINES, COLS, 0, 0);

  return 0;
}

/*
 * init_player:
 *	Roll her up
 */
void
init_player(void)
{
    THING *obj;

    pstats = max_stats;
    food_left = HUNGERTIME;

    /* Give him some food */
    obj = new_item();
    obj->o_type = FOOD;
    obj->o_count = 1;
    add_pack(obj, true);

    /* And his suit of armor */
    obj = new_item();
    obj->o_type = ARMOR;
    obj->o_which = RING_MAIL;
    obj->o_arm = armors[RING_MAIL].ac - 1;
    obj->o_flags |= ISKNOW;
    obj->o_count = 1;
    equip_item(obj);

    /* Give him his weaponry.  First a mace. */
    obj = new_item();
    init_weapon(obj, MACE);
    obj->o_hplus = 1;
    obj->o_dplus = 1;
    obj->o_flags |= ISKNOW;
    equip_item(obj);

    /* Now a +1 bow */
    obj = new_item();
    init_weapon(obj, BOW);
    obj->o_hplus = 1;
    obj->o_flags |= ISKNOW;
    add_pack(obj, true);

    /* Now some arrows */
    obj = new_item();
    init_weapon(obj, ARROW);
    obj->o_count = rnd(15) + 25;
    obj->o_flags |= ISKNOW;
    add_pack(obj, true);
}

/*
 * Contains defintions and functions for dealing with things like
 * potions and scrolls
 */

char *rainbow[] = {
    "amber",
    "aquamarine",
    "black",
    "blue",
    "brown",
    "clear",
    "crimson",
    "cyan",
    "ecru",
    "gold",
    "green",
    "grey",
    "magenta",
    "orange",
    "pink",
    "plaid",
    "purple",
    "red",
    "silver",
    "tan",
    "tangerine",
    "topaz",
    "turquoise",
    "vermilion",
    "violet",
    "white",
    "yellow",
};

#define NCOLORS (sizeof rainbow / sizeof (const char *))
int cNCOLORS = NCOLORS;

static char *sylls[] = {
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

STONE stones[] = {
    { "agate",		 25},
    { "alexandrite",	 40},
    { "amethyst",	 50},
    { "carnelian",	 40},
    { "diamond",	300},
    { "emerald",	300},
    { "germanium",	225},
    { "granite",	  5},
    { "garnet",		 50},
    { "jade",		150},
    { "kryptonite",	300},
    { "lapis lazuli",	 50},
    { "moonstone",	 50},
    { "obsidian",	 15},
    { "onyx",		 60},
    { "opal",		200},
    { "pearl",		220},
    { "peridot",	 63},
    { "ruby",		350},
    { "sapphire",	285},
    { "stibotantalite",	200},
    { "tiger eye",	 50},
    { "topaz",		 60},
    { "turquoise",	 70},
    { "taaffeite",	300},
    { "zircon",	 	 80},
};

#define NSTONES (sizeof stones / sizeof (STONE))
int cNSTONES = NSTONES;

char *wood[] = {
    "avocado wood",
    "balsa",
    "bamboo",
    "banyan",
    "birch",
    "cedar",
    "cherry",
    "cinnibar",
    "cypress",
    "dogwood",
    "driftwood",
    "ebony",
    "elm",
    "eucalyptus",
    "fall",
    "hemlock",
    "holly",
    "ironwood",
    "kukui wood",
    "mahogany",
    "manzanita",
    "maple",
    "oaken",
    "persimmon wood",
    "pecan",
    "pine",
    "poplar",
    "redwood",
    "rosewood",
    "spruce",
    "teak",
    "walnut",
    "zebrawood",
};

#define NWOOD (sizeof wood / sizeof (char *))
int cNWOOD = NWOOD;

char *metal[] = {
    "aluminum",
    "beryllium",
    "bone",
    "brass",
    "bronze",
    "copper",
    "electrum",
    "gold",
    "iron",
    "lead",
    "magnesium",
    "mercury",
    "nickel",
    "pewter",
    "platinum",
    "steel",
    "silver",
    "silicon",
    "tin",
    "titanium",
    "tungsten",
    "zinc",
};

#define NMETAL (sizeof metal / sizeof (char *))
int cNMETAL = NMETAL;
#define MAX3(a,b,c)	(a > b ? (a > c ? a : c) : (b > c ? b : c))

static bool used[MAX3(NCOLORS, NSTONES, NWOOD)];

/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */
void
init_colors(void)
{
    size_t i, j;

    for (i = 0; i < NCOLORS; i++)
	used[i] = false;
    for (i = 0; i < NPOTIONS; i++)
    {
	do
	    j = rnd(NCOLORS);
	while (used[j]);
	used[j] = true;
	p_colors[i] = rainbow[j];
    }
}

/*
 * init_names:
 *	Generate the names of the various scrolls
 */
#define MAXNAME	40	/* Max number of characters in a name */

void
init_names(void)
{
    int nsyl;
    char *cp, *sp;
    int i, nwords;
    char tmpbuf[MAXSTR*2];

    for (i = 0; i < NSCROLLS; i++)
    {
	cp = tmpbuf;
	nwords = rnd(3) + 2;
	while (nwords--)
	{
	    nsyl = rnd(3) + 1;
	    while (nsyl--)
	    {
		sp = sylls[rnd((sizeof sylls) / (sizeof (char *)))];
		if (&cp[strlen(sp)] > &tmpbuf[MAXNAME])
			break;
		while (*sp)
		    *cp++ = *sp++;
	    }
	    *cp++ = ' ';
	}
	*--cp = '\0';
	s_names[i] = (char *) malloc((unsigned) strlen(tmpbuf)+1);
	strcpy(s_names[i], tmpbuf);
    }
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */
void
init_stones(void)
{
    size_t i, j;

    for (i = 0; i < NSTONES; i++)
	used[i] = false;
    for (i = 0; i < MAXRINGS; i++)
    {
	do
	    j = rnd(NSTONES);
	while (used[j]);
	used[j] = true;
	r_stones[i] = stones[j].st_name;
	ring_info[i].oi_worth += stones[j].st_value;
    }
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */
void
init_materials(void)
{
    size_t i, j;
    char *str;
    static bool metused[NMETAL];

    for (i = 0; i < NWOOD; i++)
	used[i] = false;
    for (i = 0; i < NMETAL; i++)
	metused[i] = false;
    for (i = 0; i < MAXSTICKS; i++)
    {
	for (;;)
	    if (rnd(2) == 0)
	    {
		j = rnd(NMETAL);
		if (!metused[j])
		{
		    ws_type[i] = "wand";
		    str = metal[j];
		    metused[j] = true;
		    break;
		}
	    }
	    else
	    {
		j = rnd(NWOOD);
		if (!used[j])
		{
		    ws_type[i] = "staff";
		    str = wood[j];
		    used[j] = true;
		    break;
		}
	    }
	ws_made[i] = str;
    }
}

/*
 * sumprobs:
 *	Sum up the probabilities for items appearing
 */
static void
sumprobs(char ch)
{
  int lastprob = 0;
  const char *str;
  void *ptr;
  int i;
  int max;

  /* Ready the pointers */
  switch (ch)
  {
    case '0':    ptr = things;    max = NUMTHINGS;  str = "things";
    when POTION: ptr = pot_info;  max = NPOTIONS;   str = "potions";
    when SCROLL: ptr = scr_info;  max = NSCROLLS;   str = "scrolls";
    when RING:   ptr = ring_info; max = MAXRINGS;   str = "rings";
    when STICK:  ptr = ws_info;   max = MAXSTICKS;  str = "sticks";
    when WEAPON: ptr = weap_info; max = MAXWEAPONS; str = "weapons";
    when ARMOR:  ptr = armors;    max = NARMORS;    str = "armor";
    otherwise:   ptr = NULL;      max = 0;          str = "error";
  }

  /* Add upp percentage */
  for (i = 0; i < max; ++i)
  {
    int *prob;
    if (ptr == armors)
      prob = &((struct armor_info_t *)ptr)[i].prob;
    else
      prob = &((struct obj_info *)ptr)[i].oi_prob;
    *prob += lastprob;
    lastprob = *prob;
  }

  /* Make sure it adds up to 100 */
  if (lastprob == 100)
    return;

  /* Woops, error error! */
  endwin();
  printf("\nBad percentages for %s (bound = %d): %d%%\n", str, max, lastprob);
  for (i = 0; i < max; ++i)
  {
    int prob;
    const char *name;
    if (ptr == armors)
    {
      prob = ((struct armor_info_t *)ptr)[i].prob;
      name = ((struct armor_info_t *)ptr)[i].name;
    }
    else
    {
      prob = ((struct obj_info *)ptr)[i].oi_prob;
      name = ((struct obj_info *)ptr)[i].oi_name;
    }
    printf("%3d%% %s\n", prob, name);
  }
  exit(1);
}

/*
 * init_probs:
 *	Initialize the probabilities for the various items
 */
void
init_probs(void)
{
    sumprobs('0');
    sumprobs(POTION);
    sumprobs(SCROLL);
    sumprobs(RING);
    sumprobs(STICK);
    sumprobs(WEAPON);
    sumprobs(ARMOR);
}

/*
 * pick_color:
 *	If he is halucinating, pick a random color name and return it,
 *	otherwise return the given color.
 */
const char *
pick_color(const char *col)
{
    return (is_hallucinating(&player) ? rainbow[rnd(NCOLORS)] : col);
}
