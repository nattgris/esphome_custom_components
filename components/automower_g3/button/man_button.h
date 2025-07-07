#pragma once

#include "esphome/components/button/button.h"
#include "../automower_g3.h"

namespace esphome {
namespace automower {

class ManButton : public button::Button, public Parented<AutoMower> {
 public:
  ManButton() = default;

 protected:
  void press_action() override;
};

}  // namespace automower
}  // namespace esphome
