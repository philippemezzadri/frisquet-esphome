#!/usr/bin/env bash
# Generate .vscode/compile_commands.json so that VS Code C/C++ IntelliSense
# resolves ESPHome, ESP-IDF and framework headers with the exact flags used
# by the ESPHome build. No machine-specific path is stored in the repository.
#
# Usage: scripts/setup_vscode.sh
#
# Requires ESPHome installed (the `esphome` command, or set ESPHOME to its path).
# Run it once, then again after upgrading ESPHome or changing tests/test.esp32.yaml.
set -euo pipefail

cd "$(dirname "$0")/.."
root="$(pwd)"
esphome="${ESPHOME:-esphome}"
config="tests/test.esp32.yaml"
build_dir="tests/.esphome/build/frisquet-ci"
output=".vscode/compile_commands.json"

if ! command -v "$esphome" >/dev/null 2>&1; then
  echo "error: '${esphome}' not found. Install ESPHome or set ESPHOME=/path/to/esphome" >&2
  exit 1
fi

echo "Compiling ${config} with $("$esphome" version) (first run takes a few minutes)..."
"$esphome" compile "$config"

# The build compiles copies of the components located under
# <build>/src/esphome/components/. Point those entries to the sources of this
# repository so that IntelliSense applies them to the files being edited.
python3 - "$root" "$build_dir" "$output" <<'EOF'
import json
import os
import sys

root, build_dir, output = sys.argv[1:]
components = os.path.join(root, "components")
copies = os.path.join(root, build_dir, "src", "esphome", "components")
database = os.path.join(root, build_dir, "build", "compile_commands.json")

with open(database, encoding="utf-8") as f:
    entries = json.load(f)

remapped = 0
for entry in entries:
    path = entry["file"]
    if not os.path.isabs(path):
        path = os.path.normpath(os.path.join(entry["directory"], path))
    rel = os.path.relpath(path, copies)
    if rel.startswith(".."):
        continue
    source = os.path.join(components, rel)
    if not os.path.isfile(source):
        continue  # ESPHome built-in component, keep the build copy
    for key in ("command", "arguments"):
        if key in entry:
            value = entry[key]
            entry[key] = (
                value.replace(entry["file"], source)
                if isinstance(value, str)
                else [a.replace(entry["file"], source) for a in value]
            )
    entry["file"] = source
    remapped += 1

if remapped == 0:
    sys.exit("error: no entry of the build matches components/, nothing remapped")

os.makedirs(os.path.dirname(os.path.join(root, output)), exist_ok=True)
with open(os.path.join(root, output), "w", encoding="utf-8") as f:
    json.dump(entries, f, indent=1)

print(f"Wrote {output}: {len(entries)} entries, {remapped} mapped to components/")
EOF

echo "Done. Reload the VS Code window if IntelliSense does not pick it up."
