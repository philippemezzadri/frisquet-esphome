CODEOWNERS = ["@philippemezzadri"]

import esphome.codegen as cg

CONF_FRISQUETBOILER_ID = "frisquet_boiler_id"

frisquet_boiler_ns = cg.esphome_ns.namespace("frisquet_boiler")
FrisquetBoiler = frisquet_boiler_ns.class_("FrisquetBoiler")
