import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from ..climate import heat_curve_ns, HeatingCurveClimate

HeatCurveClimateSwitch = heat_curve_ns.class_(
    "HeatCurveClimateSwitch", switch.Switch, cg.Component
)

CONF_CLIMATE_ID = "climate_id"
CONFIG_SCHEMA = (
    switch.switch_schema(HeatCurveClimateSwitch)
    .extend(
        {
            cv.GenerateID(CONF_CLIMATE_ID): cv.use_id(HeatingCurveClimate),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_CLIMATE_ID])
    var = await switch.new_switch(config)
    await cg.register_component(var, config)

    cg.add(var.set_parent(parent))
