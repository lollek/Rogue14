#pragma once

#include <string>

#include "damage.h"
#include "Coordinate.h"

#define o_charges	o_arm

// flags for objects
#define ISCURSED 000001		/* object is cursed */
#define ISKNOW	0000002		/* player knows details about the object */
#define ISMISL	0000004		/* object is a missile type */
#define ISMANY	0000010		/* object comes in groups */
#ifndef ISFOUND
#define ISFOUND 0000020		/*...is used for both objects and creatures */
#endif /* ISFOUND */
#define ISPROT	0000040		/* armor is permanently protected */


class Item {
public:
  Item(Item const&) = default;

  virtual ~Item();

  Item& operator=(Item const&) = default;
  Item& operator=(Item&&) = default;
  virtual Item* clone() const = 0;

  // Setters
  void set_position(Coordinate const&);
  void set_x(int);
  void set_y(int);
  void set_nickname(std::string const&);
  void set_attack_damage(damage const&);
  void set_throw_damage(damage const&);
  void set_hit_plus(int amount);
  void set_damage_plus(int amount);
  void set_armor(int amount);

  // Getters (virtual)
  virtual std::string   get_description() const = 0;
  virtual bool          is_magic() const = 0;

  // Modifiers
  void modify_hit_plus(int amount);
  void modify_damage_plus(int amount);
  void modify_armor(int amount);

  // Getters
  Coordinate const&     get_position() const;
  int                   get_x() const;
  int                   get_y() const;
  std::string const&    get_nickname() const;
  int                   get_type() const;
  damage const&         get_attack_damage() const;
  damage const&         get_throw_damage() const;
  int                   get_hit_plus() const;
  int                   get_damage_plus() const;
  int                   get_armor() const;

  int           o_type;                // What kind of object it is
  int           o_launch;              // What you need to launch it
  int           o_count;               // count for plural objects
  int           o_which;               // Which object of a type it is
  int           o_flags;               // information about objects
  char          o_packch;              // What character it is in the pack

protected:
  Item() = default;

private:
  Coordinate    position_on_screen;
  std::string   nickname;              // TODO: See if we can remove this
  damage        attack_damage;         // Damage if used like sword
  damage        throw_damage;          // Damage if thrown
  int           hit_plus;              // Plusses to hit
  int           damage_plus;           // Plusses to damage
  int           armor;                 // Armor protection
};

