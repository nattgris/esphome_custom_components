#include "park_button.h"

namespace esphome {
namespace automower {

void ParkButton::press_action() { this->parent_->set_mode(MODE_PARK); }

}  // namespace automower
}  // namespace esphome
