#include "types.h"
#include "user.h"

mutex m;

void fn1(void* arg) {
  nice(10);
  macquire(&m);
  for (int i = 0; i < 100000; i++) {
    if (i % 1000 == 0) {
      sleep(0);
    }
  }
  mrelease(&m);

  exit();
}

void fn2(void* arg) {
  sleep(1);
  nice(-10);
  macquire(&m);
  for (int i = 0; i < 100000; i++) {
    if (i % 1000 == 0) {
      sleep(0);
    }
  }
  mrelease(&m);

  exit();
}

int main() {
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
