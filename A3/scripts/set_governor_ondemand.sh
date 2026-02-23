#!/bin/bash
set -e
for g in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
  echo ondemand | sudo tee "$g" > /dev/null
done
echo "Now:"
for g in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
  echo "$g: $(cat "$g")"
done
