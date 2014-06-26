/*
 * Routines to deal with the pack
 *
 * @(#)pack.c	4.40 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <string.h>

#include "rogue.h"
#include "status_effects.h"
#include "scrolls.h"

static char pack_char();
static void move_msg(THING *obj);
static void money(int value);
static char floor_ch();
static void remove_from_floor(THING *obj);

/** add_pack:
 * Pick up an object and add it to the pack.  If the argument is
 * non-null use it as the linked_list pointer instead of gettting
 * it off the ground. */

void
add_pack(THING *obj, bool silent)
{
    THING *op;
    bool from_floor = false;

    /* Either obj in an item or we try to take something from the floor */
    if (obj == NULL)
    {
	if ((obj = find_obj(hero.y, hero.x)) == NULL)
	    return;
	from_floor = true;
    }

    /* Check for and deal with scare monster scrolls */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE &&
        obj->o_flags & ISFOUND)
    {
	detach(lvl_obj, obj);
	mvaddcch(hero.y, hero.x, floor_ch());
	chat(hero.y, hero.x) = (proom->r_flags & ISGONE) ? PASSAGE : FLOOR;
	discard(obj);
	msg("the scroll turns to dust as you pick it up");
	return;
    }

    if (++inpack > PACKSIZE)
    {
	if (!terse)
	    addmsg("there's ");
	addmsg("no room");
	if (!terse)
	    addmsg(" in your pack");
	endmsg();
	if (from_floor)
	    move_msg(obj);
	inpack = PACKSIZE;
	return;
    }

    if (pack == NULL)
    {
	if (from_floor)
	    remove_from_floor(obj);
	attach(pack, obj);
	obj->o_packch = pack_char();
    }
    else
    {
	THING *lp = NULL;
	for (op = pack; op != NULL; op = next(op))
	{
	    if (op->o_type != obj->o_type)
		lp = op;
	    else
	    {
		while (op->o_type == obj->o_type && op->o_which != obj->o_which)
		{
		    lp = op;
		    if (next(op) == NULL)
			break;
		    else
			op = next(op);
		}
		if (op->o_type == obj->o_type && op->o_which == obj->o_which)
		{
		    if (op->o_type == POTION || op->o_type == SCROLL ||
                        obj->o_type == FOOD)
		    {
			if (from_floor)
			    remove_from_floor(obj);
			op->o_count++;
			discard(obj);
			obj = op;
			lp = NULL;
			break;
		    }
		    else if (obj->o_group)
		    {
			lp = op;
			while (op->o_type == obj->o_type
			    && op->o_which == obj->o_which
			    && op->o_group != obj->o_group)
			{
			    lp = op;
			    if (next(op) == NULL)
				break;
			    else
				op = next(op);
			}
			if (op->o_type == obj->o_type
			    && op->o_which == obj->o_which
			    && op->o_group == obj->o_group)
			{
				op->o_count += obj->o_count;
				inpack--;
				if (from_floor)
				    remove_from_floor(obj);
				discard(obj);
				obj = op;
				lp = NULL;
				break;
			}
		    }
		    else
			lp = op;
		}
		break;
	    }
	}

	if (lp != NULL)
	{
	    if (from_floor)
		remove_from_floor(obj);
	    obj->o_packch = pack_char();
	    next(obj) = next(lp);
	    prev(obj) = lp;
	    if (next(lp) != NULL)
		prev(next(lp)) = obj;
	    next(lp) = obj;
	}
    }

    obj->o_flags |= ISFOUND;

    /*
     * If this was the object of something's desire, that monster will
     * get mad and run at the hero.
     */
    for (op = mlist; op != NULL; op = next(op))
	if (op->t_dest == &obj->o_pos)
	    op->t_dest = &hero;

    if (obj->o_type == AMULET)
	amulet = true;

    /* Notify the user */
    if (!silent)
    {
	if (!terse)
	    addmsg("you now have ");
	msg("%s (%c)", inv_name(obj, !terse), obj->o_packch);
    }
}

/*
 * leave_pack:
 *	take an item out of the pack
 */
THING *
leave_pack(THING *obj, bool newobj, bool all)
{
    THING *nobj;

    inpack--;
    nobj = obj;
    if (obj->o_count > 1 && !all)
    {
	last_pick = obj;
	obj->o_count--;
	if (obj->o_group)
	    inpack++;
	if (newobj)
	{
	    nobj = new_item();
	    *nobj = *obj;
	    next(nobj) = NULL;
	    prev(nobj) = NULL;
	    nobj->o_count = 1;
	}
    }
    else
    {
	last_pick = NULL;
	pack_used[obj->o_packch - 'a'] = false;
	detach(pack, obj);
    }
    return nobj;
}

/*
 * pack_char:
 *	Return the next unused pack character.
 */
static char
pack_char()
{
    bool *bp;

    for (bp = pack_used; *bp; bp++)
	continue;
    *bp = true;
    return (char)((int)(bp - pack_used) + 'a');
}

/*
 * inventory:
 *	List what is in the pack.  Return true if there is something of
 *	the given type.
 */
bool
inventory(THING *list, int type)
{
    static char inv_temp[MAXSTR];

    n_objs = 0;
    for (; list != NULL; list = next(list))
    {
	if (type && type != list->o_type && !(type == CALLABLE &&
	    list->o_type != FOOD && list->o_type != AMULET) &&
	    !(type == R_OR_S && (list->o_type == RING || list->o_type == STICK)))
		continue;
	n_objs++;
	if (wizard && !list->o_packch)
	    strcpy(inv_temp, "%s");
	else
	    sprintf(inv_temp, "%c) %%s", list->o_packch);
	msg_esc = true;
	if (add_line(inv_temp, inv_name(list, false)) == KEY_ESCAPE)
	{
	    msg_esc = false;
	    msg("");
	    return true;
	}
	msg_esc = false;
    }
    if (n_objs == 0)
    {
	if (terse)
	    msg(type == 0 ? "empty handed" :
			    "nothing appropriate");
	else
	    msg(type == 0 ? "you are empty handed" :
			    "you don't have anything appropriate");
	return false;
    }
    end_line();
    return true;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */

/* TODO: Maybe move this to command.c? */
void
pick_up(char ch)
{
    THING *obj;

    if (is_levitating(&player))
	return;

    obj = find_obj(hero.y, hero.x);
    if (move_on)
	move_msg(obj);
    else
	switch (ch)
	{
	    case GOLD:
		if (obj == NULL)
		    return;
		money(obj->o_goldval);
		detach(lvl_obj, obj);
		discard(obj);
		proom->r_goldval = 0;
		break;
	    default:
	    if (wizard)
		msg("Where did you pick a '%s' up???", unctrl(ch));
	    case ARMOR:
	    case POTION:
	    case FOOD:
	    case WEAPON:
	    case SCROLL:	
	    case AMULET:
	    case RING:
	    case STICK:
		add_pack((THING *) NULL, false);
		break;
	}
}

/*
 * move_msg:
 *	Print out the message if you are just moving onto an object
 */

static void
move_msg(THING *obj)
{
    if (!terse)
	addmsg("you ");
    msg("moved onto %s", inv_name(obj, true));
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */

/* TODO: Maybe move this to command.c? */
void
picky_inven()
{
    THING *obj;
    char mch;

    if (pack == NULL)
	msg("you aren't carrying anything");
    else if (next(pack) == NULL)
	msg("a) %s", inv_name(pack, false));
    else
    {
	msg(terse ? "item: " : "which item do you wish to inventory: ");
	mpos = 0;
	if ((mch = readchar()) == KEY_ESCAPE)
	{
	    msg("");
	    return;
	}
	for (obj = pack; obj != NULL; obj = next(obj))
	    if (mch == obj->o_packch)
	    {
		msg("%c) %s", mch, inv_name(obj, false));
		return;
	    }
	msg("'%s' not in pack", unctrl(mch));
    }
}

/*
 * get_item:
 *	Pick something out of a pack for a purpose
 */
THING *
get_item(char *purpose, int type)
{
    THING *obj;
    char ch;

    if (pack == NULL)
	msg("you aren't carrying anything");
    else if (again)
	if (last_pick)
	    return last_pick;
	else
	    msg("you ran out");
    else
    {
	for (;;)
	{
	    if (!terse)
		addmsg("which object do you want to ");
	    addmsg(purpose);
	    if (terse)
		addmsg(" what");
	    msg("? (* for list): ");
	    ch = readchar();
	    mpos = 0;
	    /*
	     * Give the poor player a chance to abort the command
	     */
	    if (ch == KEY_ESCAPE)
	    {
		reset_last();
		after = false;
		msg("");
		return NULL;
	    }
	    n_objs = 1;		/* normal case: person types one char */
	    if (ch == '*')
	    {
		mpos = 0;
		if (inventory(pack, type) == 0)
		{
		    after = false;
		    return NULL;
		}
		continue;
	    }
	    for (obj = pack; obj != NULL; obj = next(obj))
		if (obj->o_packch == ch)
		    break;
	    if (obj == NULL)
	    {
		msg("'%s' is not a valid item",unctrl(ch));
		continue;
	    }
	    else 
		return obj;
	}
    }
    return NULL;
}

/*
 * money:
 *	Add or subtract gold from the pack
 */

/* TODO: Maybe inline this function? */

static void
money(int value)
{
    purse += value;
    mvaddcch(hero.y, hero.x, floor_ch());
    chat(hero.y, hero.x) = (proom->r_flags & ISGONE) ? PASSAGE : FLOOR;
    if (value > 0)
    {
	if (!terse)
	    addmsg("you found ");
	msg("%d gold pieces", value);
    }
}

/*
 * floor_ch:
 *	Return the appropriate floor character for her room
 */
static char
floor_ch()
{
    if (proom->r_flags & ISGONE)
	return PASSAGE;
    return (show_floor() ? FLOOR : SHADOW);
}

/*
 * floor_at:
 *	Return the character at hero's position, taking see_floor
 *	into account
 */
char
floor_at()
{
    char ch;

    ch = chat(hero.y, hero.x);
    if (ch == FLOOR)
	ch = floor_ch();
    return ch;
}

/*
 * reset_last:
 *	Reset the last command when the current one is aborted
 */

void
reset_last()
{
    last_comm = l_last_comm;
    last_dir = l_last_dir;
    last_pick = l_last_pick;
}

static void
remove_from_floor(THING *obj)
{
    detach(lvl_obj, obj);
    mvaddcch(hero.y, hero.x, floor_ch());
    chat(hero.y, hero.x) = (proom->r_flags & ISGONE) ? PASSAGE : FLOOR;
}
