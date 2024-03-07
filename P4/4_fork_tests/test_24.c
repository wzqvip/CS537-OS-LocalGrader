#include "types.h"
#include "user.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"

#include "wmap.h"

// ====================================================================
// Test 24
// Summary: Validates parent can NOT see child's change for filebacked private mappings with fork
//
// Checks whether the parent process sees the child's modification
// Does not unmap the mapping
// Does not check for deallocation of pages
// Does not check for lazy allocation of pages
// ====================================================================

char *test_name = "TEST_24";

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
        buff[j] = 'a';
    }
    buff[bufflen - 1] = '\n';
    // write to file
    bufflen--;
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
    get_n_validate_wmap_info(&winfo1, 0);

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

    //
    // 1. create an filebacked private Map 1
    //
    int fixed_filebacked_private = MAP_FIXED | MAP_PRIVATE;
    int addr1 = MMAPBASE;
    int len1 = filelength;
    uint map1 = wmap(addr1, len1, fixed_filebacked_private, fd);
    if (map1 != addr1) {
        printf(1, "Cause: `wmap()` returned 0x%x\n", map1);
        failed();
    }
    // validate the mapping
    struct wmapinfo winfo2;
    get_n_validate_wmap_info(&winfo2, 1);
    printf(1, "Map 1 created at 0x%x with length %d. \tOkay\n", map1, len1);

    // validate file content
    char val1 = 'a';
    char *arr1 = (char *)map1;
    // touch all pages for map1
    for (int i = 0; i < len1; i++) {
        if (arr1[i] != val1) {
            printf(1, "Cause: Map 1 has '%c', but expected %c\n", arr1[i], val1);
            failed();
        }
    }
    printf(1, "Map 1 has the same data ('%c') as the file. \tOkay\n", val1);

    int newval = 'g';
    int pid = fork();
    if (pid == 0) {
        printf(1, "Child process starts\n");

        //
        // 3. the child process should see the same data as the parent in Map 1 and Map 2
        //
        char *arr3 = (char *)map1;
        for (int i = 0; i < len1; i++) {
            char expected = val1;
            if (arr3[i] != expected) {
                printf(1, "Cause: Child sees '%c' at Map 1, but expected '%c'\n", arr3[PGSIZE + i], expected);
                failed();
            }
        }
        printf(1, "Child sees '%c' in Map 1. \tOkay\n", val1);

        //
        // 4. modify the data in Map 1 (should not affect the parent) and Map 2
        //
        for (int i = 0; i < len1; i++) {
            arr3[i] = newval;
        }
        printf(1, "Child wrote '%c' in Map 1. \tOkay\n", newval);

        // child process exits
        exit();
    } else {
        // parent process waits for the child to exit
        wait();

        //
        // 5. the parent process should see the old data in Map 1
        //
        char *arr3 = (char *)map1;
        for (int i = 0; i < len1; i++) {
            if (arr3[i] != val1) {
                printf(1, "Cause: Parent sees %d at Map 1, but expected %d\n", arr3[i], val1);
                failed();
            }
        }
        printf(1, "Parent sees '%c' (old data) in Map 1. \tOkay\n", val1);

        // clean up
        close(fd);

        // parent process exits
        success();
    }
}
