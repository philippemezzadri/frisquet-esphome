#!/usr/bin/env bash
# Add the section for an upcoming release to CHANGELOG.md, generated with
# git-cliff from the commits since the last tag.
#
# Usage: scripts/update_changelog.sh <version>   (e.g. 1.5.14 or v1.5.14)
#
# Must be run from the repository root, before the tag is created.
set -euo pipefail

changelog="CHANGELOG.md"
version="${1:?usage: $0 <version, e.g. 1.5.14>}"
version="${version#v}"
tag="v${version}"

if [[ ! "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "error: invalid version '${version}', expected X.Y.Z" >&2
  exit 1
fi
if git rev-parse -q --verify "refs/tags/${tag}" >/dev/null; then
  echo "error: tag ${tag} already exists" >&2
  exit 1
fi
if grep -q "^## \[${version}\]" "$changelog"; then
  echo "error: ${changelog} already has a section for ${version}" >&2
  exit 1
fi

# cat -s squeezes the repeated blank lines produced by the template
section="$(git-cliff --config cliff.toml --unreleased --tag "$tag" --strip all | cat -s)"
if ! grep -q "^### " <<<"$section"; then
  echo "error: no changelog-worthy commits since the last tag" >&2
  exit 1
fi

# Insert the new section before the first existing release section
# ("## [x.y.z]"), so the header can change freely. Append if there is none.
tmp="$(mktemp)"
SECTION="$section" awk '
  !done && /^## \[/ { print ENVIRON["SECTION"]; print ""; done = 1 }
  { print }
  END { if (!done) { print ""; print ENVIRON["SECTION"] } }
' "$changelog" >"$tmp"
mv "$tmp" "$changelog"

echo "Added section ${version} to ${changelog}:"
echo "$section"
