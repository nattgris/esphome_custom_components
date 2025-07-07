import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fan,uart
from esphome import automation
from esphome.automation import maybe_simple_id

from esphome.const import (
    CONF_OUTPUT_ID,
    CONF_ID,
)
from esphome.core import  coroutine_with_priority

DEPENDENCIES = ['uart']

BUZZER_ENABLE = "buzzer_enable"
REMOTE_ENABLE = "remote_enable"

ifan_ns = cg.esphome_ns.namespace("ifan")
IFan = ifan_ns.class_("IFan", cg.Component, fan.Fan, uart.UARTDevice)
CycleSpeedAction = ifan_ns.class_("CycleSpeedAction", automation.Action)

CONFIG_SCHEMA = fan.FAN_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(IFan),
        cv.Optional(BUZZER_ENABLE, default=True): cv.boolean,
        cv.Optional(REMOTE_ENABLE, default=True): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)
FAN_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(IFan),
    }
)
@automation.register_action("ifan.cycle_speed", CycleSpeedAction, FAN_ACTION_SCHEMA)
async def fan_cycle_speed_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

@coroutine_with_priority(100.0)

async def to_code(config):
    cg.add_define("USE_FAN")
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    cg.add(var.set_buzzer_enable(config[BUZZER_ENABLE]))
    cg.add(var.set_remote_enable(config[REMOTE_ENABLE]))
    if REMOTE_ENABLE in config:
        #await cg.register_component(var, config)
        await uart.register_uart_device(var, config)
    await cg.register_component(var, config)

    await fan.register_fan(var, config)

    
    cg.add_global(ifan_ns.using)