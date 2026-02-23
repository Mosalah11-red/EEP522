#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static inline uint64_t nsec_now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

typedef struct {
  pthread_mutex_t m;
  pthread_cond_t  cv;
  int turn; // 0 or 1
  int stop;
} shared_t;

typedef struct {
  shared_t *s;
  int id;
  int iters;
  uint64_t *samples_ns; // only used by thread 0 to record RTT
} arg_t;

void *worker(void *p) {
  arg_t *a = (arg_t*)p;
  shared_t *s = a->s;

  if (a->id == 0) {
    for (int i=0; i<a->iters; i++) {
      uint64_t t0 = nsec_now();

      pthread_mutex_lock(&s->m);
      while (s->turn != 0) pthread_cond_wait(&s->cv, &s->m);
      s->turn = 1;
      pthread_cond_signal(&s->cv);
      pthread_mutex_unlock(&s->m);

      pthread_mutex_lock(&s->m);
      while (s->turn != 0) pthread_cond_wait(&s->cv, &s->m);
      pthread_mutex_unlock(&s->m);

      uint64_t t1 = nsec_now();
      a->samples_ns[i] = (t1 - t0); // round-trip time
    }
    pthread_mutex_lock(&s->m);
    s->stop = 1;
    pthread_cond_broadcast(&s->cv);
    pthread_mutex_unlock(&s->m);
  } else {
    while (1) {
      pthread_mutex_lock(&s->m);
      while (s->turn != 1 && !s->stop) pthread_cond_wait(&s->cv, &s->m);
      if (s->stop) { pthread_mutex_unlock(&s->m); break; }
      s->turn = 0;
      pthread_cond_signal(&s->cv);
      pthread_mutex_unlock(&s->m);
    }
  }
  return NULL;
}

static int cmp_u64(const void *a, const void *b) {
  uint64_t x = *(const uint64_t*)a, y = *(const uint64_t*)b;
  return (x>y) - (x<y);
}

static uint64_t percentile(uint64_t *sorted, int n, double p) {
  double idx = p * (n - 1);
  int i = (int)idx;
  double frac = idx - i;
  if (i >= n-1) return sorted[n-1];
  double v = (1.0 - frac) * (double)sorted[i] + frac * (double)sorted[i+1];
  return (uint64_t)v;
}

int main(int argc, char **argv) {
  int iters = 200000;
  if (argc >= 2) iters = atoi(argv[1]);
  if (iters < 1000) iters = 1000;

  shared_t s;
  memset(&s, 0, sizeof(s));
  pthread_mutex_init(&s.m, NULL);
  pthread_cond_init(&s.cv, NULL);
  s.turn = 0;

  uint64_t *samples = (uint64_t*)malloc(sizeof(uint64_t)*iters);
  if (!samples) { perror("malloc"); return 1; }

  pthread_t t0, t1;
  arg_t a0 = {.s=&s,.id=0,.iters=iters,.samples_ns=samples};
  arg_t a1 = {.s=&s,.id=1,.iters=iters,.samples_ns=NULL};

  pthread_create(&t1, NULL, worker, &a1);
  pthread_create(&t0, NULL, worker, &a0);

  pthread_join(t0, NULL);
  pthread_join(t1, NULL);

  uint64_t sum = 0, min = samples[0], max = samples[0];
  for (int i=0;i<iters;i++){
    sum += samples[i];
    if(samples[i]<min)min=samples[i];
    if(samples[i]>max)max=samples[i];
  }
  double mean = (double)sum / (double)iters;

  qsort(samples, iters, sizeof(uint64_t), cmp_u64);
  uint64_t p50 = percentile(samples, iters, 0.50);
  uint64_t p95 = percentile(samples, iters, 0.95);
  uint64_t p99 = percentile(samples, iters, 0.99);

  printf("iters=%d\n", iters);
  printf("RTT_us: min=%.3f mean=%.3f p50=%.3f p95=%.3f p99=%.3f max=%.3f\n",
         min/1000.0, mean/1000.0, p50/1000.0, p95/1000.0, p99/1000.0, max/1000.0);

  free(samples);
  return 0;
}
