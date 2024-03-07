#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 31
// Summary: Remaps an anonymous mapping that needs to be moved
//
// Does not check for the page contents to be intact
// Does not check for deallocation of pages
// Does not check for lazy allocation
// ====================================================================

char *test_name = "TEST_31";

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
    get_n_validate_wmap_info(&winfo1, 0);

    // test begins
    int N_PAGES = 10;
    int fd = -1;
    int fixed_anon = MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE;
    int allow_move = MREMAP_MAYMOVE;
    int dont_move = 0;

    //
    // 1. create a anonymous MAP 1
    //
    uint map1addr1 = MMAPBASE + PGSIZE * N_PAGES;
    int map1len1 = PGSIZE * N_PAGES;
    uint map1v1 = wmap(map1addr1, map1len1, fixed_anon, fd);
    if (map1v1 == FAILED) {
        printf(1, "Cause: `wmap()` returned 0x%x\n", map1v1);
        failed();
    }
    printf(1, "Map 1 placed at 0x%x (length 0x%x)\n", map1v1, map1len1);

    //
    // 2. create another anonymous MAP 2
    //
    uint map2addr1 = map1addr1 - PGSIZE * N_PAGES;
    int map2len1 = PGSIZE * N_PAGES;
    uint map2v1 = wmap(map2addr1, map2len1, fixed_anon, fd);
    if (map2v1 == FAILED) {
        printf(1, "\tCause:\tmem2 failed\n");
        failed();
    }
    printf(1, "Map 2 placed at 0x%x (length 0x%x)\n", map2v1, map2len1);

    // validate mid state
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 2); // 2 maps exist
    map_exists(&winfo2, map1v1, map1len1, TRUE);
    map_exists(&winfo2, map2v1, map2len1, TRUE);
    printf(1, "Validated two maps. \tOkay\n");

    //
    // 3. expand Map 2 that requires moving, without moving flag (dont_move)
    //
    int map2len2 = PGSIZE * (N_PAGES + 2);
    uint map2v2 = wremap(map2v1, map2len1, map2len2, dont_move);
    if (map2v2 != FAILED) {
        printf(1, "Cause: `wremap` returned 0x%x\n", map2v2);
        failed();
    }
    printf(1, "Map 2 failed to move as the dont_move flag was set. \tOkay\n");

    //
    // 4. expand map2 that requires moving, with moving flag (allow_move)
    //
    int map2len3 = map2len2;
    uint map2v3 = wremap(map2v1, map2len1, map2len3, allow_move);
    if (map2v3 == FAILED) {
        printf(1, "Cause: `wremap` returned 0x%x\n", map2v3);
        failed();
    }
    if (map2v3 == map2v1) {
        printf(1, "Cause: Map 2 did not move\n");
        failed();
    }
    // validate final state
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 2);        // still 2 maps
    map_exists(&winfo3, map1v1, map1len1, TRUE); // Map 1 exists
    map_exists(&winfo3, map2v3, map2len3, TRUE); // Map 2 exists but at a different address
    map_exists(&winfo3, map2v1, map2len1, FALSE);
    printf(1, "Map 2 moved to 0x%x (length 0x%x). \tOkay\n", map2v3, map2len3);

    // test ends
    success();
}
