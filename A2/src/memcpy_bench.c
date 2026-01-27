// src/memcpy_bench.c
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

// Prevent compiler from optimising memcpy away
static volatile uint64_t sink = 0;

static void bench(size_t bytes, int iters) {
    // 64-byte aligned allocations are friendly to cache lines
    unsigned char* src = aligned_alloc(64, bytes);
    unsigned char* dst = aligned_alloc(64, bytes);
    if (!src || !dst) {
        perror("aligned_alloc");
        exit(1);
    }

    // Initialise buffers
    memset(src, 0xAB, bytes);
    memset(dst, 0x00, bytes);

    // Warm-up (cache/JIT effects)
    memcpy(dst, src, bytes);

    double t0 = now_sec();
    for (int i = 0; i < iters; i++) {
        memcpy(dst, src, bytes);
        // Touch a byte so compiler can't remove work
        sink += dst[i & (bytes - 1)];
    }
    double t1 = now_sec();

    double total = t1 - t0;
    double avg = total / iters;
    double gibps = (double)bytes / avg / (1024.0 * 1024.0 * 1024.0);

    printf("memcpy  size=%8zu bytes  iters=%6d  avg=%9.6f s  throughput=%6.3f GiB/s\n",
           bytes, iters, avg, gibps);

    free(src);
    free(dst);
}

int main(void) {
    // Use power-of-two sizes so (bytes-1) mask works
    bench(1024,            200000); // 1 KiB
    bench(1024*1024,          2000); // 1 MiB
    bench(128*1024*1024,        50); // 128 MiB (safe on Pi 4)


    printf("sink=%llu\n", (unsigned long long)sink);
    return 0;
}
