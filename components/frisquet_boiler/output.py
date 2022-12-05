import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import output
from esphome.const import CONF_ID, CONF_PIN

DEPENDENCIES = ["api"]
CODEOWNERS = ["@philippemezzadri"]

CONF_BOILER_ID = "boiler_id"

frisquet_boiler = cg.esphome_ns.namespace('frisquet_boiler')
FrisquetBoiler = frisquet_boiler.class_('FrisquetBoiler', output.FloatOutput,cg.Component)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(FrisquetBoiler),
    cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_BOILER_ID): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)
    await cg.register_component(var, config)

    boiler_id = config[CONF_BOILER_ID]
    cg.add(var.set_boiler_id(boiler_id))

    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))
