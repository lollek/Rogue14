#ifndef _ROGUE14_ROGUE_H_
#define _ROGUE14_ROGUE_H_
/*
 * Rogue definitions and variable declarations
 *
 * @(#)rogue.h	5.42 (Berkeley) 08/06/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdbool.h>
#include <ncurses.h>

#include "coord.h"
#include "things.h"

/* Version */
#define GAME_VERSION "Rogue14 " VERSION " - Based on Rogue5.4.4"

/* Tuneable - feel free to change these */
#define NUMNAME    "Ten"   /* The same number in letters  */
#define NUMSCORES    10    /* Number of highscore entries */
#define AMULETLEVEL  26    /* Level where we can find the amulet */

/* Try not to change these */
#define MAXSTR 1024 /* maximum length of strings */
#define MAXINP   50 /* max string to read from terminal or environment */
#define MAXLINES 32 /* maximum number of screen lines used */
#define MAXCOLS  80 /* maximum number of screen columns used */
#define NUMLINES 24
#define NUMCOLS  80
#define PACKSIZE 22 /* How many items we can have in our pack */
#define STATLINE (NUMLINES - 1)

extern enum rogue_game_t
{
  DEFAULT,
  QUICK
} game_type;

/* All the fun defines */
#define winat(y,x)	(moat(y,x) != NULL ? moat(y,x)->t_disguise : chat(y,x))
#define max(a,b)	((a) > (b) ? (a) : (b))
#define GOLDCALC	(rnd(50 + 10 * level) + 2)

/* Various constants */
#define WANDERTIME	spread(70)
#define BEFORE		spread(1)
#define AFTER		spread(2)
#define HUNGERTIME	1300
#define STOMACHSIZE	2000
#define STARVETIME	850
#define LEFT		0
#define RIGHT		1
#define BOLT_LENGTH	6
#define LAMPDIST	3

/* Save against things */
#define VS_POISON	00
#define VS_PARALYZATION	00
#define VS_DEATH	00
#define VS_BREATH	02
#define VS_MAGIC	03

/*
 * describe a place on the level map
 */
typedef struct {
    char p_ch;
    char p_flags;
    THING* p_monst;
} PLACE;

/* Game Options - These are set in main.c */
bool terse;       /* Terse output */
bool fight_flush; /* Flush typeahead during battle */
bool jump;        /* Show running as a series of jumps */
bool see_floor;   /* Show the lamp-illuminated floor */
bool passgo;      /* Follow the turnings in passageways */
bool use_colors;  /* Use ncurses colors */

/*
 * External variables
 */

extern bool after;
extern bool again;
extern bool door_stop;
extern bool firstmove;
extern bool has_hit;
extern bool kamikaze;
extern bool move_on;
extern bool running;
extern bool to_death;

extern char dir_ch;
extern char file_name[];
extern char huh[];
extern char whoami[];
extern char l_last_comm;
extern char l_last_dir;
extern char last_comm;
extern char last_dir;
extern char outbuf[];
extern char runch;
extern char take;
extern char* tr_name[];

extern int food_left;
extern int hungry_state;
extern int level;
extern int mpos;
extern int no_command;
extern int no_food;
extern int no_move;
extern int purse;
extern int vf_hit;
extern int wizard;

extern unsigned seed;

extern coord oldpos;

typedef struct {
    char* st_name;
    int st_value;
} STONE;


#endif /* _ROGUE14_ROGUE_H_ */
