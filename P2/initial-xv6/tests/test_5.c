#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{
    char message[256] = {"\0"};
    int fd1 = 18;
    int retVal = getfilename(fd1, message, 256); 
    printf(1,"XV6_TEST_OUTPUT Open filename: %d\n", retVal);
    exit();
}
