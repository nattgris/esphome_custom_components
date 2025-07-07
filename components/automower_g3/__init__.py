import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
    CONF_PIN,
    CONF_DISABLE_CRC,
    CONF_BATTERY_VOLTAGE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_BATTERY,
    UNIT_VOLT,
)
from esphome import automation

CODEOWNERS = ["@nattgris"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

am_ns = cg.esphome_ns.namespace("automower")
AutoMower = am_ns.class_("AutoMower", cg.Component, uart.UARTDevice)

CONF_AUTOMOWER_ID = "automower_g3_id"
CONF_TIMEOUT = "timeout"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AutoMower),
            cv.Optional(
                CONF_TIMEOUT, default="250ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_DISABLE_CRC, default=False): cv.boolean,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_timeout(config[CONF_TIMEOUT]))
    cg.add(var.set_disable_crc(config[CONF_DISABLE_CRC]))


# Actions
PINSetAction = am_ns.class_(
    "PINSetAction", automation.Action
)


PIN_SET_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(AutoMower),
        cv.Required(CONF_PIN): cv.templatable(cv.positive_int),
    }
)


@automation.register_action(
    "pin.set", PINSetAction, PIN_SET_SCHEMA
)
async def pin_set_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_PIN], args, cg.uint16)
    cg.add(var.set_pin(template_))
    return var
