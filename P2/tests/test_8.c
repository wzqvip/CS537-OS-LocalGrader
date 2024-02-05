#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{    

    int pid = fork();

    if(pid == 0){
    // child
        char *argv[2];
        argv[0] = "getfilename";
        argv[1] = "README";
        exec("/getfilename", argv);
        exit();
    }
    if(pid > 0) {
        wait();
        exit();
    }    
}