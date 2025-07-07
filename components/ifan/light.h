#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_call.h"
#include "esphome.h"

namespace esphome {
namespace ifan {

class IFanLight : public light::LightOutput {
 public:
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::ON_OFF});
    return traits;
  }
  void write_state(light::LightState *state) override {
    pinMode(9, 0x01);
    state->current_values_as_binary(&binary);

    if (binary)
      digitalWrite(9, LOW);
    else
      digitalWrite(9, HIGH);
  }
 //static void toggle(){
  // state_->toggle();
 //}
void setup_state(light::LightState *state) override {
    state_ = state;
  }
 protected:
 light::LightState *state_;
  bool binary;
};  // class
}  // namespace ifan
}  // namespace esphome
