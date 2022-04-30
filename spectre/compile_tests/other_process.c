#include <unistd.h>
#include <stdio.h>
#include <string.h>

char *buffer = "Other process secret value ";

int main() {
  fprintf(stderr, "%p\n", &buffer[0]);

  int junk = 0;
  while(1) {
    junk += buffer[0];
    sleep(1);
  }
}
