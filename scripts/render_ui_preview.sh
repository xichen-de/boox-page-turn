#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/boox-remote-preview.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

cmake -S "$project_dir/test/host/ui_preview" -B "$work_dir/build" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$work_dir/build" --target render_boox_ui --parallel
"$work_dir/build/render_boox_ui" "$work_dir/ui-preview.ppm"

output="$project_dir/docs/ui-preview.png"
if command -v magick >/dev/null 2>&1; then
    magick "$work_dir/ui-preview.ppm" -strip \
        -define png:exclude-chunk=date,time "$output"
elif command -v sips >/dev/null 2>&1; then
    sips -s format png "$work_dir/ui-preview.ppm" --out "$output" >/dev/null
elif command -v convert >/dev/null 2>&1; then
    convert "$work_dir/ui-preview.ppm" -strip \
        -define png:exclude-chunk=date,time "$output"
else
    echo "Install ImageMagick or run this script on macOS (sips is required)." >&2
    exit 1
fi

echo "Updated $output"
