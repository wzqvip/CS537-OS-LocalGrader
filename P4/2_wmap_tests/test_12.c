#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 12  
// Summary: Checks for lazy allocation in fixed anon mapping
//
// Checks for memory allocation
// Checks for lazy allocation
// Does not unmap any mapping
// ====================================================================

char *test_name = "TEST_12";

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

void no_mmaps_in_pgdir(struct pgdirinfo *info) {
    for (int i = 0; i < info->n_upages; i++) {
        if (info->va[i] >= MMAPBASE && info->va[i] < KERNBASE) {
            printf(1, "Cause: Virt.Addr. 0x%x should not exist in the list of user pages\n", info->va[i]);
            failed();
        }
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

void map_allocated(struct wmapinfo *info, uint addr, int length, int n_loaded_pages) {
    int found = 0;
    for (int i = 0; i < info->total_mmaps; i++) {
        if (info->addr[i] == addr && info->length[i] == length) {
            found = 1;
            if (info->n_loaded_pages[i] != n_loaded_pages) {
                printf(1, "Cause: expected %d pages to be loaded, but found %d\n", n_loaded_pages, info->n_loaded_pages[i]);
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

int create_big_file(char *filename, int N_PAGES) {
    printf(1, "\n\n%s\n", test_name);

    // create a file
    int bufflen = 512;
    char buff[bufflen + 1];
    int fd = open(filename, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf(1, "\tCause:\tFailed to create file %s\n", filename);
        failed();
    }
    // write in steps as we cannot have a buffer larger than PGSIZE
    char c = 'a';
    for (int i = 0; i < N_PAGES; i++) {
        int m = PGSIZE / bufflen;
        for (int k = 0; k < m; k++) {
            // prepare the content to write
            for (int j = 0; j < bufflen; j++) {
                buff[j] = c;
            }
            buff[bufflen] = '\0';
            // write to file
            if (write(fd, buff, bufflen) != bufflen) {
                printf(1, "Error: Write to file FAILED (%d, %d)\n", i, k);
                failed();
            }
        }
        c++; // first page is filled with 'a', second with 'b', and so on
    }
    close(fd);
    return N_PAGES * PGSIZE;
}

int main() {
    printf(1, "\n\n%s\n", test_name);

    // validate initial state
    struct wmapinfo winfo1;
    get_n_validate_wmap_info(&winfo1, 0); // no maps exist
    struct pgdirinfo pinfo1;
    get_n_validate_pgdirinfo(&pinfo1);
    no_mmaps_in_pgdir(&pinfo1); // no maps in the mmap range in pgdir
    printf(1, "Initially 0 maps. \tOkay.\n");

    //
    // 1. place a file-backed mmap at fixed address "0x60000000"
    //
    int N_PAGES = 3;
    int fixed_anon = MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE;
    uint addr = MMAPBASE;
    int length = PGSIZE * N_PAGES;
    uint map = wmap(addr, length, fixed_anon, 0);
    if (map != addr) {
        printf(1, "Cause: Expected 0x%x, but `wmap()` returned %d\n", addr, map);
        failed();
    }
    // validate mid state
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 1); // one map exists
    printf(1, "Map placed at 0x%x. \tOkay.\n", map);

    //
    // checks for lazy allocation
    //
    map_allocated(&winfo2, map, length, 0); // no pages loaded yet
    printf(1, "No page allocated yet. \tOkay.\n");
    struct pgdirinfo pinfo2;
    get_n_validate_pgdirinfo(&pinfo2);
    for (int i = 0; i < N_PAGES; i++) {
        va_exists(&pinfo2, map + PGSIZE * i, FALSE); // no page is mapped yet in pgdir
    }
    printf(1, "No page have entry in pgdir yet. \tOkay.\n");

    //
    // 2. read from the second page of the mapping, causing page fault
    //
    char *arr = (char *)map;
    for (int i = 0; i < PGSIZE; i++) {
        if (arr[PGSIZE + i] != 0) {
            printf(1, "Cause: Expected the page to be zero initialized\n", arr[i]);
            failed();
        }
    }
    printf(1, "Read from the second page. \tOkay.\n");

    // validate final state
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 1); // one map exists

    //
    // checks for lazy allocation
    //
    map_allocated(&winfo3, map, length, 1); // 1 page loaded
    printf(1, "Allocated 1 page. \tOkay.\n");
    struct pgdirinfo pinfo3;
    get_n_validate_pgdirinfo(&pinfo3);
    va_exists(&pinfo3, map, FALSE);         // first page is not allocated
    va_exists(&pinfo3, map + PGSIZE, TRUE); // second page is allocatedˇ
    printf(1, "Allocated page have entry in pgdir. \tOkay.\n");

    // test ends
    success();
}
