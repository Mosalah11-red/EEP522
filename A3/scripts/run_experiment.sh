#!/bin/bash
set -euo pipefail

RUN_ID=$(date +"%Y%m%d_%H%M%S")
OUTDIR="$HOME/A3/results/$RUN_ID"
mkdir -p "$OUTDIR"

echo "Run ID: $RUN_ID"
echo "Output dir: $OUTDIR"

# Snapshot basic system info (useful for report + reproducibility)
{
  echo "===== DATE ====="
  date
  echo "===== UNAME ====="
  uname -a
  echo "===== CPUINFO (model) ====="
  awk -F: '/Model/{print $2; exit}' /proc/cpuinfo || true
  echo "===== GOVERNOR ====="
  cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor || true
} > "$OUTDIR/system_info.txt"

# Start monitor in background
MONITOR_OUT="$OUTDIR/monitor.csv"
"$HOME/A3/scripts/monitor.sh" "$MONITOR_OUT" 1 &
MON_PID=$!
echo "Monitor PID: $MON_PID"
sleep 2

echo "== Warm-up 30s (light load) =="
sleep 30

echo "== Workload: CPU stress 5 min =="
{ time -p "$HOME/A3/scripts/cpu_stress.sh" 300 4; } 2> "$OUTDIR/workload_time.txt"

echo "== Cooldown 60s =="
sleep 60

# Stop monitor
kill "$MON_PID"
wait "$MON_PID" 2>/dev/null || true

echo "Done. Collected: $MONITOR_OUT"
