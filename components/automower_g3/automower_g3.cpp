#include "automower_g3.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace automower {

static const char *const TAG = "automower";
static const uint8_t STX = 0x02;
static const uint8_t ETX = 0x03;

void AutoMower::setup() {
}
void AutoMower::loop() {
  const uint32_t now = millis();

  if (now - this->last_automower_byte_ > 50) {
    this->rx_buffer_.clear();
    this->last_automower_byte_ = now;
  }
  // stop blocking new send commands after timeout_ ms regardless if a response has been received since then
  if (now - this->last_send_ > timeout_) {
    waiting_for_response = 0;
  }

  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    if (this->parse_automower_byte_(byte)) {
      this->last_automower_byte_ = now;
    } else {
      this->rx_buffer_.clear();
    }
  }

  if (now - this->last_send_ > 2000) {
    if (mode_request != 0) {
      this->send(0x0E, { mode_request });
      mode_request = 0;
    } else switch (poll_state++) {
    case 0:
      this->send(0x12, { 0x01 });
      break;
    case 1:
      this->send(0x14, { 0x01 });
      break;
    case 2:
      this->send(0x14, { 0x05 });
      break;
    default:
      this->send(0x14, { 0x07 });
      poll_state = 0;
      break;
    }
  }
}

static uint16_t get_uint16(const uint8_t *buf, size_t offset)
{
  return (uint16_t)buf[offset]
       | (uint16_t)buf[offset + 1] << 8;
}

static uint32_t get_uint32(const uint8_t *buf, size_t offset)
{
  return (uint32_t)buf[offset]
       | (uint32_t)buf[offset + 1] << 8
       | (uint32_t)buf[offset + 2] << 16
       | (uint32_t)buf[offset + 3] << 24;
}

static void publish(sensor::Sensor *sensor, float value)
{
  if (sensor != nullptr) {
    sensor->publish_state(value);
  }
}

void AutoMower::set_mode(enum AutoMowerMode mode)
{
  mode_request = mode;
}

bool AutoMower::parse_automower_byte_(uint8_t byte) {
  size_t at = this->rx_buffer_.size();
  this->rx_buffer_.push_back(byte);
  const uint8_t *raw = &this->rx_buffer_[0];
  //ESP_LOGV(TAG, "RX[%zu] 0x%02x", at, byte);
  // Byte 0: STX, otherwise restart until it is
  if (at == 0 && byte != STX)
    return false;

  if (at <= 2)
    return true;

  uint8_t command = raw[1];
  uint8_t data_len = raw[2];

  // Detect the end of a valid frame (we already know that the first byte is STX)
  if (byte == ETX && at == data_len + 4) {
    bool crc_ok = crc8(raw + 1, data_len + 3) == 0;
    if (!crc_ok) {
      if (this->disable_crc_) {
        ESP_LOGD(TAG, "Automower CRC Check failed, but ignored!");
      } else {
        ESP_LOGW(TAG, "Automower CRC Check failed!");
        return false;
      }
    }
  } else if (at >= data_len + 4) {
    ESP_LOGW(TAG, "Automower found no ETX where expected, restarting!");
    // FIXME: To guarantee sync, find the next STX in the buffer and retry parsing from there
    return false;
  } else {
    return true;
  }
  waiting_for_response = 0;

  ESP_LOGV(TAG, "AutoMower got frame: %s", format_hex_pretty(this->rx_buffer_).c_str());

  if (command == 0x13 && data_len >= 0x17) {
    auto next_start = get_uint32(raw, 10);
    publish(status_sensor_, raw[4]);
    publish(substatus_sensor_, get_uint16(raw, 7));
    publish(next_start_sensor_, next_start > 0 ? next_start : NAN);
    uint32_t current_time = get_uint32(raw, 15);
  } else if (command == 0x15 && data_len == 0x15) {
    uint16_t bat1_cap = get_uint16(raw, 6);
    uint16_t bat1_full = get_uint16(raw, 12);
    uint16_t bat2_cap = get_uint16(raw, 16);
    uint16_t bat2_full = get_uint16(raw, 22);

    publish(battery_level_sensor_, (bat1_cap + bat2_cap) * 100.0f / (bat1_full + bat2_full));
    publish(battery_1_voltage_sensor_, get_uint16(raw, 4) / 1000.0f);
    publish(battery_1_level_sensor_, bat1_cap * 100.0f / bat1_full);
    publish(battery_1_current_sensor_, (int16_t)get_uint16(raw, 8) / 1000.0f);
    publish(battery_1_temperature_sensor_, (int16_t)get_uint16(raw, 10) / 10.0f);
    publish(battery_2_voltage_sensor_, get_uint16(raw, 14) / 1000.0f);
    publish(battery_2_level_sensor_, bat2_cap * 100.0f / bat2_full);
    publish(battery_2_current_sensor_, (int16_t)get_uint16(raw, 18) / 1000.0f);
    publish(battery_2_temperature_sensor_, (int16_t)get_uint16(raw, 20) / 10.0f);
  } else if (command == 0x15 && data_len == 0x04) {
    if (this->enabled_binary_sensor_ != nullptr) {
      this->enabled_binary_sensor_->publish_state(raw[6] == 0);
    }
  } else if (command == 0x15 && data_len == 0x1f) {
    uint8_t sat = raw[5];
    uint16_t lat_deg = get_uint16(raw, 10);
    uint32_t lat_min = get_uint16(raw, 14);
    uint16_t lng_deg = get_uint16(raw, 18);
    uint32_t lng_min = get_uint16(raw, 22);

    double lat = (lat_deg / 100) + ((lat_deg % 100) * 10000 + lat_min) / 600000.0;
    double lng = (lng_deg / 100) + ((lng_deg % 100) * 10000 + lng_min) / 600000.0;

    publish(num_sat_sensor_, sat);
    publish(latitude_sensor_, lat);
    publish(longitude_sensor_, lng);

    ESP_LOGD(TAG, "GPS extra data %x, %04x %04x %04x %02x", get_uint16(raw, 6), get_uint16(raw, 26), get_uint16(raw, 28), get_uint16(raw, 30), raw[32]);
  } else if (command == 0x0E && data_len == 1) {
    ESP_LOGI(TAG, "Set Mode: 0x%02x", (unsigned)raw[3]);
  } else if (command == 0x06) {
    ESP_LOGD(TAG, "AutoMower got frame: %s", format_hex_pretty(this->rx_buffer_).c_str());
  }


  // return false to reset buffer
  return false;
}

void AutoMower::dump_config() {
  ESP_LOGCONFIG(TAG, "Automower:");
  ESP_LOGCONFIG(TAG, "  Timeout: %d ms", this->timeout_);
  ESP_LOGCONFIG(TAG, "  CRC Disabled: %s", YESNO(this->disable_crc_));
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "EnabledBinarySensor", this->enabled_binary_sensor_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON("  ", "ParkButton", this->park_button_);
  LOG_BUTTON("  ", "AutoButton", this->auto_button_);
  LOG_BUTTON("  ", "ManButton", this->man_button_);
#endif
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "StatusSensor", this->status_sensor_);
  LOG_SENSOR("  ", "SubstatusSensor", this->substatus_sensor_);
  LOG_SENSOR("  ", "NextStartSensor", this->next_start_sensor_);
  LOG_SENSOR("  ", "NumSatSensor", this->num_sat_sensor_);
  LOG_SENSOR("  ", "LatitudeSensor", this->latitude_sensor_);
  LOG_SENSOR("  ", "LongitudeSensor", this->longitude_sensor_);
  LOG_SENSOR("  ", "BatteryLevelSensor", this->battery_level_sensor_);
  LOG_SENSOR("  ", "Battery1Voltage", this->battery_1_voltage_sensor_);
  LOG_SENSOR("  ", "Battery1Level", this->battery_1_level_sensor_);
  LOG_SENSOR("  ", "Battery1Current", this->battery_1_current_sensor_);
  LOG_SENSOR("  ", "Battery1Temperature", this->battery_1_temperature_sensor_);
  LOG_SENSOR("  ", "Battery2Voltage", this->battery_2_voltage_sensor_);
  LOG_SENSOR("  ", "Battery2Level", this->battery_2_level_sensor_);
  LOG_SENSOR("  ", "Battery2Current", this->battery_2_current_sensor_);
  LOG_SENSOR("  ", "Battery2Temperature", this->battery_2_temperature_sensor_);
#endif
}
float AutoMower::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}

void AutoMower::send(uint8_t command, const std::vector<uint8_t> &payload) {
  if (payload.size() > 255) {
    ESP_LOGE(TAG, "Automower frame length exceeded");
    return;
  }

  if (command & 1 != 0) {
    ESP_LOGE(TAG, "Cannot send response as command: 0x%02x", command);
  }
  std::vector<uint8_t> data;
  data.push_back(STX);
  data.push_back(command);
  data.push_back(payload.size());

  for (auto i = payload.begin(); i < payload.end(); i++) {
    data.push_back(*i);
  }

  auto crc = crc8(data.data() + 1, data.size() - 1);
  data.push_back(crc);
  data.push_back(ETX);

  this->write_array(data);
  this->flush();

  waiting_for_response = command;
  last_send_ = millis();
  ESP_LOGV(TAG, "AutoMower write: %s", format_hex_pretty(data).c_str());
}

// Helper function for lambdas
// Send raw command. Except CRC everything must be contained in payload
void AutoMower::send_raw(const std::vector<uint8_t> &payload) {
  if (payload.empty()) {
    return;
  }

  auto crc = crc8((uint8_t*)payload.data(), payload.size());
  this->write_byte(STX);
  this->write_array(payload);
  this->write_byte(crc);
  this->write_byte(ETX);
  this->flush();
  waiting_for_response = payload[0];
  ESP_LOGV(TAG, "AutoMower write raw: %s", format_hex_pretty(payload).c_str());
  last_send_ = millis();
}

}  // namespace automower
}  // namespace esphome
