#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 21
// Summary: Validates mmaps exists in the mmap list in both parent and child for anonymous private and shared mappings with fork
//
// Checks whether the child process has the same mmaps (addr and length) as the parent
// Does not check if they share the same physical pages or not
// Does not check for deallocation of pages
// Does not check for lazy allocation of pages
// ====================================================================

char *test_name = "TEST_21";

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
    int fixed_anon_private = MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE;
    int fixed_anon_shared = MAP_FIXED | MAP_ANONYMOUS | MAP_SHARED;

    //
    // 1. create an anonymous private Map 1
    //
    int addr1 = MMAPBASE;
    int len1 = PGSIZE * N_PAGES;
    uint map1 = wmap(addr1, len1, fixed_anon_private, fd);
    if (map1 != addr1) {
        printf(1, "Cause: `wmap()` failed\n");
        failed();
    }
    printf(1, "Map 1 created at 0x%x (length 0x%x). \tOkay.\n", addr1, len1);

    //
    // 2. create an anonymous shared Map 2
    //
    int addr2 = MMAPBASE + len1 + PGSIZE;
    int len2 = PGSIZE * N_PAGES + 10;
    uint map2 = wmap(addr2, len2, fixed_anon_shared, fd);
    if (map2 != addr2) {
        printf(1, "Cause: `wmap()` failed\n");
        failed();
    }
    printf(1, "Map 2 created at 0x%x (length 0x%x). \tOkay.\n", addr2, len2);

    int pid = fork();
    if (pid == 0) {
        printf(1, "Child process starts\n");
        //
        // 3. validate state
        //
        struct wmapinfo winfo2;
        get_n_validate_wmap_info(&winfo2, 2);  // 2 maps in child process
        map_exists(&winfo2, map1, len1, TRUE); // Map 1 exists
        map_exists(&winfo2, map2, len2, TRUE); // Map 2 exists
        printf(1, "Map 1 and 2 exist in the child process. \tOkay.\n");
        // child process exits
        exit();
    } else {
        // parent process waits for the child to exit
        wait();
        //
        // 4. validate state
        //
        struct wmapinfo winfo2;
        get_n_validate_wmap_info(&winfo2, 2);  // 2 maps in parent process
        map_exists(&winfo2, map1, len1, TRUE); // Map 1 exists
        map_exists(&winfo2, map2, len2, TRUE); // Map 2 exists
        printf(1, "Map 1 and 2 exist in the parent process. \tOkay\n");
        // parent process exits
        success();
    }
}
