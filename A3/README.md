# EEP522 Assignment 3 – System-Level Validation

## Overview

This project validates subsystem characterization (Assignment 2) under realistic multi-resource system load on Raspberry Pi 4.

## Experimental Components

### 1. CPU Stress
Simulates sustained multi-core workload to evaluate scheduler and DVFS behavior.

./scripts/cpu_stress.sh <duration_seconds> <workers>

Example:
./scripts/cpu_stress.sh 120 4

---

### 2. Filesystem Benchmark
Measures sustained sequential write throughput.

./scripts/fs_bench.sh <size_MB>

Example:
./scripts/fs_bench.sh 512

---

### 3. Scheduler Latency (Ping-Pong)
Measures round-trip wake-up latency including p50/p95/p99 metrics.

./bin/pingpong_latency <iterations>

Example:
./bin/pingpong_latency 200000

---

### 4. Governor Control

./scripts/set_governor_performance.sh
./scripts/set_governor_ondemand.sh

Used to evaluate power-management impact on tail latency.

---

## Key Findings

- Filesystem throughput remains storage-bound under CPU load.
- Tail latency increases significantly under sustained stress.
- Performance governor reduces p99 latency by ~40%.


