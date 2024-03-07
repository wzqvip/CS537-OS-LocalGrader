#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 5
// Summary: Places one anonymous map at a NOT-fixed address.
//
// Does not access its memory
// Does not unmap any mapping
// Does not check for memory allocation
// Does not explicitly check for lazy allocation
// ====================================================================

char *test_name = "TEST_5";

// TEST HELPER
#define MMAPBASE 0x60000000
#define KERNBASE 0x80000000
#define PGSIZE 4096
#define TRUE 1
#define FALSE 0

void success() {
    printf(1, "\nWMMAP\t SUCCESS\n\n");
    exit();
}

void failed() {
    printf(1, "\nWMMAP\t FAILED\n\n");
    exit();
}

void get_n_validate_wmap_info(struct wmapinfo *info, int expected_total_mmaps) {
    int ret = getwmapinfo(info);
    if (ret < 0) {
        printf(1, "Cause: `getwmapinfo()` returned %d\n", ret);
        failed();
    }
    if (info->total_mmaps != expected_total_mmaps) {
        printf(1, "Cause: expected %d maps, but found %d\n", expected_total_mmaps, info->total_mmaps);
        failed();
    }
}

void map_exists(struct wmapinfo *info, uint addr, int length, int expected) {
    int found = 0;
    for (int i = 0; i < info->total_mmaps; i++) {
        if (info->addr[i] == addr && info->length[i] == length) {
            found = 1;
            break;
        }
    }
    if (found != expected) {
        printf(1, "Cause: expected 0x%x with length %d to %s in the list of maps\n", addr, length, expected ? "exist" : "not exist");
        failed();
    }
}

int main() {
    printf(1, "\n\n%s\n", test_name);

    // validate initial state
    struct wmapinfo winfo1;
    get_n_validate_wmap_info(&winfo1, 0); // no maps exist
    printf(1, "Initially 0 maps. \tOkay.\n");

    // place one map
    int fd = -1;
    int anon = MAP_ANONYMOUS | MAP_SHARED;
    uint addr = 0x60021000;
    uint length = PGSIZE + 8;
    uint map = wmap(addr, length, anon, fd);
    if (map == FAILED) {
        printf(1, "Cause: `wmap()` returned %d\n", map);
        failed();
    }
    if (map % PGSIZE != 0) {
        printf(1, "Cause: expected map at 0x%x, but found at 0x%x\n", addr, map);
        failed();
    }

    // validate final state
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 1);   // 1 map exists
    map_exists(&winfo2, map, length, TRUE); // the map exists
    printf(1, "Map 1 at 0x%x with length %d. \tOkay.\n", map, length);

    // test ends
    success();
}
