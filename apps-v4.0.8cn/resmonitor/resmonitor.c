#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sqlite3.h"

void set_stopflag(void)
{
	FILE *fp;

	fp = fopen("/etc/yuneng/stopflag.conf", "w");
	fputs("1", fp);
	fclose(fp);
}

void clear_configure_flag(void)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db;

	sqlite3_open("/home/database.db", &db);	//create a database
	strcpy(sql, "UPDATE id SET flag=0 WHERE item>=0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
}

int main()
{
	time_t tm;         //实例化time_t结构
	struct tm timenow;         //实例化tm结构指针
	
	while(1){
		time(&tm);
		memcpy(&timenow,localtime(&tm), sizeof(timenow));
		//printf("hour:%d, minute:%d, second:%d\n", timenow.tm_hour, timenow.tm_min, timenow.tm_sec);
		if(1==timenow.tm_hour){
			set_stopflag();
			system("killall main.exe");
			system("killall control_client");
			system("/home/applications/ntpapp.exe &");
			clear_configure_flag();
  		}
		sleep(3600);
	}
	
	return 0;
}
