#!/usr/bin/env bash
# Builds and runs the host-side gesture tests against every sketch that carries
# a copy of the shared headers, and checks those copies have not drifted.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
pedals="$(dirname "$here")"
out="${TMPDIR:-/tmp}/tltr-pedal-tests"
mkdir -p "$out"

sketches=()
for header in "$pedals"/*/ir_gesture.h; do
  sketches+=("$(dirname "$header")")
done

first="${sketches[0]}"
for sketch in "${sketches[@]}"; do
  for shared in inputs.h ir_gesture.h; do
    if ! cmp -s "$first/$shared" "$sketch/$shared"; then
      echo "FAIL: $sketch/$shared differs from $first/$shared" >&2
      exit 1
    fi
  done

  name="$(basename "$sketch")"
  echo "== $name"
  "${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -I "$sketch" \
    "$here/gesture_tests.cpp" -o "$out/$name"
  "$out/$name"
done
