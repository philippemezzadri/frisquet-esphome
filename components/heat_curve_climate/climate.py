import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, sensor, output
from esphome.const import CONF_ID, CONF_SENSOR

heat_curve_ns = cg.esphome_ns.namespace("heat_curve")
HeatCurveClimate = heat_curve_ns.class_("HeatCurveClimate", climate.Climate, cg.Component)

CONF_CONTROL_PARAMETERS = "control_parameters"
CONF_KP = "kp"
CONF_HEATFACTOR = "heat_factor"
CONF_OFFSET = "offset"
CONF_OUTPUT = "output"
CONF_OUTDOOR_SENSOR = "outdoor_sensor"
CONF_OUTPUT_FACTOR = "output_factor"
CONF_OUTPUT_OFFSET = "output_offset"

CONFIG_SCHEMA = cv.All(
    climate.CLIMATE_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(HeatCurveClimate),
            cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
            cv.Required(CONF_OUTDOOR_SENSOR): cv.use_id(sensor.Sensor),
            cv.Required(CONF_OUTPUT): cv.use_id(output.FloatOutput),
            cv.Required(CONF_CONTROL_PARAMETERS): cv.Schema(
                {
                    cv.Required(CONF_HEATFACTOR): cv.float_,
                    cv.Required(CONF_OFFSET): cv.float_,
                    cv.Optional(CONF_KP,  default=0): cv.float_,
                    cv.Optional(CONF_OUTPUT_FACTOR, default=1.9): cv.float_,
                    cv.Optional(CONF_OUTPUT_OFFSET, default=-41): cv.float_,
                }
            ),
        }
    ),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    sens = await cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_sensor(sens))

    outdoor = await cg.get_variable(config[CONF_OUTDOOR_SENSOR])
    cg.add(var.set_outdoor_sensor(outdoor))

    out = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(out))

    params = config[CONF_CONTROL_PARAMETERS]
    cg.add(var.set_heat_factor(params[CONF_HEATFACTOR]))
    cg.add(var.set_offset(params[CONF_OFFSET]))
    cg.add(var.set_kp(params[CONF_KP]))
    cg.add(var.set_output_calibration_factor(params[CONF_OUTPUT_FACTOR]))
    cg.add(var.set_output_calibration_offset(params[CONF_OUTPUT_OFFSET]))
