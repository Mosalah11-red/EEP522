# EEP522 Assignment 2 – Raspberry Pi Characterization

This repository contains C benchmarks used to characterize a Raspberry Pi
platform for EEP522.

## Structure
- src/      C source files
- bin/      Compiled binaries
- results/  Benchmark output (generated at runtime)

## Benchmarks
- memcpy_bench.c  
  Measures main memory copy throughput using memcpy.

- fs_copy_bench.c  
  Measures filesystem copy throughput using buffered read/write and fsync.

## Build
```bash
make
