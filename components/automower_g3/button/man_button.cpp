#include "man_button.h"

namespace esphome {
namespace automower {

void ManButton::press_action() { this->parent_->set_mode(MODE_MAN); }

}  // namespace automower
}  // namespace esphome
