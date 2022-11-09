#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

inline void clflush(volatile void *p) {
  asm volatile ("clflush (%0)" :: "r"(p));
}

// TODO: add pfence to ISA
// inline void pfence() {
//   asm volatile ("pfence");
// }

void clflushTest() {
  int32_t *ptr1 = (int32_t *) malloc(sizeof(int32_t));
  *ptr1 = 10;
  printf("alloc: %p\r\n", ptr1);
  clflush(ptr1);
  printf("ptr1: %d\r\n", *ptr1);
  clflush(ptr1);
  int a = 1;
  int b = 2;
  int c = a + b;
  a /= c;
}

void writeTest() {
  int32_t *ptr1 = (int32_t *) malloc(sizeof(int32_t) * 5000);
  for (int i = 0; i < 5000; i++) {
    ptr1[i] = rand();
    if (i % 16 == 15)
      clflush(&ptr1[i / 16 * 16]);
  }
}

int main() {
  fprintf(stderr, "#CAPTURING_START\r\n");
  clflushTest();
  writeTest();
  fprintf(stderr, "#CAPTURING_END\r\n");
  // pfence();
  return 0;
}
