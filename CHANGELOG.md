# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [1.5.13] — 2026-05-27

### Added

- **docs**: Add CONTRIBUTING.md to guide new contributors
- **docs**: CHANGELOG.md updated by GitHub Actions

### Fixed

- **frisquet_boiler**: Improve command delay handling
- **frisquet_boiler**: Add validation checks for operating mode and setpoint in `send_message`
- **frisquet_boiler**: Add initialization check in write_state to ensure proper startup behaviour

---

## [1.5.12] — 2026-05-25

### Fixed

- **frisquet_boiler**: improved `log_last_message` function — replaced heap-allocated fixed-size buffer with stack-allocated buffer sized from `LONG_MESSAGE_SIZE`; added guard against oversized `length` argument
- **frisquet_boiler**: added validation and error logging when `calibration_factor` is zero to prevent division by zero in `calculate_flow_temperature`
- **heat_curve_climate**: added validation and error logging when `output_calibration_factor` is zero in `setup()`; component is marked as failed on invalid configuration
- **frisquet_boiler**: added explanatory comments on blocking `delay()` calls in `send_message`, `send_test_message` and `send_pairing_message`
- **heat_curve_climate**: integral term is now reset when switching to OFF mode, preventing overshoot on return to HEAT mode
- **heat_curve_climate**: improved boiler shutdown logic and logging
- **frisquet_boiler**, **heat_curve_climate**: refactored and organised imports

### Documentation

- Clarified the distinction between the two mode spaces: operating mode (`0`=eco, `3`=comfort, `4`=away) and component mode (`CONTROL`, `TEST`, `CONFIG`)
- Added warning on the requirement to keep `calibration_factor`/`calibration_offset` and `output_factor`/`output_offset` synchronised between the two components
- Updated `README.md` and RST documentation accordingly

---

## [1.5.11] — 2026-05-22

### Fixed

- **frisquet_boiler**: corrected boiler ID reference in `dump_config` logging
- **heat_curve_climate**: refactored logging; added default control parameters to configuration dump

---

## [1.5.10] — 2026-05-11

### Fixed

- **frisquet_boiler**: replaced `int` with `int32_t` in `TEMPLATABLE_VALUE` for stricter type safety
- **frisquet_boiler**, **heat_curve_climate**: Compatibility with ESPHome 2026.4.0: callback registration methods now use templates
- **frisquet_boiler**, **heat_curve_climate**: Added `synchronous=True` parameter in `register_action`
- **yaml**: Added missing `platform` key in component configuration

---

## [1.5.9] — 2025-12-23

### Fixed

- **frisquet_boiler**: testing `USE_SWITCH` for conditional compilation

---

## [1.5.8] — 2025-11-20

### Fixed

- **heat_curve_climate**: Compatibility fix following ESPHome 2025.11.0 breaking changes

---

## [1.5.7] — 2025-08-26

### Fixed

- **heat_curve_climate**: `climate.py` updated to maintain compatibility with latest ESPHome API

### Documentation

- Minor documentation updates

---

## [1.5.6] — 2024-11-24

### Changed

- **frisquet_boiler**, **heat_curve_climate**: Refactored all components to use the `Parented` helper class consistently

### Documentation

- Various documentation and YAML file corrections
- Applied suggestions from contributor @tgdl
- Updated RST documentation files

---

## [1.5.5] — 2024-11-22

### Added

- **frisquet_boiler**: implemented pairing message for configuration mode
- **frisquet_boiler**: added test and pairing switches (replacing buttons)
- **yaml**: Updated Home Assistant automation package sample file

### Fixed

- **frisquet_boiler**: Various bug fixes in signal generation
- **frisquet_boiler**: Fixed message logging

### Documentation

- Improved project and component documentation
- Clarified low-value output behaviour and tips for finding `boiler_id`
- Made explicit the link between `output_factor`/`output_offset` values (contribution from @tgdl)

---

## [1.5.4] — 2024-11-07

### Added

- **heat_curve_climate**: `max_error` and `min_delta` parameters can now be set in the YAML configuration file
- **heat_curve_climate**: boiler is now forced to IDLE when ambient temperature exceeds the maximum error limit

### Fixed

- **heat_curve_climate**: Various bug fixes
- **frisquet_boiler**: Updated default boiler sensor values
- **frisquet_boiler**, **heat_curve_climate**: Improved logging

---

## [1.5.3] — 2024-03-19

### Added

- **frisquet_boiler**: added internal sensor platform (`SETPOINT`, `FLOWTEMP`) — flow temperature is now calculated by the output component
- **heat_curve_climate**: added proportional deadband multiplier (`kp` deadband)

### Fixed

- **frisquet_boiler**, **heat_curve_climate**: Bug fixes and logging improvements

---

## [1.5.2] — 2024-03-10

### Added

- **heat_curve_climate**: introduced alternate heating curve (`alt_curve`) — polynomial curve providing reduced slope at high temperature deltas

### Fixed

- **frisquet_boiler**, **heat_curve_climate**: Code cleanup and clang-format compliance
- **frisquet_boiler**: Fixed `LOW` and `HIGH` define conflicts
- **heat_curve_climate**: Removed unused weighted temperature component

### Documentation

- Improved and corrected RST documentation
- Fixed links and formatting

---

## [1.5.1] — 2024-03-01

### Fixed

- **yaml**: Updated `api.yaml` common configuration

---

## [1.5.0] — 2024-02-29

### Breaking Changes

- **heat_curve_climate**: parameters `heat_factor` and `offset` renamed to `slope` and `shift` respectively — existing YAML configurations must be updated
- **heat_curve_climate**: Output levels are now managed as floats instead of integers

### Added

- **heat_curve_climate**: Output rounding is now optional (`rounded` parameter in `output_parameters`)
- **heat_curve_climate**: Detailed heating curve documentation

### Fixed

- **heat_curve_climate**: Bug fix in `climate.py`
- **docs**: Various documentation corrections
- **frisquet_boiler**, **heat_curve_climate**: Repository cleanup and overall refactoring

---

## [1.4.2] — 2024-02-27

### Added

- **heat_curve_climate**: added `maximum_output` cap and `heat_required_output` minimum power parameters
- **heat_curve_climate**: Improved integral wind-up prevention

### Fixed

- **heat_curve_climate**: Documentation and integral wind-up patch (contribution from @lexyan)

---

## [1.4.1] — 2024-01-30

### Fixed

- **frisquet_boiler**: Patched memory leak
- **heat_curve_climate**: Code cleanup

---

## [1.4] — 2023-03-05

### Added

- **heat_curve_climate**: `Heat Required` switch — forces a minimum output level when enabled, preventing the boiler from shutting off completely
- **yaml**: Debug sensors for monitoring internal state

---

## [1.3] — 2023-01-27

### Added

- **heat_curve_climate**: `default_target_temperature` configuration parameter
- **heat_curve_climate**: additional sensor types and integral factor (`ki`)

### Fixed

- **heat_curve_climate**: Bug fixes in climate control loop

---

## [1.2] — 2022-12-22

### Added

- **frisquet_boiler**, **heat_curve_climate**: Home Assistant integration documentation
- **yaml**: ESPHome automation examples

### Changed

- **heat_curve_climate**: Code refactoring

### Fixed

- **dos**: Documentation typos and formatting

---

## [1.1] — 2022-11-30

### Added

- **frisquet_boiler**: Boiler ID configurable in YAML file
- **heat_curve_climate**: Additional sensor types
- **frisquet_boiler**, **heat_curve_climate**: Home Assistant service registration

### Fixed

- **frisquet_boiler**: Removed LED blink (blocking `delay()` in `Component::loop()` not allowed)
- **heat_curve_climate**: Skip calculation when outdoor temperature is `NAN`
- **frisquet_boiler**: Pin number configuration; removed `Arduino.h` dependency

### Changed

- **heat_curve_climate**: Separated output parameters from control parameters
- **frisquet_boiler**, **heat_curve_climate**: Code cleanup

---

## [1.0] — 2022-11

### Added

- Initial release
- `frisquet_boiler` Float Output component — communicates with Frisquet ERS boilers via differential Manchester encoding
- `heat_curve_climate` Climate component — heating curve control using outdoor temperature sensor
- Support for ESP8266 and ESP32 (Arduino framework)
