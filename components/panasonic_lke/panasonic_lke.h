#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome {
namespace panasonic_lke {

// Temperature
const float PANASONIC_TEMP_MIN = 8;  // Celsius
const float PANASONIC_TEMP_MAX = 31.5;  // Celsius

// IR Transmission
const uint32_t PANASONIC_IR_FREQUENCY = 38000;
const uint32_t PANASONIC_IDLE = 10400;
const uint32_t PANASONIC_HEADER_MARK = 3540;
const uint32_t PANASONIC_HEADER_SPACE = 1700;
const uint32_t PANASONIC_BIT_MARK = 460;
const uint32_t PANASONIC_ONE_SPACE = 1285;
const uint32_t PANASONIC_ZERO_SPACE = 410;

// State Frame size
const uint8_t PANASONIC_STATE_FRAME_SIZE = 19;

const uint16_t CMD_STATE     = 0;
const uint16_t CMD_E_ION     = 0x3361;
const uint16_t CMD_PATROL    = 0x3363;
const uint16_t CMD_QUIET     = 0x3381;
const uint16_t CMD_POWERFUL  = 0x3586;
const uint16_t CMD_CHECK     = 0x3293;
const uint16_t CMD_SET_AIR_1 = 0x328D;
const uint16_t CMD_SET_AIR_2 = 0x328E;
const uint16_t CMD_SET_AIR_3 = 0x328F;
const uint16_t CMD_AC_RESET  = 0x9D32;

const uint8_t MODE_AUTO = 0;
const uint8_t MODE_DRY  = 2;
const uint8_t MODE_COOL = 3;
const uint8_t MODE_HEAT = 4;
const uint8_t MODE_FAN  = 6;

const uint8_t SWING_1 = 1; /* H */
const uint8_t SWING_2 = 2;
const uint8_t SWING_3 = 3;
const uint8_t SWING_4 = 4;
const uint8_t SWING_5 = 5; /* V */
const uint8_t SWING_AUTO = 0xF;

const uint8_t FAN_1 = 3;
const uint8_t FAN_2 = 4;
const uint8_t FAN_3 = 5;
const uint8_t FAN_4 = 6;
const uint8_t FAN_5 = 7;
const uint8_t FAN_AUTO = 0xA;

struct panasonic_command {
	uint16_t cmd;
	uint8_t mode;
	uint8_t swing;
	uint8_t fan;
	uint16_t on_time;
	uint16_t off_time;
	uint16_t time;
	uint8_t temp; /* Temperature in half-degrees Celsius */
	bool on :1;
	bool on_timer :1;
	bool off_timer :1;
	bool no_time :1;
};

class PanasonicLkeClimate : public climate_ir::ClimateIR {
public:
	PanasonicLkeClimate()
	: climate_ir::ClimateIR(PANASONIC_TEMP_MIN, PANASONIC_TEMP_MAX, 0.5f, true, true,
			{climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
					climate::CLIMATE_FAN_HIGH},
					{climate::CLIMATE_SWING_VERTICAL,
							climate::CLIMATE_SWING_HORIZONTAL, climate::CLIMATE_SWING_BOTH}) {}
	virtual ~PanasonicLkeClimate() = default;

protected:
	void control(const climate::ClimateCall &call) override;
	// Transmit via IR the state of this climate controller.
	size_t build_frame(uint8_t data[PANASONIC_STATE_FRAME_SIZE]);
	void set_raw_state();
	void transmit_raw_state();
	void transmit_state() override;
	climate::ClimateTraits traits() override;
	struct panasonic_command cmd;
	// Handle received IR Buffer
	bool on_receive(remote_base::RemoteReceiveData data) override;
	bool parse_state_frame(const uint8_t frame[], size_t len);
};

}  // namespace panasonic_lke
}  // namespace esphome
