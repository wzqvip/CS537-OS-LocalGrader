#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 2
// Summary: Validates the output of `getpgdirinfo()` before placing any mmap.
//
// This test does not fully check the correctness of all the obtained output.
// ====================================================================

char *test_name = "TEST_2";

// TEST HELPER
#define MMAPBASE 0x60000000
#define KERNBASE 0x80000000
#define PGSIZE 4096

void success() {
    printf(1, "\nWMMAP\t SUCCESS\n\n");
    exit();
}

void failed() {
    printf(1, "\nWMMAP\t FAILED\n\n");
    exit();
}

void get_n_validate_pgdirinfo(struct pgdirinfo *info) {
    info->n_upages = -1;
    int ret = getpgdirinfo(info);
    if (ret < 0) {
        printf(1, "Cause: `getpgdirinfo()` returned %d\n", ret);
        failed();
    }
    if (info->n_upages < 1) {
        printf(1, "Cause: At least one user page expected.\n", info->n_upages);
        failed();
    }
    int found = 0;
    uint addr = 0x0;
    for (int i = 0; i < info->n_upages; i++) {
        if (info->va[i] < 0 || info->va[i] >= KERNBASE) {
            printf(1, "Cause: `info.va[%d]` is 0x%x. It should be within [0, 0x%x)\n", i, info->va[i], KERNBASE);
            failed();
        }
        if (info->va[i] == addr) {
            found = 1;
        }
    }
    if (!found) {
        printf(1, "Cause: `info.va` does not contain 0x%x\n", addr);
        failed();
    }
}

int main() {
    printf(1, "\n\n%s\n", test_name);
    struct pgdirinfo pinfo;
    get_n_validate_pgdirinfo(&pinfo);
    printf(1, "getpgdirinfo \tOkay.\n");
    // test ends
    success();
}
