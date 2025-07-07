#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/fan/fan.h"
#include "esphome/components/fan/fan_state.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
namespace esphome {
namespace ifan {

class IFan : public Component, public fan::Fan , public uart::UARTDevice {
 public:
#define TAG "IFAN"

  IFan() {}
  void setup() override;
  void dump_config() override;
  fan::FanTraits get_traits() override;
  void set_buzzer_enable(bool buzzer_enable) { this->buzzer_enable_ = buzzer_enable; }
  void set_remote_enable(bool remote_enable) { this->remote_enable_ = remote_enable;};
  bool state_;

 protected:
  void control(const fan::FanCall &call) override;
  void write_state_();
  void set_off();
  void set_low();
  void set_med();
  void set_high();
  void do_speed(int lspeed);
  void beep(int num=1);
  void long_beep(int num=1);
  int speed_count_{};
  int current_speed=0;
  bool buzzer_enable_;
  bool remote_enable_;
  // For Remote
public:
  void loop() override;
  void output(const float speed);
 protected:
  void handle_char_(uint8_t c);
  void handle_command_(uint8_t type, uint8_t param);

};
template<typename... Ts> class CycleSpeedAction : public Action<Ts...> {
 public:
  explicit CycleSpeedAction(IFan *state) : state_(state) {}

  void play(Ts... x) override {
    // check to see if fan supports speeds and is on
    if (this->state_->get_traits().supported_speed_count()) {
      if (this->state_->state) {
        int speed = this->state_->speed + 33;
        int supported_speed_count = this->state_->get_traits().supported_speed_count();
        if (speed > supported_speed_count) {
          // was running at max speed, so turn off
          speed = 1;
          auto call = this->state_->turn_off();
          call.set_speed(speed);
          call.perform();
        } else {
          auto call = this->state_->turn_on();
          call.set_speed(speed);
          call.perform();
        }
      } else {
        // fan was off, so set speed to 1
        auto call = this->state_->turn_on();
        call.set_speed(1);
        call.perform();
      }
    } else {
      // fan doesn't support speed counts, so toggle
      this->state_->toggle().perform();
    }
  }

  IFan *state_;
};
}  // namespace speed
}  // namespace esphome
