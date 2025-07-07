#pragma once

#include "esphome/components/button/button.h"
#include "../automower_g3.h"

namespace esphome {
namespace automower {

class AutoButton : public button::Button, public Parented<AutoMower> {
 public:
  AutoButton() = default;

 protected:
  void press_action() override;
};

}  // namespace automower
}  // namespace esphome
