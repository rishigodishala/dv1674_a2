#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
threads="${THREAD_COUNTS:-1 2 4 8 16 32}"
for item in im1 im2 im3 im4; do
    ./blur 15 "data/$item.ppm" "$tmp/reference.ppm"
    baseline="../../baseline/blur/data_o/${item}_baseline.ppm"
    if [[ -f "$baseline" ]] && ! cmp -s "$baseline" "$tmp/reference.ppm"; then
        echo "ERROR: sequential output differs from original saved baseline: $item" >&2
        exit 1
    fi
    for count in $threads; do
        ./blur_par 15 "data/$item.ppm" "$tmp/parallel.ppm" "$count"
        if ! cmp -s "$tmp/reference.ppm" "$tmp/parallel.ppm"; then
            echo "ERROR: blur $item differs with $count threads" >&2
            exit 1
        fi
        echo "PASS blur $item threads=$count"
    done
done
echo "All blur correctness checks passed."
