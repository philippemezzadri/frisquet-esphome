#!/usr/bin/env bash
# Print the CHANGELOG.md section of a release, without its "## [x.y.z]" title,
# to be used as GitHub release notes.
#
# Usage: scripts/release_notes.sh <version>   (e.g. 1.5.14 or v1.5.14)
set -euo pipefail

changelog="CHANGELOG.md"
version="${1:?usage: $0 <version, e.g. 1.5.14>}"
version="${version#v}"

notes="$(VERSION="$version" awk '
  index($0, "## [" ENVIRON["VERSION"] "]") == 1 { found = 1; next }
  found && /^## \[/ { exit }
  found { print }
' "$changelog")"

if [[ -z "${notes//[[:space:]]/}" ]]; then
  echo "error: no section for ${version} in ${changelog}." >&2
  echo "Run the 'Prepare release' workflow before tagging." >&2
  exit 1
fi

printf '%s\n' "$notes"
