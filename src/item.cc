#include <string>

using namespace std;

#include "io.h"
#include "armor.h"

#include "item.h"

Item::~Item() {}

void Item::set_position(Coordinate const& new_value) {
  position_on_screen = new_value;
}

void Item::set_x(int new_value) {
  position_on_screen.x = new_value;
}

void Item::set_y(int new_value) {
  position_on_screen.y = new_value;
}

void Item::set_nickname(std::string const& new_value) {
  nickname = new_value;
}



Coordinate const& Item::get_position() const {
  return position_on_screen;
}

int Item::get_x() const {
  return position_on_screen.x;
}

int Item::get_y() const {
  return position_on_screen.y;
}

string const& Item::get_nickname() const {
  return nickname;
}

int Item::get_type() const {
  return o_type;
}
