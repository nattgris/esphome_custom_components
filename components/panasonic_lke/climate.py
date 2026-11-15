import esphome.codegen as cg
from esphome.components import climate_ir

AUTO_LOAD = ["climate_ir"]

panasonic_lke_ns = cg.esphome_ns.namespace("panasonic_lke")
PanasonicLkeClimate = panasonic_lke_ns.class_("PanasonicLkeClimate", climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(PanasonicLkeClimate)


async def to_code(config):
    await climate_ir.new_climate_ir(config)
