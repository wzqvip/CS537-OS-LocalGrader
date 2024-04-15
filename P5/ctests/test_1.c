#include "types.h"
#include "user.h"

int shared_variable = 0;
mutex m;
void fn1(void* arg) {
    macquire(&m);
  for (int i = 0; i < 10000000; i++) {
    ++shared_variable;
  }
    mrelease(&m);
  exit();
}

void fn2(void* arg) {
    macquire(&m);
  for (int i = 0; i < 10000000; i++) {
    ++shared_variable;
  }
    mrelease(&m);
  exit();
}

int main() {
  minit(&m);
  char* stack1 = (char*)malloc(4096);
  char* stack2 = (char*)malloc(4096);

  clone(fn1, stack1 + 4096, 0);
  clone(fn2, stack2 + 4096, 0);

  wait();
  wait();
  printf(1,"%d",shared_variable);
  exit();
}
