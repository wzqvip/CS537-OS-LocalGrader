#include "types.h"
#include "user.h"

mutex m1, m2;

void fn1(void* arg) {
  macquire(&m1);
  sleep(1);
  printf(1, "fn1 holds m1\n");
  macquire(&m2);
  printf(1, "fn1 holds m2\n");
  for (int i = 0; i < 100000; i++) {
    if (i % 1000 == 0) {
      sleep(0);
    }
  }
  mrelease(&m1);
  mrelease(&m2);

  exit();
}

void fn2(void* arg) {
  macquire(&m2);
  sleep(1);
  printf(1, "fn2 holds m2\n");
  macquire(&m1);
  printf(1, "fn2 holds m1\n");
  for (int i = 0; i < 100000; i++) {
    if (i % 1000 == 0) {
      sleep(0);
    }
  }
  mrelease(&m1);
  mrelease(&m2);

  exit();
}

int main() {
  minit(&m1);
  minit(&m2);
  char* stack1 = (char*)malloc(4096);
  char* stack2 = (char*)malloc(4096);

  clone(fn1, stack1 + 4096, 0);
  clone(fn2, stack2 + 4096, 0);

  for (int i = 0; i < 100000; i++) {
    if (i % 1000 == 0) {
      sleep(0);
    }
  }

  wait();
  wait();
  exit();
}
