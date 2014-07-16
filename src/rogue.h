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

/* Tuneable - feel free to change these */
#define NUMSCORES 10    /* Number of highscore entries */
#define NUMNAME "Ten"   /* The same number in letters  */
#define MAXOBJ 9        /* How many attempts to put items in dungeon */
#define AMULETLEVEL 26  /* Level where we can find the amulet */
#define PACKSIZE 22     /* How many items we can have in our pack */


/* Try not to change these */
#define MAXSTR 1024 /* maximum length of strings */
#define MAXLINES 32 /* maximum number of screen lines used */
#define MAXCOLS  80 /* maximum number of screen columns used */
#define NUMLINES 24
#define NUMCOLS  80
#define STATLINE (NUMLINES - 1)

#undef CTRL
#define CTRL(c) (c & 037)

/* Maximum number of different things */
#define MAXROOMS	9
#define MAXTRAPS	10
#define NUMTHINGS	7	/* number of types of things */
#define MAXPASS		13	/* upper limit on number of passages */

extern enum rogue_game_t
{
  DEFAULT,
  QUICK
} game_type;

/* return values for get functions */
enum option_return
{
  NORMAL = 0,         /* normal exit */
  QUIT = 1,           /* quit option setting */
  MINUS               /* back up one option */
};

/* All the fun defines */
#define when		break;case
#define otherwise	break;default
#define until(expr)	while(!(expr))
#define next(ptr)	(*ptr).l_next
#define prev(ptr)	(*ptr).l_prev
#define winat(y,x)	(moat(y,x) != NULL ? moat(y,x)->t_disguise : chat(y,x))
#define same_coords(a,b)	((a).x == (b).x && (a).y == (b).y)
#define hero		player.t_pos
#define pstats		player.t_stats
#define pack		player.t_pack
#define proom		player.t_room
#define max_hp		player.t_stats.s_maxhp
#define attach(a,b)	_attach(&a,b)
#define detach(a,b)	_detach(&a,b)
#define free_list(a)	_free_list(&a)
#define max(a,b)	((a) > (b) ? (a) : (b))
#define on(thing,flag)	((bool)(((thing).t_flags & (flag)) != 0))
#define GOLDCALC	(rnd(50 + 10 * level) + 2)
#define ISRING(h,r)	(cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define ISWEARING(r)	(ISRING(LEFT, r) || ISRING(RIGHT, r))
#define INDEX(y,x)	(&places[((x) << 5) + (y)])
#define chat(y,x)	(places[((x) << 5) + (y)].p_ch)
#define flat(y,x)	(places[((x) << 5) + (y)].p_flags)
#define moat(y,x)	(places[((x) << 5) + (y)].p_monst)

/* for curses */
#define KEY_SPACE	' '
#define KEY_ESCAPE	27

/* things that appear on the screens */
#define SHADOW		' '
#define VWALL		'|'
#define HWALL		'-'
#define PASSAGE		'#'
#define DOOR		'+'
#define FLOOR		'.'
#define PLAYER		'@'
#define TRAP		'^'
#define STAIRS		'%'
#define GOLD		'*'
#define POTION		'!'
#define SCROLL		'?'
#define MAGIC		'$'
#define FOOD		':'
#define WEAPON		')'
#define ARMOR		']'
#define AMULET		','
#define RING		'='
#define STICK		'/'

/* TODO: Make sure these don't bug as chars are usually unsigned */
#define CALLABLE	-1 /* This means that we can rename it */
#define R_OR_S		-2 /* This means ring, wand of staff */

/* Various constants */
#define BEARTIME	spread(3)
#define WANDERTIME	spread(70)
#define BEFORE		spread(1)
#define AFTER		spread(2)
#define HUNGERTIME	1300
#define MORETIME	150
#define STOMACHSIZE	2000
#define STARVETIME	850
#define LEFT		0
#define RIGHT		1
#define BOLT_LENGTH	6
#define LAMPDIST	3
#define CONCURRENT_RINGS	2	/* How many rings we can wield */

/* Save against things */
#define VS_POISON	00
#define VS_PARALYZATION	00
#define VS_DEATH	00
#define VS_BREATH	02
#define VS_MAGIC	03

/*
 * Various flag bits
 */
/* flags for rooms */
#define ISDARK	0000001		/* room is dark */
#define ISGONE	0000002		/* room is gone (a corridor) */
#define ISMAZE	0000004		/* room is gone (a corridor) */

/* flags for objects */
#define ISCURSED 000001		/* object is cursed */
#define ISKNOW	0000002		/* player knows details about the object */
#define ISMISL	0000004		/* object is a missile type */
#define ISMANY	0000010		/* object comes in groups */
/*	ISFOUND 0000020		...is used for both objects and creatures */
#define	ISPROT	0000040		/* armor is permanently protected */

/* flags for creatures */
#define CANHUH	0000001		/* creature can confuse */
#define CANSEE	0000002		/* creature can see invisible creatures */
#define ISBLIND	0000004		/* creature is blind */
#define ISCANC	0000010		/* creature has special qualities cancelled */
#define ISLEVIT	0000010		/* hero is levitating */
#define ISFOUND	0000020		/* creature has been seen (used for objects) */
#define ISGREED	0000040		/* creature runs to protect gold */
#define ISHASTE	0000100		/* creature has been hastened */
#define ISTARGET 000200		/* creature is the target of an 'f' command */
#define ISHELD	0000400		/* creature has been held */
#define ISHUH	0001000		/* creature is confused */
#define ISINVIS	0002000		/* creature is invisible */
#define ISMEAN	0004000		/* creature can wake when player enters room */
#define ISHALU	0004000		/* hero is on acid trip */
#define ISREGEN	0010000		/* creature can regenerate */
#define ISRUN	0020000		/* creature is running at the player */
#define SEEMONST 040000		/* hero can detect unseen monsters */
#define ISFLY	0040000		/* creature can fly */
#define ISSLOW	0100000		/* creature has been slowed */

/*
 * Flags for level map
 */
#define F_PASS		0x80		/* is a passageway */
#define F_SEEN		0x40		/* have seen this spot before */
#define F_DROPPED	0x20		/* object was dropped here */
#define F_LOCKED	0x20		/* door is locked */
#define F_REAL		0x10		/* what you see is what you get */
#define F_PNUM		0x0f		/* passage number mask */
#define F_TMASK		0x07		/* trap number mask */

/* Trap types */
enum trap_t
{
  T_DOOR  = 0,
  T_ARROW = 1,
  T_SLEEP = 2,
  T_BEAR  = 3,
  T_TELEP = 4,
  T_DART  = 5,
  T_RUST  = 6,
  T_MYST  = 7,
  NTRAPS
};


/*
 * Weapon types
 */
#define MACE		0
#define SWORD		1
#define BOW		2
#define ARROW		3
#define DAGGER		4
#define TWOSWORD	5
#define DART		6
#define SHIRAKEN	7
#define SPEAR		8
#define FLAME		9	/* fake entry for dragon breath (ick) */
#define MAXWEAPONS	9	/* this should equal FLAME */

/*
 * Armor types
 */
#define LEATHER		0
#define RING_MAIL	1
#define STUDDED_LEATHER	2
#define SCALE_MAIL	3
#define CHAIN_MAIL	4
#define SPLINT_MAIL	5
#define BANDED_MAIL	6
#define PLATE_MAIL	7
#define MAXARMORS	8

/*
 * Ring types
 */
#define R_PROTECT	0
#define R_ADDSTR	1
#define R_SUSTSTR	2
#define R_SEARCH	3
#define R_SEEINVIS	4
#define R_NOP		5
#define R_AGGR		6
#define R_ADDHIT	7
#define R_ADDDAM	8
#define R_REGEN		9
#define R_DIGEST	10
#define R_TELEPORT	11
#define R_STEALTH	12
#define R_SUSTARM	13
#define MAXRINGS	14

/*
 * Rod/Wand/Staff types
 */
#define WS_LIGHT	0
#define WS_INVIS	1
#define WS_ELECT	2
#define WS_FIRE		3
#define WS_COLD		4
#define WS_POLYMORPH	5
#define WS_MISSILE	6
#define WS_HASTE_M	7
#define WS_SLOW_M	8
#define WS_DRAIN	9
#define WS_NOP		10
#define WS_TELAWAY	11
#define WS_TELTO	12
#define WS_CANCEL	13
#define MAXSTICKS	14

/*
 * Now we define the structures and types
 */

/*
 * Help list
 */
struct h_list {
    char h_ch;
    char *h_desc;
    bool h_print;
};

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

typedef unsigned int str_t;

/*
 * Stuff about objects
 */
struct obj_info {
    char *oi_name;
    int oi_prob;
    int oi_worth;
    char *oi_guess;
    bool oi_know;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    coord r_gold;			/* Where the gold is */
    int r_goldval;			/* How much the gold is worth */
    short r_flags;			/* info about the room */
    int r_nexits;			/* Number of exits */
    coord r_exit[12];			/* Where the exits are */
};

/*
 * Structure describing a fighting being
 */
struct stats {
    str_t s_str;			/* Strength */
    int s_exp;				/* Experience */
    int s_lvl;				/* level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;			/* Hit points */
    char s_dmg[13];			/* String describing damage done */
    int  s_maxhp;			/* Max hit points */
};

/*
 * Structure for monsters and player
 */
union thing {
    struct {
	union thing *_l_next, *_l_prev;	/* Next pointer in link */
	coord _t_pos;			/* Position */
	bool _t_turn;			/* If slowed, is it a turn to move */
	char _t_type;			/* What it is */
	char _t_disguise;		/* What mimic looks like */
	char _t_oldch;			/* Character that was where it was */
	coord *_t_dest;			/* Where it is running to */
	short _t_flags;			/* State word */
	struct stats _t_stats;		/* Physical description */
	struct room *_t_room;		/* Current room for thing */
	union thing *_t_pack;		/* What the thing is carrying */
        int _t_reserved;
    } _t;
    struct {
	union thing *_l_next, *_l_prev;	/* Next pointer in link */
	int _o_type;			/* What kind of object it is */
	coord _o_pos;			/* Where it lives on the screen */
	char *_o_text;			/* What it says if you read it */
	int  _o_launch;			/* What you need to launch it */
	char _o_packch;			/* What character it is in the pack */
	char _o_damage[8];		/* Damage if used like sword */
	char _o_hurldmg[8];		/* Damage if thrown */
	int _o_count;			/* count for plural objects */
	int _o_which;			/* Which object of a type it is */
	int _o_hplus;			/* Plusses to hit */
	int _o_dplus;			/* Plusses to damage */
	int _o_arm;			/* Armor protection */
	int _o_flags;			/* information about objects */
	int _o_group;			/* group number for this object */
	char *_o_label;			/* Label for object */
    } _o;
};

typedef union thing THING;

#define l_next		_t._l_next
#define l_prev		_t._l_prev
#define t_pos		_t._t_pos
#define t_turn		_t._t_turn
#define t_type		_t._t_type
#define t_disguise	_t._t_disguise
#define t_oldch		_t._t_oldch
#define t_dest		_t._t_dest
#define t_flags		_t._t_flags
#define t_stats		_t._t_stats
#define t_pack		_t._t_pack
#define t_room		_t._t_room
#define t_reserved      _t._t_reserved
#define o_type		_o._o_type
#define o_pos		_o._o_pos
#define o_text		_o._o_text
#define o_launch	_o._o_launch
#define o_packch	_o._o_packch
#define o_damage	_o._o_damage
#define o_hurldmg	_o._o_hurldmg
#define o_count		_o._o_count
#define o_which		_o._o_which
#define o_hplus		_o._o_hplus
#define o_dplus		_o._o_dplus
#define o_arm		_o._o_arm
#define o_charges	o_arm
#define o_goldval	o_arm
#define o_flags		_o._o_flags
#define o_group		_o._o_group
#define o_label		_o._o_label

/*
 * describe a place on the level map
 */
typedef struct {
    char p_ch;
    char p_flags;
    THING *p_monst;
} PLACE;

/*
 * Array containing information on all the various types of monsters
 */
struct monster {
    char *m_name;			/* What to call the monster */
    int m_carry;			/* Probability of carrying something */
    short m_flags;			/* things about the monster */
    struct stats m_stats;		/* Initial stats */
};

/* Game Options - These are set in main.c */
bool terse;       /* Terse output */
bool fight_flush; /* Flush typeahead during battle */
bool jump;        /* Show running as a series of jumps */
bool see_floor;   /* Show the lamp-illuminated floor */
bool passgo;      /* Follow the turnings in passageways */
bool tombstone;   /* Print out tombstone when killed */
bool use_colors;  /* Use ncurses colors */

/*
 * External variables
 */

extern bool	after, again, amulet, door_stop,
		firstmove, has_hit, inv_describe, kamikaze,
		move_on, msg_esc, pack_used[],
		running, seenstairs, to_death;

extern char	dir_ch, file_name[], huh[], *inv_t_name[], prbuf[], whoami[],
		l_last_comm, l_last_dir, last_comm, last_dir,
		outbuf[], *r_stones[], runch,
		*s_names[], take, *tr_name[], *ws_made[], *ws_type[];

extern int	a_class[], count, food_left, hungry_state,
		lastscore, level, max_hit, max_level, mpos,
		n_objs, no_command, no_food, no_move, purse,
		quiet, vf_hit, potential_wizard, wizard;

extern unsigned int	seed;

extern int	e_levels[];

extern WINDOW	*hw;

extern coord	delta, oldpos, stairs;

extern PLACE	places[];

extern THING	*cur_armor, *cur_ring[], *cur_weapon, *l_last_pick,
		*last_pick, *lvl_obj, *mlist, player;

extern struct room	*oldrp, passages[], rooms[];

extern struct stats	max_stats;

extern struct monster	monsters[];

extern struct obj_info	arm_info[], ring_info[],
			things[], ws_info[], weap_info[];

/*
 * Function types
 */

bool is_magic(THING *obj);   /* Returns true if an object radiates magic */
bool seen_stairs();          /* Return true if the player has seen the stairs */
bool turn_see(bool turn_off);/* Put on or off seeing monsters on this level */
void invis_on();             /* Turn on the ability to see invisible */

unsigned	items_in_pack();
unsigned	items_in_pack_of_type(int type);
void	_attach(THING **list, THING *item);
void	_detach(THING **list, THING *item);
void	_free_list(THING **ptr);
void	addmsg(const char *fmt, ...);
void	add_pack(THING *obj, bool silent);
void	add_pass();
void	add_str(str_t *sp, int amt);
void	accnt_maze(int y, int x, int ny, int nx);
void	aggravate();
int	attack(THING *mp);
void	badcheck(char *name, struct obj_info *info, int bound);
void	bounce(THING *weap, char *mname, bool noend);
void	call_it(struct obj_info *info);
bool	cansee(int y, int x);
int	center(char *str);
void	chg_str(int amt);
void	check_level();
void	conn(int r1, int r2);
void	command();
void	create_obj();

void	death(char monst);
char	death_monst();
void	dig(int y, int x);
void	discard(THING *item);
void	discovered();
int	dist(int y1, int x1, int y2, int x2);
int	dist_cp(coord *c1, coord *c2);
int	do_chase(THING *th);
void	do_daemons(int flag);
void	do_fuses(int flag);
void	do_maze(struct room *rp);
void	do_motion(THING *obj, int ydelta, int xdelta);
bool	do_move(char ch);
void	do_passages();
void	do_rooms();
bool	do_run(char ch);
bool	do_zap();
void	doadd(const char *fmt, va_list args);
void	door(struct room *rm, coord *cp);
void	door_open(struct room *rp);
void	drain();
void	draw_room(struct room *rp);
bool	drop();
void	eat();
size_t  encread(char *start, size_t size, FILE *inf);
size_t	encwrite(char *start, size_t size, FILE *outf);
bool	encread_and_check_version(char *buf, FILE *infile);
int	endmsg();
void	enter_room(coord *cp);
void	erase_lamp(coord *pos, struct room *rp);
int	exp_add(THING *tp);
void	extinguish(void (*func)());
void	fall(THING *obj, bool pr);
void	fire_bolt(coord *start, coord *dir, char *name);
char	floor_at();
void	flush_type();
int	fight(coord *mp, THING *weap, bool thrown);
void	fix_stick(THING *cur);
void	fuse(void (*func)(), int arg, int time, int type);
bool	get_dir();
int	gethand();
void	give_pack(THING *tp);
void	hit(char *er, char *ee, bool noend);
void	horiz(struct room *rp, int starty);
void	leave_room(coord *cp);
void	lengthen(void (*func)(), int xtime);
void	look(bool wakeup);
int	hit_monster(int y, int x, THING *obj);
int	init_graphics();
void	init_colors();
void	init_materials();
void	init_names();
void	init_player();
void	init_probs();
void	init_stones();
void	init_weapon(THING *weap, int which);
bool	print_inventory(int type);
void	clear_inventory();
void	killed(THING *tp, bool pr);
void	kill_daemon(void (*func)());
bool	lock_sc();
void	miss(char *er, char *ee, bool noend);
bool	missile(int ydelta, int xdelta);
int	move_monst(THING *tp);
int	msg(const char *fmt, ...);
void	unsaved_msg(const char *fmt, ...);
void	new_level();
void	new_monster(THING *tp, char type, coord *cp);
void	numpass(int y, int x);
void	option();
void 	passnum();
char	*pick_color(char *col);
void	pick_up(char ch);
void	picky_inven();
int	pr_list();
void	put_bool(void *b);
void	put_str(void *str);
void	put_things();
void	putpass(coord *cp);
char	randmonster(bool wander);
void    relocate(THING *th, coord *new_loc);
void	remove_mon(coord *mp, THING *tp, bool waskill);
void	reset_last();
int	ring_eat(int hand);
bool	ring_on();
bool	ring_off();
int	rnd(int range);
int	rnd_room();
int	roll(int number, int sides);
int	rs_save_file(FILE *savef);
int	rs_restore_file(FILE *inf);
void	runto(coord *runner);
void	rust_armor(THING *arm);
int	save(int which);
void	save_file(FILE *savef);
void	save_game();
int	save_throw(int which, THING *tp);
void	score(int amount, int flags, char monst);
void	set_know(THING *obj, struct obj_info *info);
void	set_oldch(THING *tp, coord *cp);
void	shell();
bool	show_floor();
void	show_map();
void	show_win(char *message);
int	sign(int nm);
int	spread(int nm);
void	start_daemon(void (*func)(), int arg, int type);
void	status(bool stat_msg);
int	step_ok(int ch);
void	strucpy(char *s1, char *s2, int len);
int	swing(int at_lvl, int op_arm, int wplus);
bool	take_off();
void	teleport();
void	total_winner();
void	thunk(THING *weap, char *mname, bool noend);
void	treas_room();
void	turnref();
void	unlock_sc();
void	vert(struct room *rp, int startx);
void	wait_for(int ch);
THING  *wake_monster(int y, int x);
void	wanderer();
void	waste_time();
bool	wear();
void	whatis(int type);
bool	wield();

bool	chase(THING *tp, coord *ee);
bool	diag_ok(coord *sp, coord *ep);
bool	dropcheck(THING *obj);
bool	fallpos(coord *pos, coord *newpos);
bool	find_floor(struct room *rp, coord *cp, int limit, bool monst);
bool	roll_em(THING *thatt, THING *thdef, THING *weap, bool hurl);
bool	see_monst(THING *mp);
bool	turn_ok(int y, int x);
bool	is_in_use(THING *obj);

enum trap_t	be_trapped(coord *tc);
char	readchar();
char	rnd_thing();

char	*charge_str(THING *obj);
char	*inv_name(THING *obj, bool drop);
char	*num(int n1, int n2, char type);
char	*ring_num(THING *obj);
char	*set_mname(THING *tp);
char	*vowelstr(char *str);

void doctor();
void leave(int);
void quit(int);
void rollwand();
void runners();
void stomach();
void swander();
void visuals();

char *md_getusername();
char *md_gethomedir();
int md_hasclreol();

enum option_return	get_bool(void *vp, WINDOW *win);
enum option_return	get_num(void *vp, WINDOW *win);
enum option_return	get_sf(void *vp, WINDOW *win);
enum option_return	get_str(void *vopt, WINDOW *win);

#define	incch() wincch(stdscr)
chtype	wincch(WINDOW *win);
#define	mvincch(y, x) mvwincch(stdscr, y, x)
chtype	mvwincch(WINDOW *win, int y, int x);
#define	addcch(ch) waddcch(stdscr, ch)
int	waddcch(WINDOW *window, const chtype ch);
#define	mvaddcch(y, x, ch) mvwaddcch(stdscr, y, x, ch)
int	mvwaddcch(WINDOW *window, int y, int x, const chtype ch);

coord	*find_dest(THING *tp);
coord	*rndmove(THING *who);

THING	*find_obj(int y, int x);
THING	*get_item(char *purpose, int type);
THING	*leave_pack(THING *obj, bool newobj, bool all);
THING	*new_item();
THING	*new_thing();

struct room	*roomin(coord *cp);

#define MAXDAEMONS 20

extern struct delayed_action {
    int d_type;
    void (*d_func)();
    int d_arg;
    int d_time;
} d_list[MAXDAEMONS];

typedef struct {
    char	*st_name;
    int		st_value;
} STONE;

extern int      total;
extern int      between;
extern int      group;
extern coord    nh;
extern char     *rainbow[];
extern int      cNCOLORS;
extern STONE    stones[];
extern int      cNSTONES;
extern char     *wood[];
extern int      cNWOOD;
extern char     *metal[];
extern int      cNMETAL;

#endif /* _ROGUE14_ROGUE_H_ */
