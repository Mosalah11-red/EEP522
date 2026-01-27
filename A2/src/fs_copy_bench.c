// src/fs_copy_bench.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void die(const char* msg) {
    perror(msg);
    exit(1);
}

static void write_file(const char* path, size_t bytes) {
    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0) die("open(write)");

    const size_t buf_sz = 1 << 20; // 1 MiB buffer
    unsigned char* buf = malloc(buf_sz);
    if (!buf) die("malloc");
    memset(buf, 0xCD, buf_sz);

    size_t left = bytes;
    while (left > 0) {
        size_t chunk = left < buf_sz ? left : buf_sz;
        ssize_t w = write(fd, buf, chunk);
        if (w < 0) die("write");
        if ((size_t)w != chunk) die("short write");
        left -= chunk;
    }

    // force flush to storage
    if (fsync(fd) != 0) die("fsync");
    close(fd);
    free(buf);
}

static void copy_file(const char* src, const char* dst) {
    int in = open(src, O_RDONLY);
    if (in < 0) die("open(src)");

    int out = open(dst, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (out < 0) die("open(dst)");

    const size_t buf_sz = 1 << 20; // 1 MiB buffer
    unsigned char* buf = malloc(buf_sz);
    if (!buf) die("malloc");

    while (1) {
        ssize_t r = read(in, buf, buf_sz);
        if (r < 0) die("read");
        if (r == 0) break;

        ssize_t w = write(out, buf, (size_t)r);
        if (w < 0) die("write");
        if (w != r) die("short write");
    }

    if (fsync(out) != 0) die("fsync(out)");
    close(in);
    close(out);
    free(buf);
}

static void bench(size_t bytes) {
    char src[256], dst[256];
    snprintf(src, sizeof(src), "results/fs_src_%zu.bin", bytes);
    snprintf(dst, sizeof(dst), "results/fs_dst_%zu.bin", bytes);

    // create source file (fresh each run)
    write_file(src, bytes);

    double t0 = now_sec();
    copy_file(src, dst);
    double t1 = now_sec();

    double sec = t1 - t0;
    double mibps = (double)bytes / sec / (1024.0 * 1024.0);

    printf("fs_copy size=%8zu bytes  time=%8.4f s  throughput=%8.2f MiB/s\n",
           bytes, sec, mibps);
}

int main(void) {
    bench(1024);              // 1 KiB
    bench(1024*1024);         // 1 MiB
    bench(128*1024*1024);     // 128 MiB
    return 0;
}
