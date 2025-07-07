import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    DEVICE_CLASS_SAFETY,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import AutoMower, CONF_AUTOMOWER_ID

AUTO_LOAD = ["automower_g3"]

CONF_ENABLED = "enabled"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_AUTOMOWER_ID): cv.use_id(AutoMower),
        cv.Optional(CONF_ENABLED): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_SAFETY,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_AUTOMOWER_ID])

    if CONF_ENABLED in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_ENABLED])
        cg.add(hub.set_enabled_binary_sensor(sens))
