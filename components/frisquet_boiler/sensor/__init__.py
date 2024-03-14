import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    ICON_GAUGE,
    CONF_TYPE,
)
from ..output import frisquet_boiler, FrisquetBoiler

FrisquetBoilerSensor = frisquet_boiler.class_(
    "FrisquetBoilerSensor", sensor.Sensor, cg.Component
)
FrisquetBoilerSensorType = frisquet_boiler.enum("FrisquetBoilerSensorType")

FRISQUET_BOILER_SENSOR_TYPES = {
    "SETPOINT": FrisquetBoilerSensorType.BOILER_SENSOR_TYPE_SETPOINT,
    "FLOWTEMP": FrisquetBoilerSensorType.BOILER_SENSOR_TYPE_FLOWTEMP,
}

CONF_BOILER_ID = "friquet_boiler_id"
CONFIG_SCHEMA = (
    sensor.sensor_schema(
        FrisquetBoilerSensor,
        icon=ICON_GAUGE,
        accuracy_decimals=1,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_BOILER_ID): cv.use_id(FrisquetBoiler),
            cv.Required(CONF_TYPE): cv.enum(FRISQUET_BOILER_SENSOR_TYPES, upper=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_BOILER_ID])
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(config[CONF_TYPE]))
