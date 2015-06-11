/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	4.12 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "io.h"
#include "os.h"
#include "rogue.h"

#include "list.h"

#ifndef NDEBUG
void
_list_assert_attached(THING const* list, THING const* item)
{
  assert(list != NULL);

  while (list != NULL && list != item)
    list = list->o.l_next;

  assert(list == item);
}
#endif

void
list_detach(THING** list, THING* item)
{
  assert(list != NULL);
  assert(*list != NULL);
  assert(item != NULL);

  if (*list == item)
    *list = item->o.l_next;

  if (item->o.l_prev != NULL)
    item->o.l_prev->o.l_next = item->o.l_next;

  if (item->o.l_next != NULL)
    item->o.l_next->o.l_prev = item->o.l_prev;

  item->o.l_next = NULL;
  item->o.l_prev = NULL;
}

void
list_attach(THING** list, THING* item)
{
  assert(list != NULL);
  assert(item != NULL);

  if (*list != NULL)
  {
    item->o.l_next = *list;
    (*list)->o.l_prev = item;
    item->o.l_prev = NULL;
  }
  else
  {
    item->o.l_next = NULL;
    item->o.l_prev = NULL;
  }
  *list = item;
}

int8_t
list_find(THING const* list, THING const* thing)
{
  assert(thing != NULL);

  THING const* ptr;
  int8_t i;
  for (ptr = list, i = 0; ptr != NULL; ptr = ptr->o.l_next, ++i)
  {
    assert(i >= 0);
    if (ptr == thing)
      return i;
  }

  return -1;
}

THING const*
list_element(THING const* list, int8_t i)
{
  if (i < 0)
    return NULL;

  for (THING const* ptr = list; ptr != NULL; ptr = ptr->o.l_next)
    if (i-- == 0)
      return ptr;

  return NULL;
}

void
list_free_all(THING** ptr)
{
  assert (ptr != NULL);

  while (*ptr != NULL)
  {
    THING* item = *ptr;
    *ptr = item->o.l_next;
    os_remove_thing(&item);
  }
}
