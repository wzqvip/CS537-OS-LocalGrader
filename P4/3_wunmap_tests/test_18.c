#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 18
// Summary: access one big filebacked map (checks for memory allocation)
//
// Checks for memory allocation and deallocation
// Does not unmap any mapping
// Does not check for lazy allocation
// ====================================================================

char *test_name = "TEST_18";

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

int create_big_file(char *filename, int N_PAGES) {
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
    printf(1, "Initially 0 maps. \tOkay.\n");

    // create and open a big file
    char *filename = "big.txt";
    int N_PAGES = 5;
    int filelength = create_big_file(filename, N_PAGES);
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf(1, "Cause: Failed to open file %s\n", filename);
        failed();
    }
    struct stat st;
    if (fstat(fd, &st) < 0) {
        printf(1, "Cause: Failed to get file stat\n");
        failed();
    }
    if (st.size != filelength) {
        printf(1, "Cause: File size %d != %d\n", st.size, filelength);
        failed();
    }
    printf(1, "Created file %s with length %d. \tOkay.\n", filename, filelength);

    // place map 1 (fixed and filebacked)
    int fixed_filebacked = MAP_FIXED | MAP_PRIVATE;
    uint addr = 0x60021000;
    uint length = filelength;
    uint map = wmap(addr, length, fixed_filebacked, fd);
    if (map != addr) {
        printf(1, "Cause: `wmap()` returned %d\n", map);
        failed();
    }
    // validate mid state
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 1);   // 1 map exists
    map_exists(&winfo2, map, length, TRUE); // map 1 exists
    printf(1, "Placed map 1 at 0x%x with length %d. \tOkay.\n", map, length);

    // access all pages of map 1
    char *arr = (char *)map;
    char val = 'p';
    for (int i = 0; i < length; i++) {
        arr[i] = val;
    }
    // validate all pages of map 1
    for (int i = 0; i < length; i++) {
        if (arr[i] != val) {
            printf(1, "Cause: expected %c at 0x%x, but got %c\n", val, addr + i, arr[i]);
            failed();
        }
    }
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 1);         // 1 map exists
    map_allocated(&winfo3, map, length, N_PAGES); // 1 page loaded
    struct pgdirinfo pinfo;
    getpgdirinfo(&pinfo);
    for (int i = 0; i < N_PAGES; i++) {
        va_exists(&pinfo, map + i * PGSIZE, TRUE);
    }
    printf(1, "Accessed all pages of Map 1. \tOkay.\n");

    // unmap map 1
    int ret = wunmap(map);
    if (ret < 0) {
        printf(1, "Cause: `wunmap()` returned %d\n", ret);
        failed();
    }
    // validate final state
    struct wmapinfo winfo4;
    get_n_validate_wmap_info(&winfo4, 0);     // no maps exist
    map_exists(&winfo4, addr, length, FALSE); // map 1 does not exist
    printf(1, "Map 1 unmapped. \tOkay.\n");
    // validate the pages are unallocated
    struct pgdirinfo pinfo2;
    getpgdirinfo(&pinfo2);
    for (int i = 0; i < N_PAGES; i++) {
        va_exists(&pinfo2, map + i * PGSIZE, FALSE);
    }
    printf(1, "Unmapped pages are unallocated. \tOkay.\n");

    // test ends
    success();
}
