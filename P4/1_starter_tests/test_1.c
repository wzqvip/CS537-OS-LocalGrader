#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 1
// Summary: Checks the presence of all system calls
//
// Description:
// This test checks the presence of system calls: wmap()`, `wunmap()`, `getpgdirinfo()`, `getwmapinfo()`,
// and `wremap()` by calling them.
// Does not verify the output of those calls.
// ====================================================================

char *test_name = "TEST_1";

// Test Helpers
#define MMAPBASE 0x60000000
#define KERNBASE 0x80000000
#define PGSIZE 4096

void success() {
    printf(1, "\n%s\t SUCCESS\n\n", test_name);
    exit();
}

void failed() {
    printf(1, "\n%s\t FAILED\n\n", test_name);
    exit();
}

// Test Functions
void test_getpgdirinfo(void) {
    struct pgdirinfo info;
    info.n_upages = -1;
    int ret = getpgdirinfo(&info);
    printf(1, "`getpgdirinfo()` returned %d\n", ret);
}

void test_wmap(void) {
    uint addr = 0;
    int length = 100;
    int flags = MAP_ANONYMOUS | MAP_PRIVATE;
    int fd = 0;
    int ret = wmap(addr, length, flags, fd);
    printf(1, "`wmap()` returned %d\n", ret);
}

void test_wunmap(void) {
    uint addr = 0;
    int ret = wunmap(addr);
    printf(1, "`wunmap()` returned %d\n", ret);
}

void test_getwmapinfo(void) {
    struct wmapinfo info;
    info.total_mmaps = -1;
    int ret = getwmapinfo(&info);
    printf(1, "`getwmapinfo()` returned %d\n", ret);
}

void test_wremap(void) {
    uint addr = MMAPBASE;
    int ret = wremap(addr, 1, 1, 0);
    printf(1, "`wremap()` returned %d\n", ret);
}

int main() {
    printf(1, "\n\n%s\n", test_name);

    //
    // Call `getpgdirinfo()`
    //
    test_getpgdirinfo();

    //
    // Call `wmap()`
    //
    test_wmap();

    //
    // Call `wunmap()`
    //
    test_wunmap();

    //
    // Call `getwmapinfo()`
    //
    test_getwmapinfo();

    //
    // Call `wremap()`
    //
    test_wremap();

    // test ends
    success();
}
