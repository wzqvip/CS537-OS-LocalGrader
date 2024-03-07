#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 32
// Summary: Validates memory access in a map that is shrinked
//
// Validates the page contents to be intact after remap
// Checks for deallocation and allocation of pages
// Does not check for lazy allocation
// ====================================================================

char *test_name = "TEST_32";

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
    int N_PAGES = 5;
    int fd = -1;
    int fixed_anon = MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE;
    int allow_move = MREMAP_MAYMOVE;

    //
    // 1. create an anonymous MAP 1
    //
    uint map1addr1 = MMAPBASE + 2 * PGSIZE;
    int map1len1 = PGSIZE * N_PAGES;
    uint map1v1 = wmap(map1addr1, map1len1, fixed_anon, fd);
    if (map1v1 == FAILED) {
        printf(1, "Cause: `wmap` returned %d\n", map1v1);
        failed();
    }
    // validate mid state 1
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 1);        // 1 maps exist
    map_exists(&winfo2, map1v1, map1len1, TRUE); // Map 1 exists
    printf(1, "Map 1 placed at 0x%x (length 0x%x)\n", map1v1, map1len1);

    //
    // 2. write to the first three pages of Map 1
    //
    char *arr = (char *)map1v1;
    int value = 'y';
    for (int i = 0; i < PGSIZE * 3; i++)
        arr[i] = value;
    // validate page in the user pages
    struct pgdirinfo pinfo;
    get_n_validate_pgdirinfo(&pinfo);
    va_exists(&pinfo, map1v1 + PGSIZE * 0, TRUE); // first page exists
    va_exists(&pinfo, map1v1 + PGSIZE * 1, TRUE); // second page exists
    va_exists(&pinfo, map1v1 + PGSIZE * 2, TRUE); // third page exists
    printf(1, "Wrote '%c' at the first three pages of Map 1. \tOkay\n", value);

    //
    // 3. shrink Map 1 such that it requires deletion of allocated pages
    //
    int map1len2 = PGSIZE;
    uint map1v2 = wremap(map1v1, map1len1, map1len2, allow_move);
    if (map1v2 != map1v1) {
        printf(1, "Cause: `wremap` returned 0x%x\n", map1v2);
        failed();
    }
    // validate final state
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 1);        // still 1 map
    map_exists(&winfo3, map1v2, map1len2, TRUE); // Map 1 exists but at a different address
    map_exists(&winfo3, map1v1, map1len1, FALSE);
    printf(1, "Map 1 shrunk to 0x%x (length 0x%x)\n", map1v2, map1len2);

    // validate the page contents
    char *arr2 = (char *)map1v2;
    for (int i = 0; i < PGSIZE; i++) {
        if (arr2[i] != value) {
            printf(1, "Cause: expected '%c' at 0x%x, but found '%c'\n", value, map1v2 + i, arr2[i]);
            failed();
        }
    }
    printf(1, "Validated the page contents to be intact after remap. \tOkay\n");

    // validate the old VA does not exist in the user pages
    struct pgdirinfo pinfo2;
    get_n_validate_pgdirinfo(&pinfo2);
    va_exists(&pinfo2, map1v2 + PGSIZE * 0, TRUE);  // first page exists
    va_exists(&pinfo2, map1v2 + PGSIZE * 1, FALSE); // second page does not exist
    va_exists(&pinfo2, map1v2 + PGSIZE * 2, FALSE); // third page does not exist
    printf(1, "Validated the old VA does not exist in the user pages. \tOkay\n");

    // test ends
    success();
}
