#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

int main() {
  unsigned int junk=0;
  register uint64_t time, cache_time, memory_time;

  uint32_t value = 10;
  volatile uint32_t *addr;

  int tries = 10000, avg1 = 0, avg2 = 0;
  for (int i = 0; i < tries; ++i) {
    _mm_clflush(&value);

    addr = &value;
    time = __rdtscp(&junk);
    junk = *addr;
    cache_time = __rdtscp(&junk) - time;
    avg1 += (int)cache_time;

    time = __rdtscp(&junk);
    junk = *addr;
    memory_time = __rdtscp(&junk) - time;
    avg2 += (int)memory_time;
  }

  avg1 /= tries;
  avg2 /= tries;

  printf("Incarcare din memorie: %d\n", (int) avg1); 
  printf("Incarcare din cache: %d\n", (int) avg2); 
}
