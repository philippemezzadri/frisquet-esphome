import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import climate, sensor, output
from esphome.const import CONF_ID, CONF_SENSOR

heat_curve_ns = cg.esphome_ns.namespace("heat_curve")
HeatingCurveClimate = heat_curve_ns.class_(
    "HeatingCurveClimate", climate.Climate, cg.Component
)

# Actions
SetControlParametersAction = heat_curve_ns.class_(
    "SetControlParametersAction", automation.Action
)
PIDResetIntegralTermAction = heat_curve_ns.class_(
    "PIDResetIntegralTermAction", automation.Action
)

CONF_DEFAULT_TARGET_TEMPERATURE = "default_target_temperature"

CONF_CONTROL_PARAMETERS = "control_parameters"
CONF_OUTPUT_PARAMETERS = "output_parameters"
CONF_KP = "kp"
CONF_KI = "ki"
CONF_SLOPE = "slope"
CONF_SHIFT = "shift"
CONF_OUTPUT = "output"
CONF_OUTDOOR_SENSOR = "outdoor_sensor"
CONF_OUTPUT_PARAMETERS = "output_parameters"
CONF_OUTPUT_FACTOR = "output_factor"
CONF_OUTPUT_OFFSET = "output_offset"
CONF_MINIMUM_OUTPUT = "minimum_output"
CONF_MAXIMUM_OUTPUT = "maximum_output"
CONF_HEATREQ_OUTPUT = "heat_required_output"
CONF_ROUNDED_OUPUT = "rounded"
CONF_ALTERNATE_CURVE = "alt_curve"
CONF_MAX_ERROR = "max_error"
CONF_MIN_DELTA = "min_delta"

CONFIG_SCHEMA = cv.All(
    climate.climate_schema(HeatingCurveClimate).extend(
        {
            # cv.GenerateID(): cv.declare_id(HeatingCurveClimate),
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
                    cv.Optional(CONF_ROUNDED_OUPUT, default=False): cv.boolean,
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

    params = config[CONF_CONTROL_PARAMETERS]
    cg.add(var.set_slope(params[CONF_SLOPE]))
    cg.add(var.set_shift(params[CONF_SHIFT]))
    cg.add(var.set_kp(params[CONF_KP]))
    cg.add(var.set_ki(params[CONF_KI]))
    cg.add(var.set_alt_curve(params[CONF_ALTERNATE_CURVE]))
    cg.add(var.set_max_error(params[CONF_MAX_ERROR]))
    cg.add(var.set_min_delta(params[CONF_MIN_DELTA]))

    output_params = config[CONF_OUTPUT_PARAMETERS]
    cg.add(var.set_rounded(output_params[CONF_ROUNDED_OUPUT]))
    cg.add(var.set_output_calibration_factor(output_params[CONF_OUTPUT_FACTOR]))
    cg.add(var.set_output_calibration_offset(output_params[CONF_OUTPUT_OFFSET]))
    cg.add(var.set_minimum_output(output_params[CONF_MINIMUM_OUTPUT]))
    cg.add(var.set_maximum_output(output_params[CONF_MAXIMUM_OUTPUT]))
    cg.add(var.set_heat_required_output(output_params[CONF_HEATREQ_OUTPUT]))

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
)
async def pid_reset_integral_term(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
