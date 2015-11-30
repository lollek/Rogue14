#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "potions.h"
#include "scrolls.h"
#include "io.h"
#include "pack.h"
#include "daemons.h"
#include "colors.h"
#include "list.h"
#include "rings.h"
#include "save.h"
#include "passages.h"
#include "rooms.h"
#include "level.h"
#include "player.h"
#include "wand.h"
#include "monster.h"
#include "os.h"
#include "weapons.h"
#include "food.h"
#include "rogue.h"

#include "state.h"

#define SUCCESS 0

#define rs_assert(_a) if (_a) { return printf("Error (#%d)", __LINE__); }

static FILE* file;

extern int group;

static bool
state_load_room(struct room* room)
{
  return
    state_load_coord(&room->r_pos) ||
    state_load_coord(&room->r_max) ||
    state_load_coord(&room->r_gold) ||
    state_load_int32(&room->r_goldval) ||
    state_load_int32(&room->r_flags) ||
    state_load_int32(&room->r_nexits) ||
    state_load_coord(&room->r_exit[0]) ||
    state_load_coord(&room->r_exit[1]) ||
    state_load_coord(&room->r_exit[2]) ||
    state_load_coord(&room->r_exit[3]) ||
    state_load_coord(&room->r_exit[4]) ||
    state_load_coord(&room->r_exit[5]) ||
    state_load_coord(&room->r_exit[6]) ||
    state_load_coord(&room->r_exit[7]) ||
    state_load_coord(&room->r_exit[8]) ||
    state_load_coord(&room->r_exit[9]) ||
    state_load_coord(&room->r_exit[10]) ||
    state_load_coord(&room->r_exit[11])

    ? io_fail("state_load_room(%p)\r\n", room)
    : SUCCESS;
}

static bool
state_read(void* buf, int32_t length)
{
  if (file == NULL)
    return io_fail("state_read(%p, %d) File is NULL\r\n", buf, length);
  if (buf == NULL)
    return io_fail("state_read(%p, %d) Buffer is NULL\r\n", buf, length);
  if (length <= 0)
    return io_fail("state_read(%p, %d) Length is too short\r\n", buf, length);

  return io_encread(buf, (size_t)length, file) == (size_t)length
    ? SUCCESS
    : io_fail("state_read:(buf: %p, length: %d)\r\n", buf, length);
}

bool
state_load_int8(int8_t* data)
{
  return state_read(  data, 1)
    ? io_fail("state_load_int8(%p)\r\n", data)
    : SUCCESS;
}

bool
state_load_int32(int32_t* data)
{
  return state_read(  data, 4)
    ? io_fail("state_load_int32(%p)\r\n", data)
    : SUCCESS;
}

bool
state_assert_int32(int32_t expected)
{
  int32_t found;
  state_load_int32(&found);
  return found != expected
    ? io_fail("state_assert_int32: %X should have been %X\r\n", found, expected)
    : SUCCESS;
}

static bool
state_load_bool(bool* data)
{
  int8_t b;

  if (state_load_int8(&b))
    return io_fail("state_load_bool(%p)\r\n", data);

  *data = b != 0;
  return SUCCESS;
}

bool
state_load_bools(bool* i, int32_t count)
{
  int32_t n = 0;

  if (state_load_int32(&n) || n != count)
    return io_fail("state_load_bools(%p, %d)\r\n", i, count);

  for (n = 0; n < count; n++)
    if (state_load_bool(&i[n]))
      return io_fail("state_load_bools(%p, %d)\r\n", i, count);

  return SUCCESS;
}

static bool
state_load_char(char* c)
{
  int8_t i;
  if (state_load_int8(&i))
    return io_fail("state_load_char('%p')\r\n", c);
  *c = (char)i;
  return SUCCESS;
}

static bool
state_load_chars(char* c, int32_t len)
{
  if (state_assert_int32(len))
    return io_fail("state_load_chars(%c, %d)\r\n", c, len);
  if (len == 0)
    return SUCCESS;
  return state_read(c, len)
    ? io_fail("state_load_chars(%c, %d)\r\n", c, len)
    : SUCCESS;
}

static bool state_load_stats(struct stats* s)
{
  return
    state_assert_int32(RSID_STATS) ||
    state_load_int32(&s->s_str) ||
    state_load_int32(&s->s_exp) ||
    state_load_int32(&s->s_lvl) ||
    state_load_int32(&s->s_arm) ||
    state_load_int32(&s->s_hpt) ||
    state_load_structs_damage(s->s_dmg) ||
    state_load_int32(&s->s_maxhp)

    ? io_fail("state_load_stats(%p)\r\n", s)
    : SUCCESS;
}


bool
state_load_string(char** s)
{
  int32_t len = 0;

  if (state_load_int32(&len) || len < 0)
    return io_fail("state_load_string(s: %p[%p]) read %d\r\n", s, *s, len);

  if (len == 0)
    *s = NULL;
  else
  {
    *s = malloc((size_t)len);
    if (*s == NULL)
      return io_fail("state_load_string(s: %p[%p]) malloc(%d) io_failed\r\n",
                  s, *s, len);
  }

  return state_load_chars(*s, len)
    ? io_fail("state_load_string(s: %p[%p]) Load Chars\r\n", s, *s)
    : SUCCESS;
}

bool
state_load_coord(coord* c)
{
  return state_load_int32(&c->y) || state_load_int32(&c->x)
    ? io_fail("state_load_coord(%p)\r\n", c)
    : SUCCESS;
}

bool
state_load_struct_damage(struct damage* dmg)
{
  return state_load_int32(&dmg->sides) || state_load_int32(&dmg->dices)
    ? io_fail("state_load_coord(%p)\r\n", dmg)
    : SUCCESS;
}

bool
state_load_structs_damage(struct damage dmg[MAXATTACKS])
{
  for (int i = 0; i < MAXATTACKS; ++i)
    if (state_load_struct_damage(&dmg[i]))
      return io_fail("state_load_structs_damage(%p)\r\n", dmg);
  return SUCCESS;
}

bool
state_load_index(char const** master, int max, char const** str)
{
  int i;

  assert(file != NULL);

  if (state_load_int32(&i) || i > max)
    return 1;

  *str = i >= 0 ? master[i] : NULL;
  return 0;
}

static int
rs_read_window(WINDOW* win)
{
  int maxlines;
  int maxcols;
  int width  = getmaxx(win);
  int height = getmaxy(win);
  int value = 0;

  if (state_assert_int32(RSID_WINDOW) ||
      state_load_int32(&maxlines) ||
      state_load_int32(&maxcols))
    return io_fail("rs_read_window(%p)\r\n", win);

  for (int row = 0; row < maxlines; row++)
    for (int col = 0; col < maxcols; col++)
    {
      if (state_load_int32(&value) != 0)
        return 1;
      else if ((row < height) && (col < width))
        mvwaddcch(win, row, col, (chtype)value);
    }

  return 0;
}

static item*
get_list_item(item* l, int i)
{
    for (int count = 0; l != NULL; count++, l = &l->l_next->o)
        if (count == i)
            return l;
    return NULL;
}

static monster*
get_list_monster(monster* l, int i)
{
    for (int count = 0; l != NULL; count++, l = &l->l_next->t)
        if (count == i)
            return l;
    return NULL;
}

static int
rs_read_daemons(struct delayed_action* d_list, int count)
{
  int i = 0;
  int value = 0;

  if (state_assert_int32(RSID_DAEMONS) ||
      state_load_int32(&value))
    return io_fail("rs_read_daemons(%p, %d)\r\n", d_list, count);

  if (value > count)
    return 1;

  for(i = 0; i < count; i++)
  {
    int func = 0;
    if (state_load_int32(&d_list[i].d_type) ||
        state_load_int32(&func) ||
        state_load_int32(&d_list[i].d_arg) ||
        state_load_int32(&d_list[i].d_time))
      return 1;

    switch(func)
    {
      case 0:    d_list[i].d_func = NULL; break;
      case 1:    d_list[i].d_func = daemon_rollwand; break;
      case 2:    d_list[i].d_func = daemon_doctor; break;
      case 3:    d_list[i].d_func = daemon_runners_move; break;
      case 4:    d_list[i].d_func = daemon_start_wanderer; break;
      case 5:    d_list[i].d_func = player_decrease_speed; break;
      case 6:    d_list[i].d_func = player_remove_confused; break;
      case 7:    d_list[i].d_func = player_remove_true_sight; break;
      case 8:    d_list[i].d_func = player_remove_blind; break;
      case 9:    d_list[i].d_func = daemon_ring_abilities; break;
      case 10:   d_list[i].d_func = player_remove_sense_monsters; break;
      case 11:   d_list[i].d_func = player_remove_hallucinating; break;
      case 12:   d_list[i].d_func = player_stop_levitating; break;
      case 13:   d_list[i].d_func = daemon_change_visuals; break;
      default:   io_msg("Unknown daemon by id %d", func); return 1;
    }
  }

  if (d_list[i].d_func == NULL)
  {
    d_list[i].d_type = 0;
    d_list[i].d_arg = 0;
    d_list[i].d_time = 0;
  }
  return 0;
}


bool
state_load_obj_info(struct obj_info* mi, int count)
{
  int value;

  if (state_assert_int32(RSID_MAGICITEMS) ||
      state_load_int32(&value) ||
      value > count)
    return io_fail("state_load_obj_info(%p, %d)\r\n", mi, count);

  for (int n = 0; n < value; n++)
    if (state_load_int32(&mi[n].oi_prob) ||
        state_load_int32(&mi[n].oi_worth) ||
        state_load_string(&mi[n].oi_guess) ||
        state_load_bool(&mi[n].oi_know))
      return 1;
  return 0;
}

static int
rs_read_rooms(struct room* r, int count)
{
  int value = 0;

  if (state_load_int32(&value) || value > count)
    return 1;

  for (int n = 0; n < value; n++)
    if (state_load_room(&r[n]))
      return 1;
  return 0;
}

static int
rs_read_room_reference(struct room** rp)
{
  int i;
  if (state_load_int32(&i))
    return 1;
  *rp = &rooms[i];
  return 0;
}

static int
rs_read_item(item* item)
{
  assert(item != NULL);

  if (state_assert_int32(RSID_ITEM) ||
      state_load_int32(&item->o_type) ||
      state_load_coord(&item->o_pos) ||
      state_load_int32(&item->o_launch) ||
      state_load_char(&item->o_packch) ||
      state_load_struct_damage(&item->o_damage) ||
      state_load_struct_damage(&item->o_hurldmg) ||
      state_load_int32(&item->o_count) ||
      state_load_int32(&item->o_which) ||
      state_load_int32(&item->o_hplus) ||
      state_load_int32(&item->o_dplus) ||
      state_load_int32(&item->o_arm) ||
      state_load_int32(&item->o_flags) ||
      state_load_string(&item->o_label))
    return io_fail("rs_read_item(%p)\r\n", item);
  return 0;
}

static int
rs_read_equipment(int32_t marker)
{
  int32_t disk_mark;
  THING* item;

  if (state_load_int32(&disk_mark))
    return io_fail("rs_read_equipment(%X)\r\n", marker);

  if (disk_mark == RSID_NULL)
    return 0;
  if (disk_mark != marker)
    return io_fail("rs_read_equipment(%X) expected %X found %x\r\n",
                marker, disk_mark);

  item = os_calloc_thing();
  if (rs_read_item(&item->o))
    return io_fail("rs_read_equipment(%X)\r\n", marker);

  pack_equip_item(item);
  return 0;
}


bool
state_load_item_list(THING** list)
{
  int cnt;

  if (state_assert_int32(RSID_ITEMLIST) ||
      state_load_int32(&cnt))
    return io_fail("state_load_list(%p[%p])\r\n", list, *list);

  THING* l = NULL;
  THING* previous = NULL;
  THING* head = NULL;
  for (int i = 0; i < cnt; i++)
  {
    l = os_calloc_thing();
    l->o.l_prev = previous;

    if (previous != NULL)
      previous->o.l_next = l;

    if (rs_read_item(&l->o))
      return io_fail("state_load_list(list: %p[%p]) %d, i=%d\r\n",
                  list, *list, __LINE__, i);

    if (previous == NULL)
      head = l;

    previous = l;
  }

  if (l != NULL)
    l->o.l_next = NULL;

  *list = head;

  return 0;
}

bool
state_load_monster(THING* t)
{
  int32_t listid = 0;
  int32_t index = -1;

  assert(t != NULL);

  if (state_assert_int32(RSID_MONSTER) ||
      state_load_int32(&index))
    return io_fail("state_load_thing(%p)\r\n", t);

  if (index == 0)
    return 0;

  if (state_load_coord(&t->t.t_pos) ||
      state_load_bool(&t->t.t_turn) ||
      state_load_char(&t->t.t_type) ||
      state_load_char(&t->t.t_disguise) ||
      state_load_char(&t->t.t_oldch))
    return 1;

  /*
     t_dest can be (listid,index):
     0,0: NULL
     0,1: location of hero
     1,i: location of a thing (monster)
     2,i: location of an object
     3,i: location of gold in a room

     We need to remember what we are chasing rather than 
     the current location of what we are chasing.
  */

  if (state_load_int32(&listid) ||
      state_load_int32(&index))
    return 1;
  t->t.t_reserved = -1;

  switch(listid)
  {
    case 0: /* hero or NULL */
      t->t.t_dest = index == 1
        ? player_get_pos()
        : NULL;
      break;

    case 1: /* monster / thing */
      t->t.t_dest     = NULL;
      t->t.t_reserved = index;
      break;

    case 2: /* object */
      {
        item* item = get_list_item(&level_items->o, index);
        if (item != NULL)
          t->t.t_dest = &item->o_pos;
      }
      break;

    case 3: /* gold */
      t->t.t_dest = &rooms[index].r_gold;
      break;

    default:
      t->t.t_dest = NULL;
      break;
  }


  if (state_load_int32(&t->t.t_flags) ||
      state_load_stats(&t->t.t_stats) ||
      rs_read_room_reference( &t->t.t_room) ||
      state_load_item_list(&t->t.t_pack))
    return 1;
  return 0;
}

static int
rs_fix_thing(THING* t)
{
  if (t->t.t_reserved < 0)
    return 0;

  monster* monster = get_list_monster(&monster_list->t, t->t.t_reserved);

  if (monster != NULL)
    t->t.t_dest = &monster->t_pos;
  return 0;
}

static int
rs_read_thing_list(THING** list)
{
  int cnt;
  THING* l = NULL;
  THING* previous = NULL;
  THING* head = NULL;

  if (state_assert_int32(RSID_MONSTERLIST) ||
      state_load_int32(&cnt))
    return io_fail("rs_read_thing_list(%p[%p])\r\n", list, *list);

  for (int i = 0; i < cnt; i++)
  {
    l = os_calloc_thing();
    l->t.l_prev = previous;

    if (previous != NULL)
      previous->t.l_next = l;

    if (state_load_monster(l))
      return 1;

    if (previous == NULL)
      head = l;
    previous = l;
  }

  if (l != NULL)
    l->t.l_next = NULL;

  *list = head;

  return 0;
}

static int
rs_fix_thing_list(THING* list)
{
  for(THING* item = list; item != NULL; item = item->t.l_next)
    rs_fix_thing(item);
  return 0;
}

static int
rs_read_thing_reference(THING* list, THING** item)
{
  int i;

  if (state_load_int32(&i))
    return 1;

  if (i != -1)
  {
    monster* mon = get_list_monster(&list->t, i);
    *item = mon ? os_monster_to_thing(&mon) : NULL;
  }
  else
  {
    *item = NULL;
  }
  return 0;
}

static bool
state_load_places(PLACE* place, int num_places)
{
  for (int i = 0; i < num_places; ++i)
    if (state_load_char(&place[i].p_ch) ||
        state_load_char(&place[i].p_flags) ||
        rs_read_thing_reference( monster_list, &place[i].p_monst))
      return io_fail("state_load_places(%p, %d) Failed to load PLACE\r\n",
                     place, num_places);
  return SUCCESS;
}

bool
state_load_file(FILE* inf)
{
  assert(inf != NULL);
  assert(file == NULL);

  file = inf;

  rs_assert(state_assert_int32(RSID_START))

  rs_assert(state_assert_int32(RSID_PACK)    ||   pack_load_state());
  rs_assert(state_assert_int32(RSID_POTIONS) || potion_load_state());
  rs_assert(state_assert_int32(RSID_RINGS)   ||   ring_load_state());
  rs_assert(state_assert_int32(RSID_SCROLLS) || scroll_load_state());
  rs_assert(state_assert_int32(RSID_WANDS)   ||   wand_load_state());
  rs_assert(state_assert_int32(RSID_PLAYER)  || player_load_state());
  rs_assert(state_assert_int32(RSID_LEVEL)   ||  level_load_state());
  rs_assert(state_assert_int32(RSID_FOOD)    ||   food_load_state());

  rs_assert(state_load_bool(&firstmove))
  rs_assert(state_load_chars(save_file_name, MAXSTR))
  rs_assert(state_load_char(&runch))
  rs_assert(state_load_chars(whoami, MAXSTR))
  rs_assert(state_load_int32(&levels_without_food))
  rs_assert(state_load_int32(&player_turns_without_moving))
  rs_assert(state_load_int32(&pack_gold))
  rs_assert(state_load_int32(&monster_flytrap_hit))
  rs_assert(state_load_int32((int32_t*) &os_rand_seed))

  rs_assert(rs_read_equipment(RSID_ARMOR))
  rs_assert(rs_read_equipment(RSID_RHAND))
  rs_assert(rs_read_equipment(RSID_RRING))
  rs_assert(rs_read_equipment(RSID_LRING))

  rs_assert(rs_read_thing_list(&monster_list))
  rs_assert(rs_fix_thing(__player_ptr()))
  rs_assert(weapons_load_state());
  rs_assert(rs_fix_thing_list(monster_list))
  rs_assert(state_load_places(level_places, MAXLINES*MAXCOLS))
  rs_assert(state_load_stats(&player_max_stats))
  rs_assert(rs_read_rooms(rooms, ROOMS_MAX))
  rs_assert(rs_read_room_reference(&room_prev))
  rs_assert(rs_read_rooms(passages, PASSAGES_MAX))
  rs_assert(state_load_obj_info(potion_info,  NPOTIONS))
  rs_assert(state_load_obj_info(ring_info,  NRINGS))
  rs_assert(rs_read_daemons(__daemons_ptr(), 20))
  rs_assert(rs_read_window(stdscr))

  rs_assert(state_assert_int32(RSID_END))

  file = NULL;

  return 0;
}
