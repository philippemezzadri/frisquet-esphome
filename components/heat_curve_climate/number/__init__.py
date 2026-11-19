import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_ICON,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_MODE,
    CONF_RESTORE_VALUE,
    CONF_STEP,
    CONF_TYPE,
    CONF_UNIT_OF_MEASUREMENT,
    ENTITY_CATEGORY_CONFIG,
)

from .. import heat_curve_ns, HeatingCurveClimate

HeatCurveClimateNumber = heat_curve_ns.class_(
    "HeatCurveClimateNumber", number.Number, cg.Component
)
HeatCurveClimateNumberType = heat_curve_ns.enum("HeatCurveClimateNumberType")

# type: (C++ enum, default min, default max, default step, default unit, default icon)
HEATCURVE_CLIMATE_NUMBER_TYPES = {
    "SLOPE": (
        HeatCurveClimateNumberType.HEAT_CURVE_NUMBER_TYPE_SLOPE,
        0.1, 5.0, 0.05, None, "mdi:slope-uphill",
    ),
    "SHIFT": (
        HeatCurveClimateNumberType.HEAT_CURVE_NUMBER_TYPE_SHIFT,
        -10.0, 10.0, 0.1, "K", "mdi:swap-vertical",
    ),
    "KP": (
        HeatCurveClimateNumberType.HEAT_CURVE_NUMBER_TYPE_KP,
        0.0, 20.0, 0.1, None, "mdi:alpha-p-circle-outline",
    ),
    "KI": (
        HeatCurveClimateNumberType.HEAT_CURVE_NUMBER_TYPE_KI,
        0.0, 0.01, 0.00001, None, "mdi:alpha-i-circle-outline",
    ),
    "MAX_ERROR": (
        HeatCurveClimateNumberType.HEAT_CURVE_NUMBER_TYPE_MAX_ERROR,
        0.1, 5.0, 0.1, "K", "mdi:thermometer-alert",
    ),
    "MIN_DELTA": (
        HeatCurveClimateNumberType.HEAT_CURVE_NUMBER_TYPE_MIN_DELTA,
        0.0, 10.0, 0.1, "K", "mdi:delta",
    ),
}

CONF_CLIMATE_ID = "climate_id"


def apply_type_defaults(config):
    _, min_value, max_value, step, unit, icon = HEATCURVE_CLIMATE_NUMBER_TYPES[
        config[CONF_TYPE]
    ]
    config.setdefault(CONF_MIN_VALUE, min_value)
    config.setdefault(CONF_MAX_VALUE, max_value)
    config.setdefault(CONF_STEP, step)
    if unit is not None:
        config.setdefault(CONF_UNIT_OF_MEASUREMENT, unit)
    config.setdefault(CONF_ICON, icon)
    if CONF_MODE not in config:
        # KI has too many steps for a slider: use an input box
        default_mode = "BOX" if config[CONF_TYPE] == "KI" else "SLIDER"
        config[CONF_MODE] = cv.enum(number.NUMBER_MODES, upper=True)(default_mode)
    if config[CONF_MAX_VALUE] <= config[CONF_MIN_VALUE]:
        raise cv.Invalid("max_value must be greater than min_value")
    return config


CONFIG_SCHEMA = cv.All(
    number.number_schema(
        HeatCurveClimateNumber,
        entity_category=ENTITY_CATEGORY_CONFIG,
    )
    .extend(
        {
            cv.GenerateID(CONF_CLIMATE_ID): cv.use_id(HeatingCurveClimate),
            cv.Required(CONF_TYPE): cv.one_of(*HEATCURVE_CLIMATE_NUMBER_TYPES, upper=True),
            cv.Optional(CONF_MIN_VALUE): cv.float_,
            cv.Optional(CONF_MAX_VALUE): cv.float_,
            cv.Optional(CONF_STEP): cv.positive_float,
            cv.Optional(CONF_RESTORE_VALUE, default=True): cv.boolean,
            # No schema default: it depends on the type (see apply_type_defaults)
            cv.Optional(CONF_MODE): cv.enum(number.NUMBER_MODES, upper=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    apply_type_defaults,
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_CLIMATE_ID])
    var = await number.new_number(
        config,
        min_value=config[CONF_MIN_VALUE],
        max_value=config[CONF_MAX_VALUE],
        step=config[CONF_STEP],
    )
    await cg.register_component(var, config)

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(HEATCURVE_CLIMATE_NUMBER_TYPES[config[CONF_TYPE]][0]))
    cg.add(var.set_restore_value(config[CONF_RESTORE_VALUE]))
