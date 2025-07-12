#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#include "esphome/components/uart/uart.h"
#include "esphome/core/defines.h"

#include <map>
#include <vector>

namespace esphome {
namespace automower {

enum AutoMowerMode {
  MODE_PARK = 0x02,
  MODE_MAN = 0x03,
  MODE_AUTO = 0x04,
};

enum AutoMowerStatus {
  STATUS_PARKING = 0x01,
  STATUS_MOWING = 0x02,
  STATUS_RETURNING = 0x03,
  STATUS_CHARGING = 0x04,
  STATUS_SEARCHING = 0x05,
  STATUS_ERROR = 0x07,
};

static const std::map<uint8_t, std::string> STATUS_INT_TO_TEXT {
  {STATUS_PARKING, "docked"},
  {STATUS_MOWING, "mowing"},
  {STATUS_RETURNING, "returning"},
  {STATUS_CHARGING, "charging"},
  {STATUS_SEARCHING, "searching"},
  {STATUS_ERROR, "error"}
};

class AutoMower : public uart::UARTDevice, public Component {
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(enabled)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(park)
  SUB_BUTTON(auto)
  SUB_BUTTON(man)
#endif
#ifdef USE_SENSOR
  SUB_SENSOR(substatus)
  SUB_SENSOR(mode)
  SUB_SENSOR(next_start)
  SUB_SENSOR(num_sat)
  SUB_SENSOR(latitude)
  SUB_SENSOR(longitude)
  SUB_SENSOR(battery_level)
  SUB_SENSOR(battery_1_voltage)
  SUB_SENSOR(battery_1_level)
  SUB_SENSOR(battery_1_current)
  SUB_SENSOR(battery_1_temperature)
  SUB_SENSOR(battery_2_voltage)
  SUB_SENSOR(battery_2_level)
  SUB_SENSOR(battery_2_current)
  SUB_SENSOR(battery_2_temperature)
#endif
#ifdef USE_SENSOR
  SUB_TEXT_SENSOR(status)
#endif

 public:
  AutoMower(uart::UARTComponent *uart) : uart::UARTDevice(uart) {}

  void setup() override;

  void loop() override;

  void dump_config() override;

  float get_setup_priority() const override;

  void send(uint8_t command, const std::vector<uint8_t> &payload);
  void send_raw(const std::vector<uint8_t> &payload);
  uint8_t waiting_for_response{0};
  void set_timeout(uint16_t time_in_ms) { timeout_ = time_in_ms; }
  void set_disable_crc(bool disable_crc) { disable_crc_ = disable_crc; }

  void set_mode(enum AutoMowerMode mode);
  void set_pin(const uint16_t pin);

 protected:
  bool parse_automower_byte_(uint8_t byte);
  uint16_t timeout_{250};
  bool disable_crc_;
  std::vector<uint8_t> rx_buffer_;
  uint32_t last_automower_byte_{0};
  uint32_t last_send_{0};
  int poll_state{0};
  uint8_t mode_request{0};
};

}  // namespace automower
}  // namespace esphome
