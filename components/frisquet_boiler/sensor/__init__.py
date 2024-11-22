import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    ICON_THERMOMETER,
    CONF_TYPE,
)
from ..output import CONF_FRISQUETBOILER_ID, frisquet_boiler_ns, FrisquetBoiler

FrisquetBoilerSensor = frisquet_boiler_ns.class_(
    "FrisquetBoilerSensor", sensor.Sensor, cg.Component
)
FrisquetBoilerSensorType = frisquet_boiler_ns.enum("FrisquetBoilerSensorType")

FRISQUET_BOILER_SENSOR_TYPES = {
    "SETPOINT": FrisquetBoilerSensorType.BOILER_SENSOR_TYPE_SETPOINT,
    "FLOWTEMP": FrisquetBoilerSensorType.BOILER_SENSOR_TYPE_FLOWTEMP,
}

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        FrisquetBoilerSensor,
        unit_of_measurement=UNIT_CELSIUS,
        icon=ICON_THERMOMETER,
        accuracy_decimals=1,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_FRISQUETBOILER_ID): cv.use_id(FrisquetBoiler),
            cv.Required(CONF_TYPE): cv.enum(FRISQUET_BOILER_SENSOR_TYPES, upper=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_FRISQUETBOILER_ID])
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    cg.add(var.set_parent(parent))
    cg.add(var.set_type(config[CONF_TYPE]))
