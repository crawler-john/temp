#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "debug.h"

int getdbsize(void){			//获取数据库的文件大小，如果数据库超过30M后，每插入一条新的数据，就删除数据库中最早的数据
	struct stat fileinfo;
	int filesize=0;
	
	stat("/home/record.db", &fileinfo);
	filesize=fileinfo.st_size;//filesize.st_size/1024;
    
	printdecmsg("Size of database", filesize);
    
	if(filesize >= 300000000)
		return 1;
	else
		return 0;
}
