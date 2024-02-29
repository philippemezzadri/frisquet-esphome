import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    ICON_GAUGE,
    CONF_TYPE,
)
from ..climate import heat_curve_ns, HeatingCurveClimate

HeatCurveClimateSensor = heat_curve_ns.class_(
    "HeatCurveClimateSensor", sensor.Sensor, cg.Component
)
HeatCurveClimateSensorType = heat_curve_ns.enum("HeatCurveClimateSensorType")

HEATCURVE_CLIMATE_SENSOR_TYPES = {
    "RESULT": HeatCurveClimateSensorType.PID_SENSOR_TYPE_RESULT,
    "SETPOINT": HeatCurveClimateSensorType.PID_SENSOR_TYPE_SETPOINT,
    "ERROR": HeatCurveClimateSensorType.PID_SENSOR_TYPE_ERROR,
    "DELTA": HeatCurveClimateSensorType.PID_SENSOR_TYPE_DELTA,
    "WATERTEMP": HeatCurveClimateSensorType.PID_SENSOR_TYPE_WATERTEMP,
    "PROPORTIONAL": HeatCurveClimateSensorType.PID_SENSOR_TYPE_PROPORTIONAL,
    "INTEGRAL": HeatCurveClimateSensorType.PID_SENSOR_TYPE_INTEGRAL,
    "SLOPE": HeatCurveClimateSensorType.PID_SENSOR_TYPE_SLOPE,
    "SHIFT": HeatCurveClimateSensorType.PID_SENSOR_TYPE_SHIFT,
    "KP": HeatCurveClimateSensorType.PID_SENSOR_TYPE_KP,
    "KI": HeatCurveClimateSensorType.PID_SENSOR_TYPE_KI,
}

CONF_CLIMATE_ID = "climate_id"
CONFIG_SCHEMA = (
    sensor.sensor_schema(
        HeatCurveClimateSensor,
        icon=ICON_GAUGE,
        accuracy_decimals=1,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_CLIMATE_ID): cv.use_id(HeatingCurveClimate),
            cv.Required(CONF_TYPE): cv.enum(HEATCURVE_CLIMATE_SENSOR_TYPES, upper=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_CLIMATE_ID])
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(config[CONF_TYPE]))
