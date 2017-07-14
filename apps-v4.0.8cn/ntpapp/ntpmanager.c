#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    while(1)
    {
      system("/home/applications/ntpapp.exe &");
      sleep(259200);
    }
    return 0;
}
