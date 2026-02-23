#!/bin/bash
# Usage: ./monitor.sh <output_csv> [interval_seconds]
# Example: ./monitor.sh results/monitor.csv 1

OUT="${1:-results/monitor.csv}"
INTERVAL="${2:-1}"

mkdir -p "$(dirname "$OUT")"

if [ ! -f "$OUT" ]; then
  echo "timestamp,unixtime,governor,freq_khz,temp_c,load1,mem_avail_kb,io_wait_pct" > "$OUT"
fi

read_iowait() {
  # Read /proc/stat twice to estimate iowait percentage over a short window
  # Returns integer percent (0-100)
  local a b
  a=$(awk '/^cpu /{print $5,$6,$7,$8,$9,$10,$11,$12}' /proc/stat)
  sleep 0.2
  b=$(awk '/^cpu /{print $5,$6,$7,$8,$9,$10,$11,$12}' /proc/stat)

  # Fields: user nice system idle iowait irq softirq steal
  # We compute delta total and delta iowait
  awk -v a="$a" -v b="$b" '
    BEGIN{
      split(a, A, " ");
      split(b, B, " ");
      idleA=A[4]; iowA=A[5];
      idleB=B[4]; iowB=B[5];

      totalA=0; totalB=0;
      for(i=1;i<=8;i++){ totalA+=A[i]; totalB+=B[i]; }

      dTotal=totalB-totalA;
      dIow=iowB-iowA;

      if(dTotal<=0){ print 0; exit; }
      pct = (dIow*100)/dTotal;
      if(pct<0) pct=0;
      if(pct>100) pct=100;
      printf("%d\n", pct);
    }'
}

while true; do
  TS=$(date +"%Y-%m-%d %H:%M:%S")
  UTS=$(date +"%s")

  GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "NA")
  FREQ=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq 2>/dev/null || echo "NA")

  TEMP_RAW=$(cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null || echo "NA")
  if [ "$TEMP_RAW" = "NA" ]; then
    TEMP="NA"
  else
    TEMP=$(awk -v t="$TEMP_RAW" 'BEGIN{printf("%.1f", t/1000.0)}')
  fi

  LOAD1=$(awk '{print $1}' /proc/loadavg)
  MEMAVAIL=$(awk '/MemAvailable/ {print $2}' /proc/meminfo)

  IOWAIT=$(read_iowait)

  echo "$TS,$UTS,$GOV,$FREQ,$TEMP,$LOAD1,$MEMAVAIL,$IOWAIT" >> "$OUT"
  sleep "$INTERVAL"
done
