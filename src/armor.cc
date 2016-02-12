#include <string>
#include <vector>
#include <sstream>

#include "disk.h"
#include "error_handling.h"
#include "io.h"
#include "item.h"
#include "misc.h"
#include "options.h"
#include "os.h"
#include "player.h"
#include "rings.h"
#include "rogue.h"

#include "armor.h"

using namespace std;

Armor::~Armor() {}

bool Armor::is_magic() const {
  return ((o_flags & ISPROT) ||
      get_armor() != Armor::ac(subtype));
}

class Armor* Armor::clone() const {
  return new Armor(*this);
}

static Armor::Type random_armor_type() {
  int value = os_rand_range(100);

  int end = static_cast<int>(Armor::Type::NARMORS);
  for (int i = 0; i < end; ++i) {
    Armor::Type type = static_cast<Armor::Type>(i);
    int probability = Armor::probability(type);

    if (value < probability) {
      return type;

    } else {
      value -= probability;
    }
  }

  error("Error! Sum of probabilities is not 100%");
}

Armor::Armor(bool random_stats) :
  Armor(random_armor_type(), random_stats)
{}

Armor::Armor(Armor::Type type, bool random_stats) :
  Item(), subtype(type), identified(false) {
  o_type = IO::Armor;
  o_which = type;
  set_armor(Armor::ac(type));

  if (random_stats) {
    int rand = os_rand_range(100);
    if (rand < 20) {
      set_cursed();
      modify_armor(os_rand_range(3) + 1);
    }
    else if (rand < 28) {
      modify_armor(-os_rand_range(3) + 1);
    }
  }
}

void Armor::set_identified() {
  identified = true;
}

bool Armor::is_identified() const {
  return identified;
}

int Armor::probability(Armor::Type type) {
  switch (type) {
    case LEATHER:         return 20;
    case RING_MAIL:       return 15;
    case STUDDED_LEATHER: return 15;
    case SCALE_MAIL:      return 13;
    case CHAIN_MAIL:      return 12;
    case SPLINT_MAIL:     return 10;
    case BANDED_MAIL:     return 10;
    case PLATE_MAIL:      return  5;
    case NARMORS:         error("Unknown type NARMORS");
  }
}

string Armor::name(Armor::Type type) {
  switch (type) {
    case LEATHER:         return "leather armor";
    case RING_MAIL:       return "ring mail";
    case STUDDED_LEATHER: return "studded leather armor";
    case SCALE_MAIL:      return "scale mail";
    case CHAIN_MAIL:      return "chain mail";
    case SPLINT_MAIL:     return "splint mail";
    case BANDED_MAIL:     return "banded mail";
    case PLATE_MAIL:      return "plate mail";
    case NARMORS:         error("Unknown type NARMORS");
  }
}

int Armor::value(Armor::Type type) {
  switch (type) {
    case LEATHER:         return 20;
    case RING_MAIL:       return 25;
    case STUDDED_LEATHER: return 20;
    case SCALE_MAIL:      return 30;
    case CHAIN_MAIL:      return 75;
    case SPLINT_MAIL:     return 80;
    case BANDED_MAIL:     return 90;
    case PLATE_MAIL:      return 150;
    case NARMORS:         error("Unknown type NARMORS");
  }
}

int Armor::ac(Armor::Type type) {
  switch (type) {
    case LEATHER:         return 8;
    case RING_MAIL:       return 7;
    case STUDDED_LEATHER: return 7;
    case SCALE_MAIL:      return 6;
    case CHAIN_MAIL:      return 5;
    case SPLINT_MAIL:     return 4;
    case BANDED_MAIL:     return 4;
    case PLATE_MAIL:      return 3;
    case NARMORS:         error("Unknown type NARMORS");
  }
}

string Armor::get_description() const {
  stringstream buffer;

  string const& obj_name = Armor::name(static_cast<Armor::Type>(o_which));
  int bonus_ac = Armor::ac(static_cast<Armor::Type>(o_which)) - get_armor();
  int base_ac = 10 - get_armor() - bonus_ac;

  buffer
    << "a"
    << vowelstr(obj_name)
    << " "
    << obj_name
    << " ["
    << base_ac;

  if (identified) {
    buffer << ",";
    if (bonus_ac > 0) {
      buffer << "+";
    }
    buffer << bonus_ac;
  }

  buffer << "]";

  if (!get_nickname().empty()) {
    buffer << " {" << get_nickname() << "}";
  }

  return buffer.str();
}

void Armor::save(std::ofstream& data) const {
  Item::save(data);
  static_assert(sizeof(Armor::Type) == sizeof(int), "Wrong Armor::Type size");
  Disk::save(TAG_ARMOR, static_cast<int>(subtype), data);
  Disk::save(TAG_ARMOR, identified, data);
}

bool Armor::load(std::ifstream& data) {
  if (!Item::load(data) ||
      !Disk::load(TAG_ARMOR, reinterpret_cast<int&>(subtype), data) ||
      !Disk::load(TAG_ARMOR, identified, data)) {
    return false;
  }
  return true;
}


