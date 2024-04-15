#include "param.h"
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "proc.h"

void log_sched(struct proc* p) {
  if (strncmp(p->name, "initcode", sizeof("initcode")) != 0 &&
      strncmp(p->name, "init", sizeof("init")) != 0 &&
      strncmp(p->name, "sh", sizeof("sh")) != 0)
    cprintf("#%s", p->name);
}
