#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"

long int get_time(char *sendcommanddatetime, char *sendcommandtime)		//发给EMA记录时获取的时间，格式：年月日时分秒，如20120902142835
{
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};
	int time_linux;

	time_linux=time(&tm);
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
    
	print2msg("Broadcast time", sendcommanddatetime);

	return time_linux;
}

void getcurrenttime(char db_time[])		//ECU上显示用的时间，与上面函数获取的时间只是格式区别，格式：年-月-日 时：分：秒，如2012-09-02 14：28：35
{
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};

	time(&tm);
	memcpy(&record_time,localtime(&tm), sizeof(record_time));
	sprintf(temp, "%d", record_time.tm_year+1900);
	strcat(db_time,temp);
	strcat(db_time,"-");
	if(record_time.tm_mon+1<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_mon+1);
	strcat(db_time,temp);
	strcat(db_time,"-");
	if(record_time.tm_mday<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_mday);
	strcat(db_time,temp);
	strcat(db_time," ");
	if(record_time.tm_hour<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_hour);
	strcat(db_time,temp);
	strcat(db_time,":");
	if(record_time.tm_min<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_min);
	strcat(db_time,temp);
	strcat(db_time,":");
	if(record_time.tm_sec<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_sec);
	strcat(db_time,temp);
}

void getdate(char date[10])		//获取日期，用于当天发电量，格式：年月日，如20120902
{
	time_t tm;
	struct tm record_time;
	char temp[5]={'\0'};

	time(&tm);
	memcpy(&record_time,localtime(&tm), sizeof(record_time));
	sprintf(temp, "%d", record_time.tm_year+1900);
	strcat(date,temp);
	if(record_time.tm_mon+1<10)
		strcat(date,"0");
	sprintf(temp, "%d", record_time.tm_mon+1);
	strcat(date,temp);
	if(record_time.tm_mday<10)
		strcat(date,"0");
	sprintf(temp, "%d", record_time.tm_mday);
	strcat(date,temp);
}

int get_date_time(char *date_time)		//发给EMA记录时获取的时间，格式：年月日时分秒，如20120902142835
{
	time_t tm;
	struct tm record_time;    //记录时间


	time(&tm);
	memcpy(&record_time, localtime(&tm), sizeof(record_time));

	sprintf(date_time, "%04d%02d%02d%02d%02d%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
			record_time.tm_hour, record_time.tm_min, record_time.tm_sec);

	return 0;
}
