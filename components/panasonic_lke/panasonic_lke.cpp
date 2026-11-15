#include "panasonic_lke.h"

#include <cmath>

#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/log.h"

namespace esphome {
namespace panasonic_lke {

static const char *const TAG = "panasonic.climate";

static const uint8_t header[] = { 0x02, 0x20, 0xE0, 0x04 };
static const uint8_t remote_header[] = {0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06};

static uint8_t sum(const uint8_t *data, int len)
{
	uint8_t sum = 0;
	for (int i = 0; i < len; i++) {
		sum += data[i];
	}
	return sum;
}

size_t PanasonicLkeClimate::build_frame(uint8_t data[PANASONIC_STATE_FRAME_SIZE])
{
	memcpy(data, header, sizeof(header));

	if (cmd.cmd != CMD_STATE) {
		data[4] = 0x80;
		data[5] = cmd.cmd;
		data[6] = cmd.cmd >> 8;
		data[7] = sum(data, 7);

		return 8;
	}

	bool no_time = cmd.time == 0 || cmd.no_time;
	uint16_t off_time = no_time ? 0x600 : cmd.off_time;
	uint16_t on_time  = no_time ? 0x600 : cmd.on_time;
	uint16_t time     = no_time ? 0 : cmd.time;

	data[4] = 0x00;
	data[5] = (cmd.mode << 4) | (1 << 3) | (cmd.off_timer << 2) | (cmd.on_timer << 1) | cmd.on;
	data[6] = cmd.temp;
	data[7] = 0x80;
	data[8] = (cmd.fan << 4) | cmd.swing;
	data[9] = 0x00;
	data[10] = on_time;
	data[11] = ((off_time & 0x03) << 4) | (1 << 3) | (on_time >> 8);
	data[12] = (1 << 7) | (off_time >> 4);
	data[13] = 0x00;
	data[14] = 0x00;
	data[15] = 0x80 | no_time;
	data[16] = time;
	data[17] = time >> 8;
	data[18] = sum(data, 18);

	return 19;
}

static void add_data(remote_base::RemoteTransmitData *data, const uint8_t *buf, size_t len)
{
	data->mark(PANASONIC_HEADER_MARK);
	data->space(PANASONIC_HEADER_SPACE);

	for (size_t i = 0; i < len; i++) {
		for (uint8_t mask = 1; mask > 0; mask <<= 1) {  // iterate through bit mask
			data->mark(PANASONIC_BIT_MARK);
			bool bit = buf[i] & mask;
			data->space(bit ? PANASONIC_ONE_SPACE : PANASONIC_ZERO_SPACE);
		}
	}

	data->mark(PANASONIC_BIT_MARK);
	data->space(PANASONIC_IDLE);
}

void PanasonicLkeClimate::transmit_raw_state()
{
	uint8_t remote_state[PANASONIC_STATE_FRAME_SIZE];

	size_t frame_len = build_frame(remote_state);

	char s[PANASONIC_STATE_FRAME_SIZE * 3 + 1];
	size_t len = 0;

	for (int i = 0; i < frame_len; i++) {
		len += snprintf(s + len, sizeof(s) - len, "%02x ", remote_state[i]);
	}
	ESP_LOGD(TAG, "TX %s", s);

	auto transmit = this->transmitter_->transmit();
	auto *data = transmit.get_data();
	data->set_carrier_frequency(PANASONIC_IR_FREQUENCY);

	add_data(data, remote_header, sizeof(remote_header));
	add_data(data, remote_state, frame_len);

	transmit.perform();
}

void PanasonicLkeClimate::set_raw_state()
{
	switch (mode) {
	case climate::CLIMATE_MODE_COOL:
		cmd.mode = MODE_COOL;
		cmd.on = true;
		break;
	case climate::CLIMATE_MODE_DRY:
		cmd.mode = MODE_DRY;
		cmd.on = true;
		break;
	case climate::CLIMATE_MODE_HEAT:
		cmd.mode = MODE_HEAT;
		cmd.on = true;
		break;
	case climate::CLIMATE_MODE_HEAT_COOL:
		cmd.mode = MODE_AUTO;
		cmd.on = true;
		break;
	case climate::CLIMATE_MODE_FAN_ONLY:
		cmd.mode = MODE_FAN;
		cmd.on = true;
		break;
	case climate::CLIMATE_MODE_OFF:
		cmd.on = false;
		break;
	}

	switch (fan_mode.value()) {
	case climate::CLIMATE_FAN_LOW:
		cmd.fan = FAN_1;
		break;
	case climate::CLIMATE_FAN_MEDIUM:
		if (cmd.fan != FAN_2 && cmd.fan != FAN_3 && cmd.fan != FAN_4) {
			cmd.fan = FAN_3;
		}
		break;
	case climate::CLIMATE_FAN_HIGH:
		cmd.fan = FAN_5;
		break;
	case climate::CLIMATE_FAN_AUTO:
		cmd.fan = FAN_AUTO;
		break;
	}

	switch (swing_mode) {
	case climate::CLIMATE_SWING_VERTICAL:
		if (cmd.swing != SWING_4 && cmd.swing != SWING_5) {
			cmd.swing = SWING_5;
		}
		break;
	case climate::CLIMATE_SWING_HORIZONTAL:
		if (cmd.swing != SWING_1 && cmd.swing != SWING_2 && cmd.swing != SWING_3) {
			cmd.swing = SWING_1;
		}
		break;
	case climate::CLIMATE_SWING_BOTH:
		cmd.swing = SWING_AUTO;
		break;
	default:
		break;
	}

	float new_temp = clamp<float>(target_temperature, PANASONIC_TEMP_MIN, PANASONIC_TEMP_MAX);
	cmd.temp = round(new_temp * 2);
}

void PanasonicLkeClimate::transmit_state()
{
	set_raw_state();
	transmit_raw_state();
}

climate::ClimateTraits PanasonicLkeClimate::traits()
{
	climate::ClimateTraits traits = climate_ir::ClimateIR::traits();
	return traits;
}

bool PanasonicLkeClimate::on_receive(remote_base::RemoteReceiveData data)
{
	uint8_t state_frame[PANASONIC_STATE_FRAME_SIZE] = {};

	size_t item_count = data.size() / 2;
	uint8_t byte = 0;
	size_t bitcount;
	size_t bytecount;
	bool in_frame = false;

	if (item_count < 66) {
		return false;
	}

	for (size_t i = 0; i < item_count; i++) {
		if (data.expect_item(PANASONIC_HEADER_MARK, PANASONIC_HEADER_SPACE)) {
			bitcount = 0;
			bytecount = 0;
			in_frame = true;
			//ESP_LOGD(TAG, "header found");
		} else if (in_frame) {
			if (data.expect_item(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE)) {
				byte = (byte >> 1) | 0x80;
			} else if (data.expect_item(PANASONIC_BIT_MARK, PANASONIC_ZERO_SPACE)) {
				byte = (byte >> 1);
			} else if (data.expect_pulse_with_gap(PANASONIC_BIT_MARK, PANASONIC_HEADER_SPACE)) {
				//ESP_LOGD(TAG, "end");
				if (bitcount != 0) {
					ESP_LOGD(TAG, "discarding %zu trailing bits: 0x%02x", bitcount, byte);
				}
				if (i + 1 < item_count) {
					ESP_LOGD(TAG, "discarding %zu items after end", item_count - i - 1);
				}
				break;
			} else {
				ESP_LOGD(TAG, "invalid item %" PRId32 ",%" PRId32,
				         data.get_raw_data()[data.get_index()],
				         data.get_raw_data()[data.get_index()+1]);
				return false;
			}

			if (++bitcount >= 8) {
				bitcount = 0;
				if (bytecount < PANASONIC_STATE_FRAME_SIZE) {
					state_frame[bytecount] = byte;
				} else {
					return false;
				}
				//ESP_LOGD(TAG, "byte 0x%02x", byte);
				bytecount++;
			}
		} else {
			ESP_LOGD(TAG, "not in frame");
		}
	}

	char s[PANASONIC_STATE_FRAME_SIZE * 3 + 1];
	size_t len = 0;

	for (int i = 0; i < bytecount; i++) {
		len += snprintf(s + len, sizeof(s) - len, "%02x ", state_frame[i]);
	}
	ESP_LOGD(TAG, "RCV %s", s);

	if (!parse_state_frame(state_frame, bytecount)) {
		return false;
	}

	/* TODO: Add proxy_ir setting and transmit frame only if it's active. */
	transmit_raw_state();
	publish_state();
	return true;
}

bool PanasonicLkeClimate::parse_state_frame(const uint8_t frame[], size_t len)
{
	if (len != 19 && len != 8) {
		ESP_LOGW(TAG, "Invalid length %d", len);
		return false;
	}

	if (sum(frame, len - 1) != frame[len - 1]) {
		ESP_LOGW(TAG, "Invalid checksum");
		return false;
	}

	if (memcmp(frame, header, sizeof(header)) != 0) {
		ESP_LOGW(TAG, "Invalid header");
		return false;
	}

	if (len == 8 && (frame[4] & 0x80) == 0) {
		return true; /* Header found */
	}

	cmd = {};

	cmd.cmd = len == 19 ? CMD_STATE : (frame[6] << 8) | frame[5];

	switch (cmd.cmd) {
	case CMD_STATE:
		break;
	case CMD_E_ION:
	case CMD_PATROL:
	case CMD_QUIET:
	case CMD_POWERFUL:
	case CMD_CHECK:
	case CMD_SET_AIR_1:
	case CMD_SET_AIR_2:
	case CMD_SET_AIR_3:
	case CMD_AC_RESET:
		return true;
	default:
		ESP_LOGW(TAG, "Invalid command %d", cmd.cmd);
		return false;
	}

	assert(len == 19);

	cmd.mode = frame[5] >> 4;

	switch (cmd.mode) {
	case MODE_AUTO:
		mode = climate::CLIMATE_MODE_HEAT_COOL;
		break;
	case MODE_COOL:
		mode = climate::CLIMATE_MODE_COOL;
		break;
	case MODE_DRY:
		mode = climate::CLIMATE_MODE_DRY;
		break;
	case MODE_FAN:
		mode = climate::CLIMATE_MODE_FAN_ONLY;
		break;
	case MODE_HEAT:
		mode = climate::CLIMATE_MODE_HEAT;
		break;
	default:
		ESP_LOGW(TAG, "Invalid mode %d", cmd.mode);
		return false;
	}

	cmd.off_timer = (frame[5] & 4) != 0;
	cmd.on_timer  = (frame[5] & 2) != 0;
	cmd.on        = (frame[5] & 1) != 0;
	if (!cmd.on) {
		mode = climate::CLIMATE_MODE_OFF;
	}

	cmd.temp = frame[6] & 0x3F;
	target_temperature = cmd.temp * 0.5f;

	cmd.swing = frame[8] & 0x0F;
	cmd.fan = frame[8] >> 4;

	switch (cmd.swing) {
	case SWING_AUTO:
		swing_mode = climate::CLIMATE_SWING_BOTH;
		break;
	case SWING_1:
	case SWING_2:
	case SWING_3:
		swing_mode = climate::CLIMATE_SWING_HORIZONTAL;
		break;
	case SWING_4:
	case SWING_5:
		swing_mode = climate::CLIMATE_SWING_VERTICAL;
		break;
	default:
		ESP_LOGW(TAG, "Invalid swing mode %d", cmd.swing);
		return false;
	}

	clear_custom_fan_mode_();
	switch (cmd.fan) {
	case FAN_AUTO:
		fan_mode = climate::CLIMATE_FAN_AUTO;
		break;
	case FAN_1:
		fan_mode = climate::CLIMATE_FAN_LOW;
		break;
	case FAN_2:
		set_custom_fan_mode_("medium low");
		break;
	case FAN_3:
		fan_mode = climate::CLIMATE_FAN_MEDIUM;
		break;
	case FAN_4:
		set_custom_fan_mode_("medium high");
		break;
	case FAN_5:
		fan_mode = climate::CLIMATE_FAN_HIGH;
		break;
	default:
		ESP_LOGW(TAG, "Invalid fan mode %d", cmd.fan);
		return false;
	}

	/* TODO: Support time settings. */

	return true;
}

void PanasonicLkeClimate::control(const climate::ClimateCall &call) {
	if (call.get_target_humidity().has_value()) {
		this->target_humidity = *call.get_target_humidity();
	}
	climate_ir::ClimateIR::control(call);
}

}  // namespace panasonic_lke
}  // namespace esphome
