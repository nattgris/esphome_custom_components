#include "ifan.h"
#include "light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ifan {


void IFan::loop() {
    if (!remote_enable_)
        return;
    while (this->available()) {
        uint8_t c;
        this->read_byte(&c);
        this->handle_char_(c);
    }
}

void IFan::handle_char_(uint8_t c) {
    if (!remote_enable_)
        return;
    static int state = 0;
    static uint8_t type = 0;
    static uint8_t param = 0;
    uint8_t csum;

    if (c == 0xaa) {
        state = 1;
        return;
    }
    switch (state) {
    case 1:
        if (c != 0x55)
        state = -1;
        break;
    case 2:
    case 5:
        if (c != 1)
        state = -1;
        break;
    case 3:
        type = c;
        break;
    case 4:
        if (c != 0)
        state = -1;
        break;
    case 6:
        param = c;
        break;
    case 7:
        csum = 2 + type + param;
        if (csum != c)
        ESP_LOGE(TAG, "checksum error: %02x != %02x", csum, c);
        else
        this->handle_command_(type, param);
        state = -1;
        break;
    default:
        state = -1;
    }
    state++;
}

void IFan::handle_command_(uint8_t type, uint8_t param) {
    if (!remote_enable_)
        return;
        ESP_LOGD(TAG, "command type %d param %d", type, param);

    if (type == 4) {
        if (param == 4){
        //IFanLight::toggle();        
        //this->light_trigger_->trigger();
        return;
        }else{
        this->do_speed(param*30);
        return;
        }
    }
}
void IFan::output(const float state){

}
}  // namespace ifan04
}  // namespace esphome
