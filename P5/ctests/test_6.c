#include "types.h"
#include "user.h"


void fn1(void* arg) {
  for (int i = 0; i < 10000; i++) {
    if (i % 1000 == 0) {
      sleep(0);
    }
  }

  exit();
}

int main() {
  char* stack1 = (char*)malloc(4096);
  clone(fn1, stack1 + 4096, 0);
  nice(-20);
  for (int i = 0; i < 10000; i++) {
    if (i % 1000 == 0) {
      sleep(0);
    }
  }

  wait();
  exit();
}
