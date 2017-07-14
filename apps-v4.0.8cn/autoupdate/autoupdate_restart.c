#include <stdio.h>
#include <stdlib.h>

int main()
{
	while(1){
		sleep(86400);
		system("killall autoupdate");
	}
	
	return 0;
}
