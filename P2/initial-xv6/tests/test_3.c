#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{    
    char *message = 0;  
    int fd1 = open("README",O_RDONLY);
    int retVal = getfilename(fd1, message, 256); 
    close(fd1);
    printf(1,"XV6_TEST_OUTPUT Open filename: %d\n", retVal);
    exit();
}
