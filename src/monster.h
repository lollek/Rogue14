#pragma once

#include <string>
#include <list>
#include <vector>

#include "character.h"
#include "level_rooms.h"
#include "Coordinate.h"
#include "item.h"
#include "rogue.h"

struct monster_template {
    std::string const    m_name;    // What to call the monster
    int                  m_carry;   // Probability of carrying something
    int                  m_flags;   // things about the monster
    int                  m_basexp;  // Base xp
    int                  m_level;   // Level
    int                  m_armor;   // Armor
    std::vector<damage>  m_dmg;     // Monster attacks
};

class Monster : public Character {
public:
  Monster(char type, Coordinate const& pos, room* room);
  Monster(Monster const&) = delete; // Deleted since they would share inventory

  virtual ~Monster() = default;

  Monster& operator=(Monster const&) = delete; // Deleted since they would share inventory
  Monster& operator=(Monster&&) = default;

  // Setters
  void set_oldch(Coordinate &coord);
  void set_invisible() override;

  // Getters
  int get_armor() const override;
  std::string get_attack_string(bool successful_hit) const override;
  std::string get_name() const override;

  // Statics
  static char random_monster_type();

  // Variables (TODO: Make these private)
  Coordinate         t_dest;    // Where it is running to
  std::list<Item*>   t_pack;    // What the thing is carrying

  char               t_disguise;// What mimic looks like
  char               t_oldch;   // Character that was where it was
  bool               t_turn;    // If slowed, is it a turn to move

private:
  Monster(char type, Coordinate const& pos, room* room,
          monster_template const& m_template);
};





/* Variables, TODO: Remove these */
extern int    monster_flytrap_hit;


/* What to do when the hero steps next to a monster */
Monster *monster_notice_player(int y, int x);

/* Give a pack to a monster if it deserves one */
void monster_give_pack(Monster* mon);

/* See if a creature save against something */
int monster_save_throw(int which, Monster const* mon);

/* Make monster start running (towards hero?) */
void monster_start_running(Coordinate const* runner);

/* Called to put a monster to death */
void monster_on_death(Monster* tp, bool pr);

/* Remove a monster from the screen */
void monster_remove_from_screen(Coordinate const* mp, Monster* tp, bool waskill);

bool monster_is_dead(Monster const* monster);

void monster_teleport(Monster* monster, Coordinate const* destination);

void monster_do_special_ability(Monster** monster);

std::string const& monster_name_by_type(char monster_type);
bool monster_seen_by_player(Monster const* monster);

/* Is any monster seen by the player? */
bool monster_is_anyone_seen_by_player(void);
/* Change all monster visuals due to player tripping */
void monster_show_all_as_trippy(void);
/* Make all monsters take their turn */
void monster_move_all(void);
/* Make all monsters start chasing the player */
void monster_aggravate_all(void);
/* Show all monsters as they truly are */
void monster_show_all_hidden(void);
/* Does any monster desire this item? If so, aggro player */
void monster_aggro_all_which_desire_item(Item* item);
/* Hide all invisible monsters */
void monster_hide_all_invisible(void);
/* Show all monsters that the player does not currently sees
 * Return true if there was atleast one, else false */
bool monster_sense_all_hidden(void);
void monster_unsense_all_hidden(void);
/* Print all monsters as they look (or hide, is in disguise) */
void monster_print_all(void);
/* Print a $ where there is a monster with a magic item
 * Returns true if there was atleast one, else false */
bool monster_show_if_magic_inventory(void);

/* Add nearby monsters to the given list. Returns the number of results */
int monster_add_nearby(Monster** nearby_monsters, struct room const* room);

/* Transform the monster into something else */
void monster_polymorph(Monster* monster);

/** monster_chase.c **/
bool monster_chase(Monster* tp); /* Make a monster chase */
