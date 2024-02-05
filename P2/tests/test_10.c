#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{    
    char message[256] = {"\0"};
    int fd1 = 1; //stdout
    if(getfilename(fd1, message, 256) != 0)
    {    
        exit();//&message[0]
    }
    printf(1,"XV6_TEST_OUTPUT Open filename: %s\n", message);
    exit();
}
