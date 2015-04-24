/*
 * Contains functions for dealing with things that happen in the
 * future.
 *
 * @(#)daemon.c	4.7 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "io.h"
#include "rogue.h"
#include "daemons.h"

#define EMPTY 0
#define DAEMON -1
#define MAXDAEMONS 20

static struct delayed_action d_list[MAXDAEMONS] = {
    { EMPTY }, { EMPTY }, { EMPTY }, { EMPTY }, { EMPTY },
    { EMPTY }, { EMPTY }, { EMPTY }, { EMPTY }, { EMPTY },
    { EMPTY }, { EMPTY }, { EMPTY }, { EMPTY }, { EMPTY },
    { EMPTY }, { EMPTY }, { EMPTY }, { EMPTY }, { EMPTY },
};

void *__daemons_ptr(void) { return d_list; }

/** daemon_empty_slot:
 * Find an empty slot in the daemon/fuse list */
static struct delayed_action *
daemon_empty_slot(void)
{
  struct delayed_action *dev;

  for (dev = d_list; dev <= &d_list[MAXDAEMONS-1]; dev++)
    if (dev->d_type == EMPTY)
      return dev;

  msg("DEBUG: Ran out of fuse slots :(");
  return NULL;
}

/** daemon_find_slot:
 * Find a particular slot in the table */
static struct delayed_action *
daemon_find_slot(void (*func)())
{
  struct delayed_action *dev;

  for (dev = d_list; dev <= &d_list[MAXDAEMONS-1]; dev++)
    if (dev->d_type != EMPTY && func == dev->d_func)
      return dev;

  return NULL;
}

/** daemon_start:
 * Start a daemon, takes a function. */
void
daemon_start(void (*func)(), int arg, int type)
{
  struct delayed_action *dev = daemon_empty_slot();
  if (dev != NULL)
  {
    dev->d_type = type;
    dev->d_func = func;
    dev->d_arg = arg;
    dev->d_time = DAEMON;
  }
}

/** daemon_kill:
 * Remove a daemon from the list */
void
daemon_kill(void (*func)())
{
  struct delayed_action *dev = daemon_find_slot(func);
  if (dev != NULL)
    dev->d_type = EMPTY;
}

/** daemon_run_all:
 * Run all the daemons that are active with the current flag,
 * passing the argument to the function. */
void
daemon_run_all(int flag)
{
  struct delayed_action *dev;

  for (dev = d_list; dev <= &d_list[MAXDAEMONS-1]; dev++)
    if (dev->d_type == flag && dev->d_time == DAEMON)
      (*dev->d_func)(dev->d_arg);
}

/** fuse:
 * Start a fuse to go off in a certain number of turns */
void
daemon_start_fuse(void (*func)(), int arg, int time, int type)
{
  struct delayed_action *wire = daemon_empty_slot();
  if (wire != NULL)
  {
    wire->d_type = type;
    wire->d_func = func;
    wire->d_arg = arg;
    wire->d_time = time;
  }
}

/** daemon_lengthen_fuse:
 * Increase the time until a fuse goes off */
void
daemon_lengthen_fuse(void (*func)(), int xtime)
{
  struct delayed_action *wire = daemon_find_slot(func);
  if (wire != NULL)
    wire->d_time += xtime;
}

/** daemon_extinguish_fuse:
 * Put out a fuse */
void
daemon_extinguish_fuse(void (*func)())
{
  struct delayed_action *wire = daemon_find_slot(func);
  if (wire != NULL)
    wire->d_type = EMPTY;
}

/** daemon_run_fuses:
 * Decrement counters and start needed fuses */
void
daemon_run_fuses(int flag)
{
  struct delayed_action *wire;

  for (wire = d_list; wire <= &d_list[MAXDAEMONS-1]; wire++)
    if (flag == wire->d_type && wire->d_time > 0 && --wire->d_time == 0)
    {
      wire->d_type = EMPTY;
      (*wire->d_func)(wire->d_arg);
    }
}
