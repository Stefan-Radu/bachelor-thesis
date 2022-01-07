#include <stdio.h>

int main() {
	char *kernel_secret_address = (char *) 0xf88a8000;
	char kernel_secret = *kernel_secret_address;
	printf("code reached this point without exceptions");
}
