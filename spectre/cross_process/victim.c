#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/file.h>
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
int fd_lock;

unsigned int array1_size = 16, secret_size = 2048;
uint8_t unused1[64];
uint8_t array1[160] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
uint8_t unused2[64]; 
uint8_t *array2;

char secret[2048];

const char *lock_file_name = "spectre.lock";
const char *shared_memory_name = "shared_mem";
const char *index_file_name = "index.txt";


int load_secret(const char* file_name) {
	FILE *f = fopen(file_name, "r");
	if (f == NULL) {
		return -1;
	}
	fread(secret, sizeof(*secret), secret_size, f);
	fclose(f);
	return 0;
}

void victim_function(size_t x) {
	static uint8_t temp = 0;  /* Used so compiler won't optimize out victim_function() */
	if (x < array1_size) {
		// some action performed on array2 with index proportional to x in array1
		temp &= array2[array1[x] * 512];
	}
}

void readMemoryByte() {
	// acquire lock
	static int offset, locked, no_items;
	static size_t buffer[64], buffer_size = 64;

	// wait for file containing indexes to appear
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

	fscanf(f, "%d", &no_items);
	if (no_items > buffer_size)
		no_items = buffer_size;

	for (int i = 0; i < no_items; ++i) {
		fscanf(f, "%zu", &buffer[i]);
	}

	for (int i = 0; i < no_items; ++i) {
		_mm_clflush(&array1_size);
		for (volatile int z = 0; z < 100; z++) {}  /* Delay (can also mfence) */
		victim_function(buffer[i]);
	}

	fclose(f);
	remove(index_file_name);

	// release lock
	unlink(lock_file_name);
	flock(fd_lock, LOCK_UN);
	close(fd_lock);
}

int main(int argc, const char **argv) {
	// map shared memory area
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

	if (argc == 2) {
		if (load_secret(argv[1]) == -1) {
			printf("could not open file");
			return 0;
		}
	} else {
		const char *message = "The secret message is NOT squeamish ossifrage.";
		memcpy(secret, message, strlen(message));
	}

	while (1) readMemoryByte();

	return 0;
}
