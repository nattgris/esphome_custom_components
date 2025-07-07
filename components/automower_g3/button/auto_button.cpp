#include "auto_button.h"

namespace esphome {
namespace automower {

void AutoButton::press_action() { this->parent_->set_mode(MODE_AUTO); }

}  // namespace automower
}  // namespace esphome
