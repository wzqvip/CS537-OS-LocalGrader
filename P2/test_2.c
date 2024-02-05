#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{    

    char message[256] = {"\0"};
    int fd1 = open("README",O_RDONLY);

    if(getfilename(fd1, message, 256) != 0)
    {    
        exit();
    }
    
    printf(1,"XV6_TEST_OUTPUT Open filename: %s\n",message);
    close(fd1);
    exit();  
}
