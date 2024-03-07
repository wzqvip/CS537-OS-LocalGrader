#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 7
// Summary: Places multiple maps at different addresses.
//
// Does not access its memory
// Does not unmap any mapping
// Does not check for memory allocation
// Does not explicitly check for lazy allocation
// ====================================================================

char *test_name = "TEST_7";

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
        printf(1, "Cause: expected %d maps, but found %d\n", expected_total_mmaps,
               info->total_mmaps);
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
        printf(1, "Cause: expected 0x%x with length %d to %s in the list of maps\n",
               addr, length, expected ? "exist" : "not exist");
        failed();
    }
}

int create_small_file(char *filename) {
    // create a file
    int bufflen = 512 + 1;
    char buff[bufflen];
    int fd = open(filename, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf(1, "\tCause:\tFailed to create file %s\n", filename);
        failed();
    }
    // prepare the content to write
    for (int j = 0; j < bufflen; j++) {
        buff[j] = 'a' + (j % 4);
    }
    buff[bufflen - 1] = '\n';
    // write to file
    if (write(fd, buff, bufflen) != bufflen) {
        printf(1, "Error: Write to file FAILED\n");
        failed();
    }
    close(fd);
    return bufflen;
}

int main() {
    printf(1, "\n\n%s\n", test_name);

    // validate initial state
    struct wmapinfo winfo1;
    get_n_validate_wmap_info(&winfo1, 0); // no maps exist
    printf(1, "Initially 0 maps. \tOkay.\n");

    // create and open a small file
    char *filename = "small.txt";
    int filelength = create_small_file(filename);
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
    printf(1, "Map 1 at 0x%x with length %d. \tOkay.\n", map, length);

    // place map 2 (fixed and anonymous)
    int fixed_anon = MAP_FIXED | MAP_ANONYMOUS | MAP_SHARED;
    uint addr2 = addr + PGSIZE;
    uint length2 = PGSIZE * 2 + 8;
    uint map2 = wmap(addr2, length2, fixed_anon, fd);
    if (map2 != addr2) {
        printf(1, "Cause: `wmap()` returned %d\n", map2);
        failed();
    }
    // validate mid state
    struct wmapinfo winfo3;
    get_n_validate_wmap_info(&winfo3, 2);     // 2 maps exist
    map_exists(&winfo3, map, length, TRUE);   // the map exists
    map_exists(&winfo3, map2, length2, TRUE); // map 2 exists
    printf(1, "Map 2 at 0x%x with length %d. \tOkay.\n", map2, length2);

    // place map 3 (fixed and anonymous) - should fail
    uint addr3 = addr2 + PGSIZE;
    uint length3 = PGSIZE * 3 + 8;
    uint map3 = wmap(addr3, length3, fixed_anon, fd);
    if (map3 != FAILED) {
        printf(1, "Cause: `wmap()` did not fail\n", map3);
        failed();
    }
    // validate mid state
    struct wmapinfo winfo4;
    get_n_validate_wmap_info(&winfo4, 2);      // 2 maps exist
    map_exists(&winfo4, map, length, TRUE);    // map 1 exists
    map_exists(&winfo4, map2, length2, TRUE);  // map 2 exists
    map_exists(&winfo4, map3, length3, FALSE); // map 3 does not exist
    printf(1, "Map 3 did not exist. \tOkay.\n");

    // place map 4 (not-fixed and anonymous)
    int anon = MAP_ANONYMOUS | MAP_PRIVATE;
    uint addr4 = addr + length + length2;
    uint length4 = PGSIZE * 4 + 8;
    uint map4 = wmap(addr4, length4, anon, fd);
    if (map4 == FAILED) {
        printf(1, "Cause: `wmap()` returned %d\n", map4);
        failed();
    }
    // validate final state
    struct wmapinfo winfo5;
    get_n_validate_wmap_info(&winfo5, 3);      // 3 maps exist
    map_exists(&winfo5, map, length, TRUE);    // map 1 exists
    map_exists(&winfo5, map2, length2, TRUE);  // map 2 exists
    map_exists(&winfo5, map3, length3, FALSE); // map 3 does not exist
    map_exists(&winfo5, map4, length4, TRUE);  // map 4 exists
    printf(1, "Map 4 at 0x%x with length %d. \tOkay.\n", map4, length4);

    // check for overlap among maps
    uint maps[3] = {map, map2, map4};
    uint lengths[3] = {length, length2, length4};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == j)
                continue;
            if (maps[i] >= maps[j] && maps[i] < maps[j] + lengths[j]) {
                printf(1, "Cause: Map 0x%x overlaps with Map %d\n", maps[i], maps[j]);
                failed();
            }
        }
    }
    printf(1, "Map 1, Map 2, Map 4 do not overlap with each other. \tOkay\n"); // NOTE

    // test ends
    success();
}
