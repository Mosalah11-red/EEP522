#!/bin/bash
# Usage: ./cpu_stress.sh <seconds> [workers]
DUR="${1:-300}"
WORKERS="${2:-4}"
end=$(( $(date +%s) + DUR ))

for i in $(seq 1 "$WORKERS"); do
  ( while [ "$(date +%s)" -lt "$end" ]; do :; done ) &
done

wait
