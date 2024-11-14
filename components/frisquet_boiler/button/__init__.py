import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG
from ..output import CONF_FRISQUETBOILER_ID, frisquet_boiler_ns, FrisquetBoiler

CONF_TEST = "test"
CONF_PAIR = "pair"

TestButton = frisquet_boiler_ns.class_("TestButton", button.Button)
PairButton = frisquet_boiler_ns.class_("PairButton", button.Button)

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_FRISQUETBOILER_ID): cv.use_id(FrisquetBoiler),
    cv.Optional(CONF_TEST): button.button_schema(
        TestButton,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:cog",
    ),
    cv.Optional(CONF_PAIR): button.button_schema(
        PairButton,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:cog",
    ),
}


async def to_code(config):
    parent = await cg.get_variable(config[CONF_FRISQUETBOILER_ID])
    if test_config := config.get(CONF_TEST):
        b = await button.new_button(test_config)
        await cg.register_parented(b, config[CONF_FRISQUETBOILER_ID])
        cg.add(parent.set_test_button(b))
    if pair_config := config.get(CONF_PAIR):
        b = await button.new_button(pair_config)
        await cg.register_parented(b, config[CONF_FRISQUETBOILER_ID])
        cg.add(parent.set_pair_button(b))
