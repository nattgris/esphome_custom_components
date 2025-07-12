import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    CONF_ID,
)
from . import AutoMower, CONF_AUTOMOWER_ID

AUTO_LOAD = ["automower_g3"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_AUTOMOWER_ID): cv.use_id(AutoMower),
        cv.Optional("status"): text_sensor.text_sensor_schema(
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_AUTOMOWER_ID])

    text_sensors = []
    for key, conf in config.items():
        if not isinstance(conf, dict):
            continue
        id = conf[CONF_ID]
        if id and id.type == text_sensor.TextSensor:
            sens = await text_sensor.new_text_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
            text_sensors.append(f"F({key})")

    if text_sensors:
        cg.add_define(
            "TEXT_SENSOR_LIST(F, sep)", cg.RawExpression(" sep ".join(text_sensors))
        )
