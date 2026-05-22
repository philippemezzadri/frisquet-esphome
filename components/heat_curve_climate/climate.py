import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import climate, sensor, output
from esphome.const import CONF_ID, CONF_SENSOR

from . import heat_curve_ns, HeatingCurveClimate

# Actions
SetControlParametersAction = heat_curve_ns.class_(
    "SetControlParametersAction", automation.Action
)
PIDResetIntegralTermAction = heat_curve_ns.class_(
    "PIDResetIntegralTermAction", automation.Action
)

# Default values for control parameters
DEF_SLOPE = 1.5
DEF_SHIFT = 0.0
DEF_KP = 0.0
DEF_KI = 0.0
DEF_ALT_CURVE = False
DEF_MAX_ERROR = 1.0
DEF_MIN_DELTA = 2.0

# Default values for output parameters
DEF_ROUNDED = False
DEF_OUTPUT_FACTOR = 1.0
DEF_OUTPUT_OFFSET = 0.0
DEF_MIN_OUTPUT = 0.0
DEF_MAX_OUTPUT = 1.0
DEF_HEATREQ_OUTPUT = 0.1

CONF_DEFAULT_TARGET_TEMPERATURE = "default_target_temperature"

CONF_CONTROL_PARAMETERS = "control_parameters"
CONF_OUTPUT_PARAMETERS = "output_parameters"
CONF_KP = "kp"
CONF_KI = "ki"
CONF_SLOPE = "slope"
CONF_SHIFT = "shift"
CONF_OUTPUT = "output"
CONF_OUTDOOR_SENSOR = "outdoor_sensor"
CONF_OUTPUT_FACTOR = "output_factor"
CONF_OUTPUT_OFFSET = "output_offset"
CONF_MINIMUM_OUTPUT = "minimum_output"
CONF_MAXIMUM_OUTPUT = "maximum_output"
CONF_HEATREQ_OUTPUT = "heat_required_output"
CONF_ROUNDED_OUTPUT = "rounded"
CONF_ALTERNATE_CURVE = "alt_curve"
CONF_MAX_ERROR = "max_error"
CONF_MIN_DELTA = "min_delta"

CONFIG_SCHEMA = cv.All(
    climate.climate_schema(HeatingCurveClimate).extend(
        {
            cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
            cv.Required(CONF_DEFAULT_TARGET_TEMPERATURE): cv.temperature,
            cv.Required(CONF_OUTDOOR_SENSOR): cv.use_id(sensor.Sensor),
            cv.Required(CONF_OUTPUT): cv.use_id(output.FloatOutput),
            cv.Optional(CONF_CONTROL_PARAMETERS): cv.Schema(
                {
                    cv.Optional(CONF_SLOPE, default=1.5): cv.float_,
                    cv.Optional(CONF_SHIFT, default=0): cv.float_,
                    cv.Optional(CONF_KP, default=0): cv.float_,
                    cv.Optional(CONF_KI, default=0): cv.float_,
                    cv.Optional(CONF_ALTERNATE_CURVE, default=False): cv.boolean,
                    cv.Optional(CONF_MAX_ERROR, default=1): cv.float_,
                    cv.Optional(CONF_MIN_DELTA, default=2): cv.float_,
                }
            ),
            cv.Optional(CONF_OUTPUT_PARAMETERS): cv.Schema(
                {
                    cv.Optional(CONF_ROUNDED_OUTPUT, default=False): cv.boolean,
                    cv.Optional(CONF_OUTPUT_FACTOR, default=1): cv.float_,
                    cv.Optional(CONF_OUTPUT_OFFSET, default=0): cv.float_,
                    cv.Optional(CONF_MINIMUM_OUTPUT, default=0.0): cv.float_,
                    cv.Optional(CONF_MAXIMUM_OUTPUT, default=1.0): cv.float_,
                    cv.Optional(CONF_HEATREQ_OUTPUT, default=0.1): cv.float_,
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

    p = config.get(CONF_CONTROL_PARAMETERS, {})
    cg.add(var.set_slope(p.get(CONF_SLOPE, DEF_SLOPE)))
    cg.add(var.set_shift(p.get(CONF_SHIFT, DEF_SHIFT)))
    cg.add(var.set_kp(p.get(CONF_KP, DEF_KP)))
    cg.add(var.set_ki(p.get(CONF_KI, DEF_KI)))
    cg.add(var.set_alt_curve(p.get(CONF_ALTERNATE_CURVE, DEF_ALT_CURVE)))
    cg.add(var.set_max_error(p.get(CONF_MAX_ERROR, DEF_MAX_ERROR)))
    cg.add(var.set_min_delta(p.get(CONF_MIN_DELTA, DEF_MIN_DELTA)))

    o = config.get(CONF_OUTPUT_PARAMETERS, {})
    cg.add(var.set_rounded(o.get(CONF_ROUNDED_OUTPUT, DEF_ROUNDED)))
    cg.add(
        var.set_output_calibration_factor(o.get(CONF_OUTPUT_FACTOR, DEF_OUTPUT_FACTOR))
    )
    cg.add(
        var.set_output_calibration_offset(o.get(CONF_OUTPUT_OFFSET, DEF_OUTPUT_OFFSET))
    )
    cg.add(var.set_minimum_output(o.get(CONF_MINIMUM_OUTPUT, DEF_MIN_OUTPUT)))
    cg.add(var.set_maximum_output(o.get(CONF_MAXIMUM_OUTPUT, DEF_MAX_OUTPUT)))
    cg.add(var.set_heat_required_output(o.get(CONF_HEATREQ_OUTPUT, DEF_HEATREQ_OUTPUT)))

    cg.add(var.set_default_target_temperature(config[CONF_DEFAULT_TARGET_TEMPERATURE]))


@automation.register_action(
    "climate.heat_curve.set_control_parameters",
    SetControlParametersAction,
    automation.maybe_simple_id(
        {
            cv.Required(CONF_ID): cv.use_id(HeatingCurveClimate),
            cv.Required(CONF_SLOPE): cv.templatable(cv.float_),
            cv.Required(CONF_SHIFT): cv.templatable(cv.float_),
            cv.Optional(CONF_KP, default=0.0): cv.templatable(cv.float_),
            cv.Optional(CONF_KI, default=0.0): cv.templatable(cv.float_),
            cv.Optional(CONF_MAX_ERROR, default=1.0): cv.templatable(cv.float_),
            cv.Optional(CONF_MIN_DELTA, default=2.0): cv.templatable(cv.float_),
        }
    ),
    synchronous=True,
)
async def set_control_parameters(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    kp_template_ = await cg.templatable(config[CONF_KP], args, float)
    cg.add(var.set_kp(kp_template_))

    ki_template_ = await cg.templatable(config[CONF_KI], args, float)
    cg.add(var.set_ki(ki_template_))

    slope_template_ = await cg.templatable(config[CONF_SLOPE], args, float)
    cg.add(var.set_slope(slope_template_))

    shift_template_ = await cg.templatable(config[CONF_SHIFT], args, float)
    cg.add(var.set_shift(shift_template_))

    max_error_template_ = await cg.templatable(config[CONF_MAX_ERROR], args, float)
    cg.add(var.set_max_error(max_error_template_))

    min_delta_template_ = await cg.templatable(config[CONF_MIN_DELTA], args, float)
    cg.add(var.set_min_delta(min_delta_template_))

    return var


@automation.register_action(
    "climate.heat_curve.reset_integral_term",
    PIDResetIntegralTermAction,
    automation.maybe_simple_id(
        {
            cv.Required(CONF_ID): cv.use_id(HeatingCurveClimate),
        }
    ),
    synchronous=True,
)
async def pid_reset_integral_term(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
