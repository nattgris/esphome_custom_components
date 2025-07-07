#pragma once

#include "esphome/components/button/button.h"
#include "../automower_g3.h"

namespace esphome {
namespace automower {

class ParkButton : public button::Button, public Parented<AutoMower> {
 public:
  ParkButton() = default;

 protected:
  void press_action() override;
};

}  // namespace automower
}  // namespace esphome
