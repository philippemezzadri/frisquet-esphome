# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [1.5.13] — 2026-05-27

### Added
- **docs**: Add CONTRIBUTING.md to guide new contributors
- **docs**: Add CONTRIBUTING.md to guide new contributors
- **changelog**: Add changelog generation workflow and configuration


### Fixed
- **frisquet_boiler**: Improve command delay handling
- **frisquet_boiler**: Add validation checks for operating mode and setpoint in send_message
- **frisquet_boiler**: Add initialization check in write_state to ensure proper startup behaviour
- **changelog**: Fix changelog generation steps
- **changelog**: Update git-cliff download URL to specific version
- **changelog**: Fix installation command in changelog workflow
- **changelog**: Fix installation command in changelog workflow (2)
- **changelog**: Fix installation command in changelog workflow (3)


## [1.5.9] — 2025-12-23

### Added
- **frisquet_boiler, heat_curve_climate**: Refactor and organize imports


### Fixed
- **frisquet_boiler**: Replace int with int32_t in TEMPLATABLE_VALUE
- **workspace**: IntelliSense settings
- **climate**: Refactor logging and add default control parameters
- **frisquet_boiler**: Correct boiler ID reference in dump_config logging
- **heat_curve_climate**: Improve boiler shutdown logic and logging
- **frisquet_boiler**: Improve log_last_message function to handle long messages and optimize memory usage
- **frisquet_boiler, heat_curve_climate**: Add validation and error logging for zero calibration factor
- **docs**: Clarify parameter requirements and mode distinctions for frisquet_boiler and heat_curve_climate


## [1.5.5] — 2024-11-22

### [frisquet_boiler]
- Clarify low-value output and tips for boiler_id


### [heat_curve_climate]
- Make explicit link between factor and offset values


## [1.4.2] — 2024-02-27


