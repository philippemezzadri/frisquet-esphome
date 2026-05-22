CODEOWNERS = ["@philippemezzadri"]

import esphome.codegen as cg
from esphome.components import climate

climate_ns = cg.esphome_ns.namespace("climate")
heat_curve_ns = climate_ns.namespace("heat_curve")
HeatingCurveClimate = heat_curve_ns.class_(
    "HeatingCurveClimate", climate.Climate, cg.Component
)
