#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>

uint8_t array[10*4096];

const float threshold = 0.6;

#define THRESHOLD 100

int main(int argc, const char **argv) {
  unsigned int junk=0;
  register uint64_t time1, time2;
  volatile uint8_t *addr;
  int i;
  
  // Initialize the array
  for(i=0; i<10; i++) array[i*4096]=1;

  // FLUSH the array from the CPU cache
  for(i=0; i<10; i++) _mm_clflush(&array[i*4096]);

  // Access some of the array items
  array[3*4096] = 100;
  array[7*4096] = 200;

  /* float sum = 0; */
  /* int arr[10]; */

  for(i=0; i<10; i++) {
    addr = &array[i*4096];
    time1 = __rdtscp(&junk);
    junk = *addr;
    time2 = __rdtscp(&junk) - time1;
    /* sum += time2; */
    /* arr[i] = time2; */
    printf("Access time for array[%d*4096]: %d CPU cycles\n",i, (int)time2);
    if (time2 < THRESHOLD) {
      printf("Cache hit at i = %d\n", i);
    }
  }

  /* float avg = sum / 10; */
  /* printf("Average time: %0.2f\n", avg); */
  /*  */
  /* for (int i = 0; i < 10; ++i) { */
  /*   if (arr[i] <= avg * threshold) { */
  /*     printf("Cache hit at i = %d\n", i); */
  /*   } */
  /* } */

  return 0;
}
