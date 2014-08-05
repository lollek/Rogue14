/*
 * Code for one creature to chase another
 *
 * @(#)chase.c	4.57 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>

#include "rogue.h"
#include "status_effects.h"
#include "scrolls.h"
#include "command.h"
#include "io.h"

#include "chase.h"

#define DRAGONSHOT  5  /* one chance in DRAGONSHOT that a dragon will flame */
static coord ch_ret;   /* Where chasing takes you */

void
runners(void)
{
  THING *tp;
  THING *next;

  for (tp = mlist; tp != NULL; tp = next)
  {
    /* remember this in case the monster's "next" is changed */
    next = next(tp);
    if (!on(*tp, ISHELD) && on(*tp, ISRUN))
    {
      bool wastarget = on(*tp, ISTARGET);
      coord orig_pos = tp->t_pos;
      if (move_monst(tp) == -1)
        continue;

      if (on(*tp, ISFLY) && dist_cp(&hero, &tp->t_pos) >= 3)
        move_monst(tp);

      if (wastarget && !same_coords(orig_pos, tp->t_pos))
      {
        tp->t_flags &= ~ISTARGET;
        to_death = false;
      }
    }
  }
  if (has_hit)
  {
    endmsg();
    has_hit = false;
  }
}

/* TODO: Change this to a bool */
int
move_monst(THING *tp)
{
  if (!on(*tp, ISSLOW) || tp->t_turn)
    if (do_chase(tp) == -1)
      return -1;

  if (on(*tp, ISHASTE))
    if (do_chase(tp) == -1)
      return -1;

  tp->t_turn ^= true;
  return(0);
}

int
do_chase(THING *th)
{
    coord *cp;
    struct room *rer = th->t_room; /* room of chaser, */
    struct room *ree; /* room of chasee */
    int mindist = 32767;
    int curdist;
    bool stoprun = false; /* true means we are there */
    bool door;
    THING *obj;
    static coord this; /* Temporary destination for chaser */

    if (on(*th, ISGREED) && rer->r_goldval == 0)
	th->t_dest = &hero;	/* If gold has been taken, run after hero */

    if (th->t_dest == &hero)	/* Find room of chasee */
	ree = proom;
    else
	ree = roomin(th->t_dest);

    /* We don't count doors as inside rooms for this routine */
    door = (chat(th->t_pos.y, th->t_pos.x) == DOOR);

    /* If the object of our desire is in a different room,
     * and we are not in a corridor, run to the door nearest to
     * our goal */
over:
    if (rer != ree)
    {
	for (cp = rer->r_exit; cp < &rer->r_exit[rer->r_nexits]; cp++)
	{
	    curdist = dist_cp(th->t_dest, cp);
	    if (curdist < mindist)
	    {
		this = *cp;
		mindist = curdist;
	    }
	}
	if (door)
	{
	    rer = &passages[flat(th->t_pos.y, th->t_pos.x) & F_PNUM];
	    door = false;
	    goto over;
	}
    }
    else
    {
	this = *th->t_dest;
	
	/* For dragons check and see if (a) the hero is on a straight
	 * line from it, and (b) that it is within shooting distance,
	 * but outside of striking range */
	if (th->t_type == 'D' && (th->t_pos.y == hero.y || th->t_pos.x == hero.x
	    || abs(th->t_pos.y - hero.y) == abs(th->t_pos.x - hero.x))
	    && dist_cp(&th->t_pos, &hero) <= BOLT_LENGTH * BOLT_LENGTH
	    && !is_cancelled(th) && rnd(DRAGONSHOT) == 0)
	{
	    delta.y = sign(hero.y - th->t_pos.y);
	    delta.x = sign(hero.x - th->t_pos.x);
	    if (has_hit)
		endmsg();
	    fire_bolt(&th->t_pos, &delta, "flame");
	    stop_counting(true);
	    quiet = 0;
	    if (to_death && !on(*th, ISTARGET))
	    {
		to_death = false;
		kamikaze = false;
	    }
	    return(0);
	}
    }
    /*
     * This now contains what we want to run to this time
     * so we run to it.  If we hit it we either want to fight it
     * or stop running
     */
    if (!chase(th, &this))
    {
	if (same_coords(this, hero))
	    return( attack(th) );
	else if (same_coords(this, *th->t_dest))
	{
	    for (obj = lvl_obj; obj != NULL; obj = next(obj))
		if (th->t_dest == &obj->o_pos)
		{
		    detach(lvl_obj, obj);
		    attach(th->t_pack, obj);
		    chat(obj->o_pos.y, obj->o_pos.x) =
			(th->t_room->r_flags & ISGONE) ? PASSAGE : FLOOR;
		    th->t_dest = find_dest(th);
		    break;
		}
	    if (th->t_type != 'F')
		stoprun = true;
	}
    }
    else
    {
	if (th->t_type == 'F')
	    return(0);
    }

    if (!same_coords(ch_ret, th->t_pos))
    {
      struct room *oroom;
      mvaddcch(th->t_pos.y, th->t_pos.x, th->t_oldch);
      th->t_room = roomin(&ch_ret);
      set_oldch(th, &ch_ret);
      oroom = th->t_room;
      moat(th->t_pos.y, th->t_pos.x) = NULL;

      if (oroom != th->t_room)
        th->t_dest = find_dest(th);
      th->t_pos = ch_ret;
      moat(ch_ret.y, ch_ret.x) = th;
    }
    move(ch_ret.y, ch_ret.x);

    if (see_monst(th))
      addcch(th->t_disguise);
    else if (on(player, SEEMONST))
      addcch(th->t_type | A_STANDOUT);

    /* And stop running if need be */
    if (stoprun && same_coords(th->t_pos, *(th->t_dest)))
      th->t_flags &= ~ISRUN;
    return(0);
}

void
set_oldch(THING *tp, coord *cp)
{
  char sch = tp->t_oldch;

  if (same_coords(tp->t_pos, *cp))
    return;

  tp->t_oldch = mvincch(cp->y, cp->x);
  if (!is_blind(&player))
  {
    if ((sch == FLOOR || tp->t_oldch == FLOOR) &&
        (tp->t_room->r_flags & ISDARK))
      tp->t_oldch = SHADOW;
    else if (dist_cp(cp, &hero) <= LAMPDIST && see_floor)
      tp->t_oldch = chat(cp->y, cp->x);
  }
}

bool
see_monst(THING *mp)
{
  int y = mp->t_pos.y;
  int x = mp->t_pos.x;

  if (is_blind(&player) ||
      (is_invisible(mp) && !is_true_seeing(&player)))
    return false;

  if (dist(y, x, hero.y, hero.x) < LAMPDIST)
  {
    if (y != hero.y && x != hero.x &&
        !step_ok(chat(y, hero.x)) && !step_ok(chat(hero.y, x)))
      return false;
    return true;
  }

  if (mp->t_room != proom)
    return false;
  return ((bool)!(mp->t_room->r_flags & ISDARK));
}

void
runto(coord *runner)
{
  THING *tp = moat(runner->y, runner->x);

  /* If we couldn't find him, something is funny */
  if (wizard && tp == NULL)
    msg("couldn't find monster in runto at (%d,%d)", runner->y, runner->x);

  /* Start the beastie running */
  tp->t_flags |= ISRUN;
  tp->t_flags &= ~ISHELD;
  tp->t_dest = find_dest(tp);
}

bool
chase(THING *tp, coord *ee)
{
    THING *obj;
    int x, y;
    int curdist, thisdist;
    coord *er = &tp->t_pos;
    char ch;
    int plcnt = 1;
    static coord tryp;

    /*
     * If the thing is confused, let it move randomly. Invisible
     * Stalkers are slightly confused all of the time, and bats are
     * quite confused all the time
     */
    if ((is_confused(tp) && rnd(5) != 0) || (tp->t_type == 'P' && rnd(5) == 0)
	|| (tp->t_type == 'B' && rnd(2) == 0))
    {
	/*
	 * get a valid random move
	 */
	ch_ret = *rndmove(tp);
	curdist = dist_cp(&ch_ret, ee);
	/*
	 * Small chance that it will become un-confused 
	 */
	if (rnd(20) == 0)
	    set_confused(tp, false);
    }
    /*
     * Otherwise, find the empty spot next to the chaser that is
     * closest to the chasee.
     */
    else
    {
	int ey, ex;
	/*
	 * This will eventually hold where we move to get closer
	 * If we can't find an empty spot, we stay where we are.
	 */
	curdist = dist_cp(er, ee);
	ch_ret = *er;

	ey = er->y + 1;
	if (ey >= NUMLINES - 1)
	    ey = NUMLINES - 2;
	ex = er->x + 1;
	if (ex >= NUMCOLS)
	    ex = NUMCOLS - 1;

	for (x = er->x - 1; x <= ex; x++)
	{
	    if (x < 0)
		continue;
	    tryp.x = x;
	    for (y = er->y - 1; y <= ey; y++)
	    {
		tryp.y = y;
		if (!diag_ok(er, &tryp))
		    continue;
		ch = winat(y, x);
		if (step_ok(ch))
		{
		    /*
		     * If it is a scroll, it might be a scare monster scroll
		     * so we need to look it up to see what type it is.
		     */
		    if (ch == SCROLL)
		    {
			for (obj = lvl_obj; obj != NULL; obj = next(obj))
			{
			    if (y == obj->o_pos.y && x == obj->o_pos.x)
				break;
			}
			if (obj != NULL && obj->o_which == S_SCARE)
			    continue;
		    }
		    /*
		     * It can also be a Xeroc, which we shouldn't step on
		     */
		    if ((obj = moat(y, x)) != NULL && obj->t_type == 'X')
			continue;
		    /*
		     * If we didn't find any scrolls at this place or it
		     * wasn't a scare scroll, then this place counts
		     */
		    thisdist = dist(y, x, ee->y, ee->x);
		    if (thisdist < curdist)
		    {
			plcnt = 1;
			ch_ret = tryp;
			curdist = thisdist;
		    }
		    else if (thisdist == curdist && rnd(++plcnt) == 0)
		    {
			ch_ret = tryp;
			curdist = thisdist;
		    }
		}
	    }
	}
    }
    return (bool)(curdist != 0 && !same_coords(ch_ret, hero));
}

struct room *
roomin(coord *cp)
{
  char *fp = &flat(cp->y, cp->x);
  struct room *rp;

  if (*fp & F_PASS)
    return &passages[*fp & F_PNUM];

  for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
    if (cp->x <= rp->r_pos.x + rp->r_max.x && rp->r_pos.x <= cp->x
        && cp->y <= rp->r_pos.y + rp->r_max.y && rp->r_pos.y <= cp->y)
      return rp;

  msg("in some bizarre place (%d, %d)", cp->y, cp->x);
  return NULL;
}

bool
diag_ok(coord *sp, coord *ep)
{
  if (ep->x < 0 || ep->x >= NUMCOLS || ep->y <= 0 || ep->y >= NUMLINES - 1)
    return false;
  if (ep->x == sp->x || ep->y == sp->y)
    return true;
  return (bool)(step_ok(chat(ep->y, sp->x)) && step_ok(chat(sp->y, ep->x)));
}

bool
cansee(int y, int x)
{
    struct room *rer;
    static coord tp;

    if (is_blind(&player))
	return false;
    if (dist(y, x, hero.y, hero.x) < LAMPDIST)
    {
	if (flat(y, x) & F_PASS)
	    if (y != hero.y && x != hero.x &&
		!step_ok(chat(y, hero.x)) && !step_ok(chat(hero.y, x)))
		    return false;
	return true;
    }
    /*
     * We can only see if the hero in the same room as
     * the coordinate and the room is lit or if it is close.
     */
    tp.y = y;
    tp.x = x;
    return (bool)((rer = roomin(&tp)) == proom && !(rer->r_flags & ISDARK));
}

coord *
find_dest(THING *tp)
{
    THING *obj;
    int prob;

    if ((prob = monsters[tp->t_type - 'A'].m_carry) <= 0 || tp->t_room == proom
	|| see_monst(tp))
	    return &hero;
    for (obj = lvl_obj; obj != NULL; obj = next(obj))
    {
	if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
	    continue;
	if (roomin(&obj->o_pos) == tp->t_room && rnd(100) < prob)
	{
	    for (tp = mlist; tp != NULL; tp = next(tp))
		if (tp->t_dest == &obj->o_pos)
		    break;
	    if (tp == NULL)
		return &obj->o_pos;
	}
    }
    return &hero;
}

int
dist(int y1, int x1, int y2, int x2)
{
    return ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

