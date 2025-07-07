import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from .. import AutoMower, CONF_AUTOMOWER_ID, am_ns

AUTO_LOAD = ["automower_g3"]

ParkButton = am_ns.class_("ParkButton", button.Button)
AutoButton = am_ns.class_("AutoButton", button.Button)
ManButton = am_ns.class_("ManButton", button.Button)

CONF_PARK = "park"
CONF_AUTO = "auto"
CONF_MAN = "man"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_AUTOMOWER_ID): cv.use_id(AutoMower),
    cv.Optional(CONF_PARK): button.button_schema(
        ParkButton,
        icon="mdi:home-import-outline",
    ),
    cv.Optional(CONF_AUTO): button.button_schema(
        AutoButton,
        icon="mdi:timer-refresh",
    ),
    cv.Optional(CONF_MAN): button.button_schema(
        ManButton,
        icon="mdi:refresh",
    ),
}


async def to_code(config):
    hub = await cg.get_variable(config[CONF_AUTOMOWER_ID])

    if CONF_PARK in config:
        b = await button.new_button(config[CONF_PARK])
        await cg.register_parented(b, config[CONF_AUTOMOWER_ID])
        cg.add(hub.set_park_button(b))
    if CONF_AUTO in config:
        b = await button.new_button(config[CONF_AUTO])
        await cg.register_parented(b, config[CONF_AUTOMOWER_ID])
        cg.add(hub.set_auto_button(b))
    if CONF_MAN in config:
        b = await button.new_button(config[CONF_MAN])
        await cg.register_parented(b, config[CONF_AUTOMOWER_ID])
        cg.add(hub.set_man_button(b))
