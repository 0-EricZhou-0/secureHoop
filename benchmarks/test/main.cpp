#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

inline void clflush(volatile void *p) {
  asm volatile ("clflush (%0)" :: "r"(p));
}

int main() {
  // fprintf(stderr, "#CAPTURING_START\r\n");
  // int32_t *ptr1 = (int32_t *) malloc(sizeof(int32_t));
  // *ptr1 = 10;
  // printf("alloc: %p\r\n", ptr1);
  // clflush(ptr1);
  // printf("ptr1: %d\r\n", *ptr1);
  // clflush(ptr1);
  // int a = 1;
  // int b = 2;
  // int c = a + b;
  // a /= c;
  // fprintf(stderr, "#CAPTURING_END\r\n");
  return 0;
}
