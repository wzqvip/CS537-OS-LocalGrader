#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 28
// Summary: Validates child unmapping a shared anon map does not affect the parent
//
// Checks whether the child process sees old contents of the mapping
// Checks whether the parent process sees the old contents of the mapping after child unmaps
// Does not check for deallocation of pages
// Does not check for lazy allocation of pages
// ====================================================================

char *test_name = "TEST_28";

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
    int fixed_anon_shared = MAP_FIXED | MAP_ANONYMOUS | MAP_SHARED;

    //
    // 1. create an anonymous Map 1
    //
    int addr = MMAPBASE;
    int len = PGSIZE * N_PAGES;
    uint map = wmap(addr, len, fixed_anon_shared, fd);
    if (map != addr) {
        printf(1, "Cause: expected 0x%x, but got 0x%x\n", addr, map);
        failed();
    }
    printf(1, "Map 1 created at 0x%x with length %d. \tOkay\n", map, len);

    //
    // 2. write some data to all pages of the mapping
    //
    char val = 'a';
    char *arr = (char *)map;
    for (int i = 0; i < len; i++) {
        arr[i] = val;
    }
    printf(1, "Wrote '%c' all pages of Map 1. \tOkay\n", val);

    int pid = fork();
    if (pid == 0) {
        // validate initial state
        struct wmapinfo winfo2;
        get_n_validate_wmap_info(&winfo2, 1);
        map_exists(&winfo2, map, len, TRUE);
        printf(1, "Child sees Map 1. \tOkay\n");

        //
        // 3. the child process validates the data in Map 1
        //
        char *arr3 = (char *)map;
        for (int i = 0; i < len; i++) {
            if (arr3[i] != val) {
                printf(1, "Cause: Child sees '%c' at Map 1, but expected '%c'\n", arr3[i], val);
                failed();
            }
        }
        printf(1, "Child sees '%c' in Map 1. \tOkay\n", val);

        // unmap the mapping
        int ret = wunmap(map);
        if (ret < 0) {
            printf(1, "Cause: `wunmap()` returned %d\n", ret);
            failed();
        }
        // validate the mapping is removed
        struct wmapinfo winfo3;
        get_n_validate_wmap_info(&winfo3, 0);
        map_exists(&winfo3, map, len, FALSE);
        printf(1, "Child unmapped Map 1. \tOkay\n");

        // child process exits
        exit();
    } else {

        // parent process waits for the child to exit
        wait();

        // validate initial state
        struct wmapinfo winfo2;
        get_n_validate_wmap_info(&winfo2, 1);
        map_exists(&winfo2, map, len, TRUE);
        printf(1, "Parent sees Map 1. \tOkay\n");

        //
        // 5. the parent process should see the old data in Map 1
        //
        char *arr = (char *)map;
        for (int i = 0; i < len; i++) {
            if (arr[i] != val) {
                printf(1, "Cause: Parent sees %d at Map 1, but expected %d\n", arr[i], val);
                failed();
            }
        }
        printf(1, "Parent sees '%c' in Map 1. \tOkay\n", val);

        // parent process exits
        success();
    }
}
