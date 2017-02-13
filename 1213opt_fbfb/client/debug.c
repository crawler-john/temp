#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define DEBUG
#define DEBUGLOG

#ifdef DEBUG
#define DEBUG_NAME	"client"
#endif

#ifdef DEBUGLOG
#define	FILE_NAME	"/home/client.log"
#define NEW_FILE_NAME	"/home/client2.log"

char date_time[20];

char *get_current_time()		//发给EMA记录时获取的时间，格式：年月日时分秒，如20120902142835
{
	time_t tm;
	struct tm record_time;    //记录时间

	time(&tm);
	memcpy(&record_time, localtime(&tm), sizeof(record_time));

	sprintf(date_time, "%04d-%02d-%02d %02d:%02d:%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
			record_time.tm_hour, record_time.tm_min, record_time.tm_sec);

	return date_time;
}

int get_file_size(char *file_name)			//获取数据库的文件大小，如果数据库超过30M后，每插入一条新的数据，就删除数据库中最早的数据
{
	struct stat file_info;
	int file_size=0;

	stat(file_name, &file_info);
	file_size = file_info.st_size;

	return file_size;
}

int file_rename()
{
	if(get_file_size(FILE_NAME)>=30000000) {
		rename(FILE_NAME, NEW_FILE_NAME);
	}

	return 0;
}
#endif

void printmsg(char *msg)		//打印字符串
{
#ifdef DEBUG
	printf("%s==>%s!\n", DEBUG_NAME, msg);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s!\n", get_current_time(), msg);
		fclose(fp);
	}
	file_rename();
#endif
}

void print2msg(char *msg1, char *msg2)		//打印字符串
{
#ifdef DEBUG
	printf("%s==>%s: %s!\n", DEBUG_NAME, msg1, msg2);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: %s!\n", get_current_time(), msg1, msg2);
		fclose(fp);
	}
	file_rename();
#endif
}

void printdecmsg(char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	printf("%s==>%s: %d!\n", DEBUG_NAME, msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: %d!\n", get_current_time(), msg, data);
		fclose(fp);
	}
	file_rename();
#endif
}

void printfloatmsg(char *msg, float data)		//打印实数
{
#ifdef DEBUG
	printf("%s==>%s: %f!\n", DEBUG_NAME, msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: %f!\n", get_current_time(), msg, data);
		fclose(fp);
	}
	file_rename();
#endif
}

void printhexmsg(char *msg, char *data, int size)		//打印十六进制数据
{
#ifdef DEBUG
	int i;

	printf("%s==>%s: ", DEBUG_NAME, msg);
	for(i=0; i<size; i++)
		printf("%X, ", data[i]);
	printf("\n");
#endif
#ifdef DEBUGLOG
	FILE *fp;
	int j;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: ", get_current_time(), msg);
		for(j=0; j<size; j++)
			fprintf(fp, "%X, ", data[j]);
		fprintf(fp, "\n");
		fclose(fp);
	}
	file_rename();
#endif
}
