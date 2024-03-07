// author: fariha.t13

#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 36
// Summary: Remap an file-backed mapping
//
// Shrinks a mapping so that allocted pages are removed
// Grows a map so it needs to be moved
// Unmaps the mapping but does not check its effect rigorously
// Checks for deallocation of pages
// Does not check for lazy allocation of pages
//
// This test checks whether `wremap()` can move a file-backed mapping
// correctly. It also checks for the proper allocation and
// deallocation of pages.
// ====================================================================

char *test_name = "TEST_36";

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
    char c = 'g';
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
    char *filename1 = "big1.txt";
    int N_PAGES = 3;
    int filelength1 = create_big_file(filename1, N_PAGES);
    int fd1 = open(filename1, O_RDWR);
    if (fd1 < 0) {
        printf(1, "Cause: Failed to open file %s\n", filename1);
        failed();
    }
    struct stat st;
    if (fstat(fd1, &st) < 0) {
        printf(1, "Cause: Failed to get file stat\n");
        failed();
    }
    if (st.size != filelength1) {
        printf(1, "Cause: File size %d != %d\n", st.size, filelength1);
        failed();
    }
    printf(1, "Created file %s with length %d. \tOkay.\n", filename1, filelength1);

    int fixed_anon = MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE;
    int fixed_file = MAP_FIXED | MAP_PRIVATE;
    int allow_move = MREMAP_MAYMOVE;
    int dont_move = 0;

    //
    // 1. Create an anonymous Map 1
    //
    uint map1addr1 = MMAPBASE + PGSIZE * 3;
    int map1len1 = st.size;
    uint map1v1 = wmap(map1addr1, map1len1, fixed_file, fd1);
    if (map1v1 != map1addr1) {
        printf(1, "Cause: `wmap()` returned 0x%x\n", map1v1);
        failed();
    }
    printf(1, "Map 1 placed at 0x%x (length 0x%x). \tOkay\n", map1v1, map1len1);

    //
    // 2. cause a page fault in the third page, also validate the content
    //    of that page
    //
    int n_pg = 2;
    char *arr1 = (char *)map1v1;
    char value1 = 'g';
    int offset1 = PGSIZE * n_pg;
    for (int i = 0; i < PGSIZE; i++) {
        if (arr1[offset1 + i] != value1) {
            printf(1, "Cause: expected %c, but found %c\n", value1, arr1[offset1 + i]);
            failed();
        }
    }
    printf(1, "File Contents are intact in Map 1. \tOkay\n", map1v1);

    // validate mid state 1
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 1);        // one map exists
    map_exists(&winfo2, map1v1, map1len1, TRUE); // Map 1 exists
    struct pgdirinfo pinfo2;
    get_n_validate_pgdirinfo(&pinfo2);
    va_exists(&pinfo2, map1addr1 + offset1, TRUE); // allocated page exists in pgdir
    printf(1, "Map 1 exists. \tOkay\n");

    //
    // 3. shrink Map 1 so that the third page needs to be deallocated
    //
    uint map1len2 = PGSIZE;
    uint map1v2 = wremap(map1v1, map1len1, map1len2, dont_move);
    if (map1v2 != map1v1) {
        printf(1, "Cause: `wremap()` returned 0x%x\n", map1v2);
        failed();
    }
    // validate mid state 2
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 1);        // one map exists
    map_exists(&winfo3, map1v2, map1len2, TRUE); // Map 1 exists, with new length
    map_exists(&winfo3, map1v1, map1len1, FALSE);
    struct pgdirinfo pinfo3;
    get_n_validate_pgdirinfo(&pinfo3);
    va_exists(&pinfo3, map1addr1 + offset1, FALSE); // de-allocated page does not exist in pgdir
    printf(1, "Map 1 shrunk to 0x%x. \tOkay\n", map1v2);

    // create and open another big file
    char *filename2 = "big2.txt";
    int filelength2 = create_big_file(filename2, N_PAGES);
    int fd2 = open(filename2, O_RDWR);
    if (fd2 < 0) {
        printf(1, "Cause: Failed to open file %s\n", filename2);
        failed();
    }
    if (fstat(fd2, &st) < 0) {
        printf(1, "Cause: Failed to get file stat\n");
        failed();
    }
    if (st.size != filelength2) {
        printf(1, "Cause: File size %d != %d\n", st.size, filelength2);
        failed();
    }
    printf(1, "Created file %s with length %d. \tOkay.\n", filename2, filelength2);

    //
    // 4. create anonymous MAP 2
    //
    uint map2addr1 = map1addr1 - PGSIZE * N_PAGES;
    int map2len1 = PGSIZE * (N_PAGES - 1);
    uint map2v1 = wmap(map2addr1, map2len1, fixed_anon, fd2);
    if (map2v1 == FAILED) {
        printf(1, "Cause: `wmap()` returned 0x%x\n", map2v1);
        failed();
    }
    printf(1, "Map 2 placed at 0x%x (length 0x%x). \tOkay\n", map2v1, map2len1);

    //
    // 5. create page fault at the second page of MAP 2
    //
    char *arr2 = (char *)map2v1;
    int value2 = 'y';
    int offset2 = PGSIZE;
    for (int i = 0; i < PGSIZE; i++)
        arr2[offset2 + i] = value2;
    printf(1, "Wrote to Map 2. \tOkay\n");

    // validate mid state 3
    struct wmapinfo winfo4;
    get_n_validate_wmap_info(&winfo4, 2);        // 2 maps exist
    map_exists(&winfo4, map2v1, map2len1, TRUE); // Map 2 exists
    struct pgdirinfo pinfo4;
    get_n_validate_pgdirinfo(&pinfo4);
    va_exists(&pinfo4, map2addr1 + offset2, TRUE); // allocated page exists in pgdir
    printf(1, "Map 2 exists. \tOkay\n");

    //
    // 6. grow Map 2 so that it needs to be moved
    //
    int map2len2 = PGSIZE * (N_PAGES + 2);
    uint map2v2 = wremap(map2v1, map2len1, map2len2, allow_move);
    if (map2v2 == FAILED || map2v2 == map2v1) {
        printf(1, "Cause: `wremap()` returned 0x%x\n", map2v2);
        failed();
    }
    printf(1, "Map 2 moved to 0x%x. \tOkay\n", map2v2);

    // validate final state
    struct wmapinfo winfo5;
    get_n_validate_wmap_info(&winfo5, 2);        // 2 maps exist
    map_exists(&winfo5, map2v2, map2len2, TRUE); // Map 2 exists, with new length
    struct pgdirinfo pinfo5;
    get_n_validate_pgdirinfo(&pinfo5);
    va_exists(&pinfo5, map2v2 + offset2, TRUE); // allocated page exists in pgdir
    // check page contents are correct in MAP2
    arr2 = (char *)map2v2;
    for (int i = 0; i < PGSIZE; i++) {
        if (arr2[offset2 + i] != value2) {
            printf(1, "Cause: expected '%c', but found '%c'\n", value2, arr2[offset2 + i]);
            failed();
        }
    }
    printf(1, "Map 2 Contents are intact. \tOkay\n", map2v2);

    // unmap Map 1
    int ret = wunmap(map1v2);
    if (ret < 0) {
        printf(1, "Cause: `wunmap()` returned %d\n", ret);
        failed();
    }
    printf(1, "Map 1 unmapped. \tOkay\n");
    // unmap Map 2
    ret = wunmap(map2v2);
    if (ret < 0) {
        printf(1, "Cause: `wunmap()` returned %d\n", ret);
        failed();
    }
    printf(1, "Map 2 unmapped. \tOkay\n");

    // cleanup
    close(fd1);
    close(fd2);

    // test ends
    success();
}
