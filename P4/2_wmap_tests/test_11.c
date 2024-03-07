#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 11
// Summary: Places a large number of anonymous maps at fixed and non-fixed addresses and then accesses all pages of each map.
//
// Does not unmap any mapping
// Does not check for memory allocation
// Does not check for lazy allocation
// ====================================================================

char *test_name = "TEST_11";

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

void map_allocated(struct wmapinfo *info, uint addr, int length, int n_loaded_pages) {
    int found = 0;
    for (int i = 0; i < info->total_mmaps; i++) {
        if (info->addr[i] == addr && info->length[i] == length) {
            found = 1;
            if (info->n_loaded_pages[i] != n_loaded_pages) { // assumes page aligned mmaps
                printf(1, "Cause: expected %d pages to be loaded, but found %d for map 0x%x with length %d\n",
                       n_loaded_pages, info->n_loaded_pages[i], addr, length);
                failed();
            }
            break;
        }
    }
    if (!found) {
        printf(1, "Cause: expected 0x%x with length %d to exist in the list of maps\n", addr, length);
        failed();
    }
}

int main() {
    printf(1, "\n\n%s\n", test_name);
    printf(1, "Maximum #allocatable pages: %d\n", (KERNBASE - MMAPBASE) / PGSIZE); // 131072

    // validate initial state
    struct wmapinfo winfo1;
    get_n_validate_wmap_info(&winfo1, 0); // no maps exist
    printf(1, "Initially 0 maps. \tOkay.\n");

    int N_MAPS = 10;
    int N_SMALL_MAPS = 3;
    int fd = -1;
    int fixed = MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE;
    int not_fixed = MAP_ANONYMOUS | MAP_PRIVATE;
    uint *maps = malloc(N_MAPS * sizeof(uint));
    uint *lengths = malloc(N_MAPS * sizeof(uint));

    // test begins

    //
    // 1. Places Map 1 at fixed address with length "401 pages"
    //
    uint addr = 0x60021000;
    uint length = PGSIZE * 400 + 8;
    uint map = wmap(addr, length, fixed, fd);
    if (map != addr) {
        printf(1, "Cause: `wmap()` returned %d\n", map);
        failed();
    }
    maps[0] = map;
    lengths[0] = length;
    printf(1, "Map 1 is placed at 0x%x. \tOkay.\n", addr);

    //
    // 2. Place 6 maps at non-fixed addresses with lengths "1 page", "2001 page", "4001 pages", ...
    //    And then, place 3 maps at fixed addresses with lengths "2 page" each
    //
    for (int i = 1; i < N_MAPS; i++) {
        addr = -1;
        length = PGSIZE * (i * 2000 + 1) + i * 8;
        if (i >= N_MAPS - N_SMALL_MAPS) // the last mmaps are small
            length = PGSIZE + i * 16;
        map = wmap(addr, length, not_fixed, fd);
        if (map < 0) {
            printf(1, "Cause: `wmap()` returned %d\n", map);
            failed();
        }
        if (map < MMAPBASE || map + length > KERNBASE) {
            printf(1, "Cause: expected 0x%x <= 0x%x < 0x%x <= 0x%x, but found 0x%x\n", MMAPBASE, map, map + length, KERNBASE);
            failed();
        }
        maps[i] = map;
        lengths[i] = length;
        printf(1, "Map %d is placed at 0x%x. \tOkay.\n", i + 1, map);
    }

    // check for overlap among maps
    for (int i = 0; i < N_MAPS; i++) {
        for (int j = 0; j < N_MAPS; j++) {
            if (i == j)
                continue;
            if (maps[i] >= maps[j] && maps[i] < maps[j] + lengths[j]) {
                printf(1, "Cause: Map %d overlaps with Map %d\n", i + 1, j + 1);
                failed();
            }
        }
    }
    printf(1, "Map 1 ~ %d do not overlap with each other. \tOkay\n", N_MAPS); // NOTE

    // access all pages of each map
    for (int i = 0; i < N_MAPS; i++) {
        char *arr = (char *)maps[i];
        char val = 'p';
        for (int j = 0; j < lengths[i]; j++) {
            arr[j] = val;
        }
        printf(1, "Accessed all pages of Map %d with length %d. \tOkay.\n", i + 1, lengths[i]);
    }
    printf(1, "Accessed all pages of Map 1 ~ %d. \tOkay.\n", N_MAPS);
    // validate final state
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, N_MAPS); // N_MAPS maps exist
    for (int i = 0; i < N_MAPS; i++) {
        int n_pages = (lengths[i] + PGSIZE - 1) / PGSIZE;
        map_allocated(&winfo2, maps[i], lengths[i], n_pages);
        printf(1, "Map %d at 0x%x with length %d. \tOkay.\n", i + 1, maps[i], lengths[i]);
    }

    // test ends
    success();
}
