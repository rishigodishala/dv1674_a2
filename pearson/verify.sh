#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
threads="${THREAD_COUNTS:-1 2 4 8 16 32}"
for item in 128 256 512 1024; do
    ./pearson "data/$item.data" "$tmp/reference.data"
    baseline="../../baseline/pearson/data_o/${item}_baseline.data"
    if [[ -f "$baseline" ]] && ! cmp -s "$baseline" "$tmp/reference.data"; then
        echo "ERROR: sequential output differs from original saved baseline: $item" >&2
        exit 1
    fi
    for count in $threads; do
        ./pearson_par "data/$item.data" "$tmp/parallel.data" "$count"
        if ! cmp -s "$tmp/reference.data" "$tmp/parallel.data"; then
            echo "ERROR: pearson $item differs with $count threads" >&2
            exit 1
        fi
        ./verify "$tmp/reference.data" "$tmp/parallel.data"
        echo "PASS pearson $item threads=$count"
    done
done
echo "All pearson correctness checks passed."
