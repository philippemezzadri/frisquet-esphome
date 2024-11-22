import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome import pins
from esphome.components import output
from esphome.const import CONF_ID, CONF_PIN, CONF_MODE

CONF_BOILER_ID = "boiler_id"
CONF_FRISQUETBOILER_ID = "friquet_boiler_id"
CONF_CALIBRATION_FACTOR = "calibration_factor"
CONF_CALIBRATION_OFFSET = "calibration_offset"

frisquet_boiler_ns = cg.esphome_ns.namespace("frisquet_boiler")
FrisquetBoiler = frisquet_boiler_ns.class_(
    "FrisquetBoiler", output.FloatOutput, cg.Component
)


# Actions
SetModeAction = frisquet_boiler_ns.class_("SetModeAction", automation.Action)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(FrisquetBoiler),
        cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_BOILER_ID): cv.string,
        cv.Optional(CONF_CALIBRATION_FACTOR, default=1.9): cv.float_,
        cv.Optional(CONF_CALIBRATION_OFFSET, default=-41): cv.float_,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)
    await cg.register_component(var, config)

    boiler_id = config[CONF_BOILER_ID]
    cg.add(var.set_boiler_id(boiler_id))
    cg.add(var.set_output_calibration_factor(config[CONF_CALIBRATION_FACTOR]))
    cg.add(var.set_output_calibration_offset(config[CONF_CALIBRATION_OFFSET]))

    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))


@automation.register_action(
    "boiler.set_mode",
    SetModeAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(FrisquetBoiler),
            cv.Required(CONF_MODE): cv.templatable(cv.positive_int),
        }
    ),
)
async def boiler_set_mode_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_MODE], args, int)
    cg.add(var.set_mode(template_))
    return var
