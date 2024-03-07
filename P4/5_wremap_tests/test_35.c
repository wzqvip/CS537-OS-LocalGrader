#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 35
// Summary: Remaps an anonymous mapping that requires moving and unmaps it
//
// Validates that memory is properly allocated and deallocated
// Does not check for lazy allocation
// ====================================================================

char *test_name = "TEST_35";

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

void va_exists(struct pgdirinfo *info, uint va, int expected) {
    int found = 0;
    for (int i = 0; i < info->n_upages; i++) {
        if (info->va[i] == va) {
            found = 1;
            break;
        }
    }
    if (found != expected) {
        printf(1, "Cause: expected Virt.Addr. 0x%x to %s in the list of user pages\n", va, expected ? "exist" : "not exist");
        failed();
    }
}

int main() {
    printf(1, "\n\n%s\n", test_name);

    // validate initial state
    struct wmapinfo winfo1;
    get_n_validate_wmap_info(&winfo1, 0);

    // test begins
    int N_PAGES = 3;
    int fd = -1;
    int fixed_anon = MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE;
    int allow_move = MREMAP_MAYMOVE;

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
    // 3. write to map 2
    //
    char *arr = (char *)map2v1;
    int value = 'y';
    for (int i = 0; i < map2len1; i++)
        arr[i] = value;
    // validate page in the user pages
    struct pgdirinfo pinfo;
    get_n_validate_pgdirinfo(&pinfo);
    for (int i = 0; i < map2len1; i += PGSIZE)
        va_exists(&pinfo, map2v1 + i, TRUE);

    //
    // 4. expand map2 that requires moving, with moving flag (allow_move)
    //
    int map2len2 = map2len1 + PGSIZE;
    uint map2v2 = wremap(map2v1, map2len1, map2len2, allow_move);
    if (map2v2 == FAILED) {
        printf(1, "Cause: `wremap` returned 0x%x\n", map2v2);
        failed();
    }
    if (map2v2 == map2v1) {
        printf(1, "Cause: Map 2 did not move\n");
        failed();
    }
    // validate mid state
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 2);        // still 2 maps
    map_exists(&winfo3, map1v1, map1len1, TRUE); // Map 1 exists
    map_exists(&winfo3, map2v2, map2len2, TRUE); // Map 2 exists but at a different address
    map_exists(&winfo3, map2v1, map2len1, FALSE);
    printf(1, "Map 2 moved to 0x%x (length 0x%x). \tOkay\n", map2v2, map2len2);
    // validate the old VA does not exist in the user pages
    struct pgdirinfo pinfo2;
    get_n_validate_pgdirinfo(&pinfo2);
    for (int i = 0; i < map2len1; i += PGSIZE)
        va_exists(&pinfo2, map2v1 + i, FALSE);
    printf(1, "Validated the old VA does not exist in the user pages. \tOkay\n");
    // validate the new VA exists in the user pages
    for (int i = 0; i < map2len1; i += PGSIZE)
        va_exists(&pinfo2, map2v2 + i, TRUE);
    printf(1, "Validated the new VA exists in the user pages. \tOkay\n");

    //
    // 5. Unmap the Map 2
    //
    int ret = wunmap(map2v2);
    if (ret < 0) {
        printf(1, "Cause: `wunmap()` returned %d\n", ret);
        failed();
    }
    // validate final state
    struct wmapinfo winfo4;
    get_n_validate_wmap_info(&winfo4, 1);         // 1 map exists
    map_exists(&winfo4, map2v2, map2len2, FALSE); // Map 2 does not exist
    // validate the VA does not exist in the user pages
    struct pgdirinfo pinfo3;
    get_n_validate_pgdirinfo(&pinfo3);
    for (int i = 0; i < map2len2; i += PGSIZE)
        va_exists(&pinfo3, map2v2 + i, FALSE);
    printf(1, "Validated the VA does not exist in the user pages. \tOkay\n");

    // test ends
    success();
}
