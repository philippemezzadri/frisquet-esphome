import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG
from .. import frisquet_boiler_ns, CONF_FRISQUETBOILER_ID, FrisquetBoiler

CONF_TEST = "test"
CONF_PAIR = "pair"

TestSwitch = frisquet_boiler_ns.class_("TestSwitch", switch.Switch)
PairSwitch = frisquet_boiler_ns.class_("PairSwitch", switch.Switch)

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_FRISQUETBOILER_ID): cv.use_id(FrisquetBoiler),
    cv.Optional(CONF_TEST): switch.switch_schema(
        TestSwitch,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:cog",
    ),
    cv.Optional(CONF_PAIR): switch.switch_schema(
        PairSwitch,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:cog",
    ),
}


async def to_code(config):
    parent = await cg.get_variable(config[CONF_FRISQUETBOILER_ID])
    if test_config := config.get(CONF_TEST):
        b = await switch.new_switch(test_config)
        await cg.register_parented(b, config[CONF_FRISQUETBOILER_ID])
        cg.add(parent.set_test_switch(b))
    if pair_config := config.get(CONF_PAIR):
        b = await switch.new_switch(pair_config)
        await cg.register_parented(b, config[CONF_FRISQUETBOILER_ID])
        cg.add(parent.set_pair_switch(b))
