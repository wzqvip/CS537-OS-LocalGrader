#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 30
// Summary: Remaps an anonymous mapping (shrink twice)
//
// Does not check for the page contents to be intact
// Does not check for deallocation of pages
// Does not check for lazy allocation
// ====================================================================

char *test_name = "TEST_30";

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
    for (int i = 0; i < info->n_upages; i++) {
        if (info->va[i] < 0 || info->va[i] >= KERNBASE) {
            printf(1, "Cause: `info.va[%d]` is 0x%x. It should be within [0, 0x%x)\n", i, info->va[i], KERNBASE);
            failed();
        }
        // printf(1, "va[%d] points to pa[%x]\n", info.va[i], info.pa[i]);
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
    get_n_validate_wmap_info(&winfo1, 0);

    // test begins
    int N_PAGES = 10;
    int fd = -1;
    int fixed = MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE;
    int allow_move = MREMAP_MAYMOVE;
    int dont_move = 0;

    //
    // 1. Create an anonymous Map 1
    //
    uint addr1 = 0x60021000;
    int length1 = PGSIZE * N_PAGES;
    uint mapv1 = wmap(addr1, length1, fixed, fd);
    if (mapv1 != addr1) {
        printf(1, "Cause: `wmap()` returned 0x%x\n", mapv1);
        failed();
    }

    //
    // 2. cause a page fault in the third page
    //
    char *arr1 = (char *)mapv1;
    char value1 = 'x';
    int offset1 = PGSIZE * 2;
    for (int i = 0; i < PGSIZE; i++)
        arr1[offset1 + i] = value1;

    // validate mid state 1
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 1);      // one map exists
    map_exists(&winfo2, mapv1, length1, TRUE); // mapv1 exists
    printf(1, "Map 1 placed at 0x%x. \tOkay\n", mapv1);

    //
    // 3. shrink Map 1 by 2 pages
    //
    int length2 = length1 - 2 * PGSIZE;
    int mapv2 = wremap(mapv1, length1, length2, dont_move);
    if (mapv2 != mapv1) {
        printf(1, "Cause: `wremap()` returned 0x%x\n", mapv2);
        failed();
    }
    // validate mid state 2
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 1);       // still one map exists
    map_exists(&winfo3, mapv2, length2, TRUE);  // length of Map 1 updated
    map_exists(&winfo3, mapv1, length1, FALSE); // old mapv1 does not exist
    printf(1, "Map 1 shrunk to 0x%x (length 0x%x). \tOkay\n", mapv2, length2);

    //
    // 4. shrink Map 1 by 2 pages again
    //
    int length3 = length2 - 2 * PGSIZE;
    int mapv3 = wremap(mapv2, length2, length3, allow_move);
    if (mapv3 != mapv2) {
        printf(1, "Cause: `wremap()` returned 0x%x\n", mapv3);
        failed();
    }
    // validate final state 3
    struct wmapinfo winfo4;
    get_n_validate_wmap_info(&winfo4, 1);       // still one map exists
    map_exists(&winfo4, mapv3, length3, TRUE);  // length of Map 1 updated
    map_exists(&winfo4, mapv2, length2, FALSE); // old mapv2 does not exist
    printf(1, "Map 1 shrunk to 0x%x (length 0x%x). \tOkay\n", mapv3, length3);

    // test ends
    success();
}
