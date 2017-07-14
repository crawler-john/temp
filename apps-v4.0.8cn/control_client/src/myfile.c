#include <stdio.h>
#include <string.h>
#include <time.h>

#include "myfile.h"
#include "mydebug.h"

void delete_newline(char *s)
{
	if(10 == s[strlen(s)-1])
		s[strlen(s)-1] = '\0';
}

/* 获取(存储单个参数的)配置文件中的值 */
char *file_get_one(char *s, int count, const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if(fp == NULL){
		debug_err(filename);
		return NULL;
	}
	fgets(s, count, fp);
	if (10 == s[strlen(s) - 1])
		s[strlen(s) - 1] = '\0';
	fclose(fp);
	return s;
}

/* 将单个参数值写入配置文件中 */
int file_set_one(const char *s, const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "w");
	if(fp == NULL){
		debug_err(filename);
		return -1;
	}
	if(EOF == fputs(s, fp)){
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

/* 获取(存储多个参数的)配置文件中的值 */
int file_get_array(MyArray *array, int num, const char *filename)
{
	FILE *fp;
	int count = 0;
	char buffer[128] = {'\0'};

	memset(array, 0 ,sizeof(MyArray)*num);
	fp = fopen(filename, "r");
	if(fp == NULL){
		debug_err(filename);
		return -1;
	}
	while(!feof(fp))
	{
		if(count >= num)return 0;
		memset(buffer, 0 ,sizeof(buffer));
		fgets(buffer, 128, fp);
		if(!strlen(buffer))continue;

		strncpy(array[count].name, buffer, strcspn(buffer, "="));
		strncpy(array[count].value, &buffer[strlen(array[count].name)+1], 64);
		delete_newline(array[count].value);
		count++;
	}
	fclose(fp);
	return 0;
}

int file_set_array(const MyArray *array, int num, const char *filename)
{
	FILE *fp;
	int i;

	fp = fopen(filename, "w");
	if(fp == NULL){
		debug_err(filename);
		return -1;
	}
	for(i=0; i<num; i++){
		fprintf(fp, "%s=%s\n", array[i].name, array[i].value);
	}
	fclose(fp);
	return 0;
}

int get_time(char *sendcommanddatetime, char *sendcommandtime)		//发给EMA记录时获取的时间，格式：年月日时分秒，如20120902142835
{
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};

	time(&tm);
	memcpy(&record_time,localtime(&tm), sizeof(record_time));

	sprintf(temp, "%d", record_time.tm_year+1900);
	strcat(sendcommanddatetime,temp);
	//strcat(db_time,"-");
	if(record_time.tm_mon+1<10)
		strcat(sendcommanddatetime,"0");
	sprintf(temp, "%d", record_time.tm_mon+1);
	strcat(sendcommanddatetime,temp);
	//strcat(db_time,"-");
	if(record_time.tm_mday<10)
		strcat(sendcommanddatetime,"0");
	sprintf(temp, "%d", record_time.tm_mday);
	strcat(sendcommanddatetime,temp);
	//strcat(db_time," ");
	if(record_time.tm_hour<10)
		strcat(sendcommanddatetime,"0");
	sprintf(temp, "%d", record_time.tm_hour);
	strcat(sendcommanddatetime,temp);
	//strcat(db_time,":");
	if(record_time.tm_min<10)
		strcat(sendcommanddatetime,"0");
	sprintf(temp, "%d", record_time.tm_min);
	strcat(sendcommanddatetime,temp);
	//strcat(db_time,":");
	if(record_time.tm_sec<10)
		strcat(sendcommanddatetime,"0");
	sprintf(temp, "%d", record_time.tm_sec);
	strcat(sendcommanddatetime,temp);

	sendcommandtime[0] = record_time.tm_hour;
	sendcommandtime[1] = record_time.tm_min;

}

