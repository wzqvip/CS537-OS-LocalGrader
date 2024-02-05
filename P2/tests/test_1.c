#include "types.h"
#include "stat.h"
#include "user.h"

int main(void)
{    
    char message[256];  
    int retVal = getfilename(3, message, 256); 
    printf(1,"XV6_TEST_OUTPUT Open filename: %d\n", retVal);
    exit();
}
