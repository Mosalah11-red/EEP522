#!/bin/bash
# Usage: ./fs_bench.sh <size_MB>

SIZE_MB="${1:-512}"
OUTFILE="fs_test.bin"

sync
echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null

echo "Writing ${SIZE_MB}MB..."

START=$(date +%s.%N)

dd if=/dev/zero of=$OUTFILE bs=1M count=$SIZE_MB conv=fdatasync status=none

END=$(date +%s.%N)

ELAPSED=$(echo "$END - $START" | bc)

THROUGHPUT=$(echo "scale=2; $SIZE_MB / $ELAPSED" | bc)

echo "Elapsed: $ELAPSED seconds"
echo "Throughput: $THROUGHPUT MB/s"

rm -f $OUTFILE
