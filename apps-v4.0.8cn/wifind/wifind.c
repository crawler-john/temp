#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

//#define DEBUG
//#define DEBUGLOG


#define FILE_NAME	"/home/wifind.log"

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
void printdecmsg(char *msg, int data)		//打印整形数据
{

#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: %d!\n", get_current_time(), msg, data);
		fclose(fp);
	}
	//file_rename();
#endif
}


int main()
{
	FILE *fp;
	char ss1[256]={'\0'};
	char ss2[256]={'\0'};
	char ss3[256]={'\0'};
	char ss4[256]={'\0'};
	char ss5[256]={'\0'};
	char ss6[256]={'\0'};
	char ss7[256]={'\0'};
	char ss8[256]={'\0'};
	char ss9[256]={'\0'};
	char ss10[256]={'\0'};

	int ret;
	while(1)
	{
		ret=system("iwlist wlan0 scan | grep -E 'SSID|Quality|Encryption|Group' | sed 's/^ *//' >/tmp/wifind.conf");
//		printf("scan=%d\n",ret);
		//ret=system("iwlist wlan0 rate ");
		//printf("rate=%d\n",ret);
		//ret=system("iwlist wlan0 channel | grep -E 'SSID|Quality|Encryption|Group' | sed 's/^ *//' >/tmp/wifind.conf");
		//printf("channel=%d\n",ret);

		fp=fopen("/tmp/wifind.conf","r");
		if(fp==NULL)
		{
			fp=fopen("/tmp/wifind.conf","w");
		}
		else
		{
			//fgets(ss,200,fp);
			fscanf(fp,"%s",ss1);
			fscanf(fp,"%s",ss2);
			fscanf(fp,"%s",ss3);
			fscanf(fp,"%s",ss4);
			fscanf(fp,"%s",ss5);
			fscanf(fp,"%s",ss6);
			fscanf(fp,"%s",ss7);
			fscanf(fp,"%s",ss8);
			fscanf(fp,"%s",ss9);
			fscanf(fp,"%s",ss10);
/*			printf("%s\n",ss1);
			printf("%s\n",ss2);
			printf("%s\n",ss3);
			printf("%s\n",ss4);
			printf("%s\n",ss5);
			printf("%s\n",ss6);
			printf("%s\n",ss7);
			printf("%s\n",ss8);
			printf("%s\n",ss9);
			printf("%s\n",ss10);*/
			fclose(fp);
			//if(strncmp(ss,"ESSI",4))
			if(
				(!strncmp(ss1,"ESSID",5))&&
				(!strncmp(ss2,"Quality",7))&&
				(!strncmp(ss3,"Signal",6))&&
				(!strncmp(ss4,"level",5))&&
				(!strncmp(ss5,"dBm",3))&&
				(!strncmp(ss6,"Noise",5))&&
				(!strncmp(ss7,"level",5))&&
				(!strncmp(ss8,"dBm",3))&&
				(!strncmp(ss9,"Encryption",10))&&
				(!strncmp(ss10,"key",3))
			)
			{
				printdecmsg("wifi_ok",0);
//				printf("wifi_ok\n");
			}
			else
			{
//				printf("wifind\n");

				printdecmsg("wifi_init",0);

				system("killall hostapd");
				system("killall udhcpd");

				system("rmmod /home/modules/ar6000.ko");
				sleep(10);
				system("insmod /home/modules/ar6000.ko fwpath=/home/modules devmode=sta,ap");
				sleep(10);
				system("/home/applications/wifi_init &");
			}
		}
		sleep(7200);
	}
}
