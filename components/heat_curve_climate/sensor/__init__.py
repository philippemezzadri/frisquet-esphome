import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    ICON_THERMOMETER,
    CONF_TYPE,
)
from ..climate import heat_curve_ns, HeatCurveClimate

HeatCurveClimateSensor = heat_curve_ns.class_("HeatCurveClimateSensor", sensor.Sensor, cg.Component)
HeatCurveClimateSensorType = heat_curve_ns.enum("HeatCurveClimateSensorType")

HEATCURVE_CLIMATE_SENSOR_TYPES = {
    "RESULT": HeatCurveClimateSensorType.PID_SENSOR_TYPE_RESULT,
    "SETPOINT": HeatCurveClimateSensorType.PID_SENSOR_TYPE_SETPOINT,
    "ERROR": HeatCurveClimateSensorType.PID_SENSOR_TYPE_ERROR,
    "DELTA": HeatCurveClimateSensorType.PID_SENSOR_TYPE_DELTA,
    "WATERTEMP": HeatCurveClimateSensorType.PID_SENSOR_TYPE_WATERTEMP,
    "PROPORTIONAL": HeatCurveClimateSensorType.PID_SENSOR_TYPE_PROPORTIONAL,
}

CONF_CLIMATE_ID = "climate_id"
CONFIG_SCHEMA = (
    sensor.sensor_schema(
        HeatCurveClimateSensor,
        unit_of_measurement=UNIT_CELSIUS,
        icon=ICON_THERMOMETER,
        accuracy_decimals=1,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_CLIMATE_ID): cv.use_id(HeatCurveClimate),
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