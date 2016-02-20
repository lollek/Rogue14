#include "disk.h"
#include "os.h"
#include "misc.h"
#include "game.h"
#include "scrolls.h"

#include "character.h"

using namespace std;

Character::Character(int str_, int dex_, int con_, int wis_, int int_, int cha_,
    int exp_, int lvl_, int ac_, int hd_, std::vector<damage> const& attacks_,
    Coordinate const& position_, unsigned long long flags, char type_,
    int speed_) :

  strength{str_}, default_strength{str_},
  dexterity{dex_}, default_dexterity{dex_},
  constitution{con_}, default_constitution{con_},
  wisdom{con_}, default_wisdom{wis_},
  intelligence{int_}, default_intelligence{int_},
  charisma{cha_}, default_charisma{cha_},

  experience{exp_}, level{lvl_}, base_ac{ac_}, hit_dice{hd_},
  base_health{hit_dice + roll(level-1, hit_dice)},
  health{get_max_health()},
  attacks{attacks_}, position{position_}, type{type_}, speed{speed_}, turns_not_moved{0},

  confusing_attack{0}, true_sight{0}, blind{0}, cancelled{0}, levitating{0},
  found{0}, greedy{0}, players_target{0}, held{0}, confused{0},
  invisible{0}, mean{0}, regenerating{0}, running{0},
  flying{0}, stuck{0}, attack_freeze{0}, attack_damage_armor{0},
  attack_steal_gold{0}, attack_steal_item{0}, attack_drain_strength{0},
  attack_drain_health{0}, attack_drain_experience{0}
{
  if (flags & 010000000000000000000) { greedy = true; }
  if (flags & 020000000000000000000) { mean = true; }
  if (flags & 040000000000000000000) { flying = true; }
  if (flags & 001000000000000000000) { regenerating = true; }
  if (flags & 002000000000000000000) { invisible = true; }
  if (flags & 004000000000000000000) { attack_freeze = true; }
  if (flags & 000100000000000000000) { attack_damage_armor = true; }
  if (flags & 000200000000000000000) { attack_steal_gold = true; }
  if (flags & 000400000000000000000) { attack_steal_item = true; }
  if (flags & 000010000000000000000) { attack_drain_strength = true; }
  if (flags & 000020000000000000000) { attack_drain_health = true; }
  if (flags & 000040000000000000000) { attack_drain_experience = true; }
}

int  Character::get_speed() const { return speed; }
bool Character::is_blind() const { return blind; }
bool Character::is_cancelled() const { return cancelled; }
bool Character::is_confused() const { return confused; }
bool Character::has_confusing_attack() const { return confusing_attack; }
bool Character::is_found() const { return found; }
bool Character::is_invisible() const { return invisible; }
bool Character::is_levitating() const { return levitating; }
bool Character::has_true_sight() const { return true_sight; }
bool Character::is_held() const { return held; }
bool Character::is_stuck() const { return stuck; }
bool Character::is_chasing() const { return running; }
bool Character::is_running() const { return running; }
bool Character::is_mean() const { return mean; }
bool Character::is_greedy() const { return greedy; }
bool Character::is_players_target() const { return players_target; }
bool Character::is_flying() const { return flying; }
bool Character::attack_freezes() const { return attack_freeze; }
bool Character::attack_damages_armor() const { return attack_damage_armor; }
bool Character::attack_steals_gold() const { return attack_steal_gold; }
bool Character::attack_steals_item() const { return attack_steal_item; }
bool Character::attack_drains_strength() const { return attack_drain_strength; }
bool Character::attack_drains_health() const { return attack_drain_health; }
bool Character::attack_drains_experience() const { return attack_drain_experience; }

void Character::set_blind() { blind = true; }
void Character::set_cancelled() { cancelled = true; }
void Character::set_confused() { confused = true; }
void Character::set_confusing_attack() { confusing_attack = true; }
void Character::set_found() { found = true; }
void Character::set_levitating() { levitating = true; }
void Character::set_true_sight() { true_sight = true; }
void Character::set_stuck() { stuck = true; }
void Character::set_not_blind() { blind = false; }
void Character::set_not_cancelled() { cancelled = false; }
void Character::set_not_confused() { confused = false; }
void Character::remove_confusing_attack() { confusing_attack = false; }
void Character::set_not_found() { found = false; }
void Character::set_not_invisible() { invisible = false; }
void Character::set_not_levitating() { levitating = false; }
void Character::remove_true_sight() { true_sight = false; }
void Character::set_not_held() { held = false; }

void Character::set_held() {
  held = true;
  running = false;
}

void Character::take_damage(int damage) {
  health -= damage;
}

Coordinate Character::possible_random_move() {
  Coordinate ret;

  // Generate a random
  int x = ret.x = position.x + os_rand_range(3) - 1;
  int y = ret.y = position.y + os_rand_range(3) - 1;

  if (y == position.y && x == position.x) {
    return ret;
  }

  if (!Game::level->can_step(x, y)) {
    ret.x = position.x;
    ret.y = position.y;
    return ret;
  }

  Item* item = Game::level->get_item(x, y);
  if (item != nullptr && item->o_type == IO::Scroll && item->o_which == Scroll::SCARE) {
    ret.x = position.x;
    ret.y = position.y;
    return ret;
  }

  return ret;
}

void Character::restore_strength() {
  strength = default_strength;
}

void Character::modify_strength(int amount) {
  // Negative is temporary and Positive is permanent (if not below default).
  strength += amount;

  if (strength > default_strength) {
    default_strength = strength;
  }
}

int Character::get_default_strength() const {
  return default_strength;
}

void Character::restore_health(int amount, bool can_raise_max_health) {
  health += amount;

  if (can_raise_max_health)
  {
    int extra_max_hp = 0;
    if (health > get_max_health() + level + 1)
      ++extra_max_hp;
    if (health > get_max_health())
      ++extra_max_hp;

    base_health += extra_max_hp;
  }

  if (health > get_max_health())
    health = get_max_health();
}

bool Character::is_hurt() const {
  return health != get_max_health();
}

void Character::modify_max_health(int amount) {
  base_health += amount;
  health += amount;
}

void Character::raise_level(int amount) {
  experience = 0;
  level += amount;

  // Roll extra HP
  modify_max_health(roll(amount, hit_dice));
}

void Character::lower_level(int amount) {
  level = max(1, level - amount);

  modify_max_health(-roll(amount, hit_dice));
}

int Character::get_level() const {
  return level;
}

int Character::get_ac() const {
  return base_ac + (dexterity - 10) / 2;
}

int Character::get_type() const {
  return type;
}

int Character::get_experience() const {
  return experience;
}

int Character::get_strength() const {
  return strength;
}

int Character::get_health() const {
  return health;
}

int Character::get_max_health() const {
  return base_health + (constitution - 10) / 2;
}

Coordinate const& Character::get_position() const {
  return position;
}

std::vector<damage> const& Character::get_attacks() const {
  return attacks;
}

void Character::set_mean() { mean = true; }
void Character::set_players_target() { players_target = true; }
void Character::set_not_players_target() { players_target = false; }
void Character::gain_experience(int experience_) {
  experience += experience_;
}

void Character::set_not_mean() { mean = false; }
void Character::set_greedy() { greedy = true; }
void Character::set_not_greedy() { greedy = false; }
void Character::set_flying() { flying = true; }
void Character::set_not_flying() { flying = false; }
void Character::set_running() { running = true; }
void Character::set_not_running() { running = false; }
void Character::set_chasing() { running = true; }
void Character::set_not_chasing() { running = false; }
void Character::set_not_stuck() { stuck = false; }
void Character::set_invisible() { invisible = true; }
void Character::set_position(Coordinate const& position_) {
  position = position_;
}


void Character::save(ofstream& data) const {
  Disk::save_tag(TAG_CHARACTER, data);
  Disk::save(TAG_CHARACTER, strength, data);
  Disk::save(TAG_CHARACTER, default_strength, data);
  Disk::save(TAG_CHARACTER, dexterity, data);
  Disk::save(TAG_CHARACTER, default_dexterity, data);
  Disk::save(TAG_CHARACTER, constitution, data);
  Disk::save(TAG_CHARACTER, default_constitution, data);
  Disk::save(TAG_CHARACTER, wisdom, data);
  Disk::save(TAG_CHARACTER, default_wisdom, data);
  Disk::save(TAG_CHARACTER, intelligence, data);
  Disk::save(TAG_CHARACTER, default_intelligence, data);
  Disk::save(TAG_CHARACTER, charisma, data);
  Disk::save(TAG_CHARACTER, default_charisma, data);

  Disk::save(TAG_CHARACTER, experience, data);
  Disk::save(TAG_CHARACTER, level, data);
  Disk::save(TAG_CHARACTER, base_ac, data);
  Disk::save(TAG_CHARACTER, hit_dice, data);
  Disk::save(TAG_CHARACTER, base_health, data);
  Disk::save(TAG_CHARACTER, health, data);
  Disk::save(TAG_CHARACTER, attacks, data);
  Disk::save(TAG_CHARACTER, position, data);
  Disk::save(TAG_CHARACTER, type, data);
  Disk::save(TAG_CHARACTER, speed, data);
  Disk::save(TAG_CHARACTER, turns_not_moved, data);

  Disk::save(TAG_CHARACTER, confusing_attack, data);
  Disk::save(TAG_CHARACTER, true_sight, data);
  Disk::save(TAG_CHARACTER, blind, data);
  Disk::save(TAG_CHARACTER, cancelled, data);
  Disk::save(TAG_CHARACTER, levitating, data);
  Disk::save(TAG_CHARACTER, found, data);
  Disk::save(TAG_CHARACTER, greedy, data);
  Disk::save(TAG_CHARACTER, players_target, data);
  Disk::save(TAG_CHARACTER, held, data);
  Disk::save(TAG_CHARACTER, confused, data);
  Disk::save(TAG_CHARACTER, invisible, data);
  Disk::save(TAG_CHARACTER, mean, data);
  Disk::save(TAG_CHARACTER, regenerating, data);
  Disk::save(TAG_CHARACTER, running, data);
  Disk::save(TAG_CHARACTER, flying, data);
  Disk::save(TAG_CHARACTER, stuck, data);
  Disk::save(TAG_CHARACTER, attack_freeze, data);
  Disk::save(TAG_CHARACTER, attack_damage_armor, data);
  Disk::save(TAG_CHARACTER, attack_steal_gold, data);
  Disk::save(TAG_CHARACTER, attack_steal_item, data);
  Disk::save(TAG_CHARACTER, attack_drain_strength, data);
  Disk::save(TAG_CHARACTER, attack_drain_health, data);
  Disk::save(TAG_CHARACTER, attack_drain_experience, data);
}

bool Character::load(ifstream& data) {
  if (!Disk::load_tag(TAG_CHARACTER, data) ||
      !Disk::load(TAG_CHARACTER, strength, data) ||
      !Disk::load(TAG_CHARACTER, default_strength, data) ||
      !Disk::load(TAG_CHARACTER, dexterity, data) ||
      !Disk::load(TAG_CHARACTER, default_dexterity, data) ||
      !Disk::load(TAG_CHARACTER, constitution, data) ||
      !Disk::load(TAG_CHARACTER, default_constitution, data) ||
      !Disk::load(TAG_CHARACTER, wisdom, data) ||
      !Disk::load(TAG_CHARACTER, default_wisdom, data) ||
      !Disk::load(TAG_CHARACTER, intelligence, data) ||
      !Disk::load(TAG_CHARACTER, default_intelligence, data) ||
      !Disk::load(TAG_CHARACTER, charisma, data) ||
      !Disk::load(TAG_CHARACTER, default_charisma, data) ||

      !Disk::load(TAG_CHARACTER, experience, data) ||
      !Disk::load(TAG_CHARACTER, level, data) ||
      !Disk::load(TAG_CHARACTER, base_ac, data) ||
      !Disk::load(TAG_CHARACTER, hit_dice, data) ||
      !Disk::load(TAG_CHARACTER, base_health, data) ||
      !Disk::load(TAG_CHARACTER, health, data) ||
      !Disk::load(TAG_CHARACTER, attacks, data) ||
      !Disk::load(TAG_CHARACTER, position, data) ||
      !Disk::load(TAG_CHARACTER, type, data) ||
      !Disk::load(TAG_CHARACTER, speed, data) ||
      !Disk::load(TAG_CHARACTER, turns_not_moved, data) ||

      !Disk::load(TAG_CHARACTER, confusing_attack, data) ||
      !Disk::load(TAG_CHARACTER, true_sight, data) ||
      !Disk::load(TAG_CHARACTER, blind, data) ||
      !Disk::load(TAG_CHARACTER, cancelled, data) ||
      !Disk::load(TAG_CHARACTER, levitating, data) ||
      !Disk::load(TAG_CHARACTER, found, data) ||
      !Disk::load(TAG_CHARACTER, greedy, data) ||
      !Disk::load(TAG_CHARACTER, players_target, data) ||
      !Disk::load(TAG_CHARACTER, held, data) ||
      !Disk::load(TAG_CHARACTER, confused, data) ||
      !Disk::load(TAG_CHARACTER, invisible, data) ||
      !Disk::load(TAG_CHARACTER, mean, data) ||
      !Disk::load(TAG_CHARACTER, regenerating, data) ||
      !Disk::load(TAG_CHARACTER, running, data) ||
      !Disk::load(TAG_CHARACTER, flying, data) ||
      !Disk::load(TAG_CHARACTER, stuck, data) ||
      !Disk::load(TAG_CHARACTER, attack_freeze, data) ||
      !Disk::load(TAG_CHARACTER, attack_damage_armor, data) ||
      !Disk::load(TAG_CHARACTER, attack_steal_gold, data) ||
      !Disk::load(TAG_CHARACTER, attack_steal_item, data) ||
      !Disk::load(TAG_CHARACTER, attack_drain_strength, data) ||
      !Disk::load(TAG_CHARACTER, attack_drain_health, data) ||
      !Disk::load(TAG_CHARACTER, attack_drain_experience, data)) {
    return false;
  }
  return true;
}

void Character::increase_speed() {
  if (++speed == 0) {
    speed = 1;
  }
  turns_not_moved = 0;
}

void Character::decrease_speed() {
  if (--speed) {
    speed = -1;
  }
  turns_not_moved = 0;
}


// If speed > 0, its the number of turns to take.
// Else it's a turn per 1 / (-speed +2) rounds. E.g. 0 -> 1/2, -1 -> 1/3
int Character::get_moves_this_round() {
  if (speed > 0) {
    return speed;
  }

  if (turns_not_moved > -speed) {
    turns_not_moved = 0;
    return 1;
  }

  turns_not_moved++;
  return 0;
}

int Character::get_dexterity() const {
  return dexterity;
}

int Character::get_default_dexterity() const {
  return default_dexterity;
}

int Character::get_constitution() const {
  return constitution;
}

int Character::get_default_constitution() const {
  return default_constitution;
}

int Character::get_wisdom() const {
  return wisdom;
}

int Character::get_default_wisdom() const {
  return default_wisdom;
}

int Character::get_intelligence() const {
  return intelligence;
}

int Character::get_default_intelligence() const {
  return default_intelligence;
}

int Character::get_charisma() const {
  return charisma;
}

int Character::get_default_charisma() const {
  return default_charisma;
}


