# Contributing to frisquet-esphome

Thank you for your interest in contributing to this project. Contributions are welcome, whether they are bug reports, documentation improvements, or code changes.

This project aims to eventually be integrated into the official [ESPHome](https://esphome.io) project. All contributions should therefore follow ESPHome's coding conventions and quality standards.

---

## Getting Started

### Prerequisites

- [ESPHome](https://esphome.io/guides/installing_esphome) installed locally
- [PlatformIO](https://platformio.org) for compilation
- An ESP32 development board
- A Frisquet boiler equipped with the Eco Radio System (ERS) module, or access to one for testing

### Setting Up the Development Environment

1. Fork the repository and clone your fork:
   ```bash
   git clone https://github.com/<your-username>/frisquet-esphome.git
   cd frisquet-esphome
   ```

2. Create a branch from `dev` for your changes:
   ```bash
   git checkout dev
   git checkout -b fix/my-fix-description
   ```

3. Copy the `components` folder into your ESPHome configuration directory, or reference it locally in your YAML:
   ```yaml
   external_components:
     - source: components
   ```

---

## Making Changes

### Branching Strategy

- `main` — stable releases only; do not submit pull requests directly to `main`
- `dev` — integration branch; all pull requests should target `dev`

### Commit Message Convention

This project uses [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <short description>
```

Types:
- `feat` — new feature
- `fix` — bug fix
- `docs` — documentation only
- `refactor` — code change that neither fixes a bug nor adds a feature
- `chore` — maintenance (dependencies, tooling, etc.)

Scopes: `frisquet_boiler`, `heat_curve_climate`, `docs`, `yaml`

Examples:
```
fix(frisquet_boiler): add guard against zero calibration factor
feat(heat_curve_climate): add min_delta configuration parameter
docs: clarify operating mode vs component mode distinction
```

---

## Code Style

### C++

This project follows the [ESPHome C++ style guide](https://developers.esphome.io/contributing/development/style-guide/). A `.clang-format` file is provided at the root of the repository.

Before submitting, format your code with:
```bash
clang-format -i components/**/*.cpp components/**/*.h
```

Key conventions:
- 2-space indentation, no tabs
- 120-character line limit
- `snake_case` for variables and methods
- `CamelCase` for class names
- Namespaces: `esphome::<component_name>`
- Use `ESP_LOGD`, `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE` for logging — never `printf` or `Serial`

### Python

Component configuration files (`__init__.py`, `climate.py`, `output.py`) follow the ESPHome Python conventions:
- 4-space indentation
- Follow existing patterns for schema definition and `to_code` functions

### YAML

- YAML example files are illustrative only and not part of the component itself
- Follow ESPHome YAML conventions

---

## Testing

There is currently no automated test suite. Before submitting a pull request, please verify:

- [ ] The component compiles without errors or warnings using ESPHome
- [ ] The component behaves correctly on hardware if you have access to a compatible boiler
- [ ] No regressions are introduced in existing functionality
- [ ] The `boiler.yaml` example file remains valid

If you do not have access to a compatible boiler, please clearly state this in your pull request so that it can be tested before merging.

---

## Documentation

If your change affects the behaviour or configuration of a component, please update the relevant documentation:

- `README.md` — primary user-facing documentation
- `doc/frisquet_boiler.rst` — RST documentation (maintained for future ESPHome integration)
- `doc/heat_curve_climate.rst` — RST documentation (maintained for future ESPHome integration)

RST documentation follows [ESPHome documentation conventions](https://developers.esphome.io/contributing/documentation/).

---

## Submitting a Pull Request

1. Ensure your branch is up to date with `dev`:
   ```bash
   git fetch origin
   git rebase origin/dev
   ```

2. Open a pull request targeting the `dev` branch.

3. In your pull request description, please include:
   - A clear summary of the changes and their motivation
   - Any relevant context (ESPHome version tested, boiler model if applicable)
   - Whether the change has been tested on hardware

---

## Reporting Issues

Please open a [GitHub issue](https://github.com/philippemezzadri/frisquet-esphome/issues) with:
- A clear description of the problem
- Your ESPHome version (`esphome version`)
- Your boiler model and ERS module version if relevant
- Relevant log output (set `level: DEBUG` or `level: VERBOSE` in your logger configuration)

---

## Compatibility Note

This project targets eventual integration into the official ESPHome repository. Contributions should:
- Avoid external dependencies not already present in ESPHome
- Follow ESPHome component architecture (no direct use of Arduino APIs where ESPHome abstractions exist)
- Remain compatible with both ESP32 and ESP8266 where possible

Thank you for contributing!
