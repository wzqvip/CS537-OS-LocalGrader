#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{    
    char message[256] = {"\0"};
    int fd1 = open("file.txt",O_CREATE);
    int fd2 = open("helloworld.txt",O_CREATE);

    if(getfilename(fd2, message, 256) != 0)
    {    
        exit();//&message[0]
    }
    
    printf(1,"XV6_TEST_OUTPUT Open filename: %s\n", message);
    close(fd1);
    close(fd2);
    exit();
}
