# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [v1.5.12] — 2026-05-25

### Fixed
- `frisquet_boiler`: improved `log_last_message` function — replaced heap-allocated fixed-size buffer with stack-allocated buffer sized from `LONG_MESSAGE_SIZE`; added guard against oversized `length` argument
- `frisquet_boiler`: added validation and error logging when `calibration_factor` is zero to prevent division by zero in `calculate_flow_temperature`
- `heat_curve_climate`: added validation and error logging when `output_calibration_factor` is zero in `setup()`; component is marked as failed on invalid configuration
- `frisquet_boiler`: added explanatory comments on blocking `delay()` calls in `send_message`, `send_test_message` and `send_pairing_message`
- `heat_curve_climate`: integral term is now reset when switching to OFF mode, preventing overshoot on return to HEAT mode
- `heat_curve_climate`: improved boiler shutdown logic and logging
- `frisquet_boiler`, `heat_curve_climate`: refactored and organised imports

### Documentation
- Clarified the distinction between the two mode spaces: operating mode (`0`=eco, `3`=comfort, `4`=away) and component mode (`CONTROL`, `TEST`, `CONFIG`)
- Added warning on the requirement to keep `calibration_factor`/`calibration_offset` and `output_factor`/`output_offset` synchronised between the two components
- Updated `README.md` and RST documentation accordingly

---

## [v1.5.11] — 2026-05-22

### Fixed
- `frisquet_boiler`: corrected boiler ID reference in `dump_config` logging
- `heat_curve_climate`: refactored logging; added default control parameters to configuration dump
- Workspace: updated IntelliSense settings

---

## [v1.5.10] — 2026-05-11

### Fixed
- `frisquet_boiler`: replaced `int` with `int32_t` in `TEMPLATABLE_VALUE` for stricter type safety
- Compatibility with ESPHome 2026.4.0: callback registration methods now use templates
- `register_action`: added `synchronous=True` parameter
- Added missing `platform` key in component configuration

---

## [v1.5.9] — 2025-12-23

### Fixed
- Reverted experimental `USE_SWITCH` conditional compilation (introduced and reverted in this version)

---

## [v1.5.8] — 2025-11-20

### Fixed
- Compatibility fix following ESPHome 2025.11.0 breaking changes in `heat_curve_climate`

---

## [v1.5.7] — 2025-08-26

### Fixed
- `climate.py`: updated to maintain compatibility with latest ESPHome API

### Documentation
- Minor documentation updates

---

## [v1.5.6] — 2024-11-24

### Changed
- Refactored all components to use the `Parented` helper class consistently

### Fixed
- Various documentation and YAML file corrections
- Applied suggestions from contributor @tgdl
- Updated RST documentation files

---

## [v1.5.5] — 2024-11-22

### Added
- `frisquet_boiler`: implemented pairing message for configuration mode
- `frisquet_boiler`: added test and pairing switches (replacing buttons)
- Updated Home Assistant automation package sample file

### Fixed
- Various bug fixes in `frisquet_boiler` signal generation
- Fixed message logging

### Documentation
- Improved project and component documentation
- Clarified low-value output behaviour and tips for finding `boiler_id`
- Made explicit the link between `output_factor`/`output_offset` values (contribution from @tgdl)

---

## [v1.5.4] — 2024-11-07

### Added
- `heat_curve_climate`: `max_error` and `min_delta` parameters can now be set in the YAML configuration file
- `heat_curve_climate`: boiler is now forced to IDLE when ambient temperature exceeds the maximum error limit

### Fixed
- Various bug fixes in `heat_curve_climate`
- Updated default boiler sensor values
- Improved logging

---

## [v1.5.3] — 2024-03-19

### Added
- `frisquet_boiler`: added internal sensor platform (`SETPOINT`, `FLOWTEMP`) — flow temperature is now calculated by the output component
- `heat_curve_climate`: added proportional deadband multiplier (`kp` deadband)

### Fixed
- Bug fixes and logging improvements

---

## [v1.5.2] — 2024-03-10

### Added
- `heat_curve_climate`: introduced alternate heating curve (`alt_curve`) — polynomial curve providing reduced slope at high temperature deltas

### Fixed
- Code cleanup and clang-format compliance
- Fixed `LOW` and `HIGH` define conflicts
- Removed unused weighted temperature component

### Documentation
- Improved and corrected RST documentation
- Fixed links and formatting

---

## [v1.5.1] — 2024-03-01

### Fixed
- Updated `api.yaml` common configuration

---

## [v1.5.0] — 2024-02-29

### Breaking Changes
- `heat_curve_climate`: parameters `heat_factor` and `offset` renamed to `slope` and `shift` respectively — existing YAML configurations must be updated
- Output levels are now managed as floats instead of integers
- Repository cleanup and overall refactoring

### Added
- Output rounding is now optional (`rounded` parameter in `output_parameters`)
- Detailed heating curve documentation

### Fixed
- Bug fix in `climate.py`
- Various documentation corrections

---

## [v1.4.2] — 2024-02-27

### Added
- `heat_curve_climate`: added `maximum_output` cap and `heat_required_output` minimum power parameters
- Improved integral wind-up prevention

### Fixed
- Documentation and integral wind-up patch (contribution from @lexyan)

---

## [v1.4.1] — 2024-01-30

### Fixed
- Patched memory leak
- Code cleanup

---

## [v1.4] — 2023-03-05

### Added
- `heat_curve_climate`: *Heat Required* switch — forces a minimum output level when enabled, preventing the boiler from shutting off completely
- Debug sensors for monitoring internal state

---

## [v1.3] — 2023-01-27

### Added
- `heat_curve_climate`: `default_target_temperature` configuration parameter
- `heat_curve_climate`: additional sensor types and integral factor (`ki`)

### Fixed
- Bug fixes in climate control loop

---

## [v1.2] — 2022-12-22

### Added
- Home Assistant integration documentation
- ESPHome automation examples

### Changed
- Code refactoring in `heat_curve_climate`

### Fixed
- Documentation typos and formatting

---

## [v1.1] — 2022-11-30

### Added
- Boiler ID configurable in YAML file
- Additional sensor types
- Home Assistant service registration

### Fixed
- Removed LED blink (blocking `delay()` in `Component::loop()` not allowed)
- Skip calculation when outdoor temperature is `NAN`
- Pin number configuration; removed `Arduino.h` dependency

### Changed
- Separated output parameters from control parameters
- Code cleanup

---

## [V1.0] — 2022-11

### Added
- Initial release
- `frisquet_boiler` Float Output component — communicates with Frisquet ERS boilers via differential Manchester encoding
- `heat_curve_climate` Climate component — heating curve control using outdoor temperature sensor
- Support for ESP8266 and ESP32 (Arduino framework)
