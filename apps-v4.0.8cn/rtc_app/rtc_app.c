#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

int main(void)
{
    int fd,retr=0,retw=0;
    char buff[15]={'\0'};
    char year[5]={'\0'};
    char month[3]={'\0'};
    char day[3]={'\0'};
    char hour[3]={'\0'};
    char minute[3]={'\0'};
    char second[3]={'\0'};
    char set_date[15]={'\0'};
    char set_time[15]={'\0'};

    time_t tm;
    struct tm set_tm;
    struct timeval tv;
    time_t timep;

    fd=open("/dev/rtc2",O_RDONLY);

    retr=read(fd,buff,14);
    year[0]=buff[0];
    year[1]=buff[1];
    year[2]=buff[2];
    year[3]=buff[3];
    month[0]=buff[4];
    month[1]=buff[5];
    day[0]=buff[6];
    day[1]=buff[7];
    hour[0]=buff[8];
    hour[1]=buff[9];
    minute[0]=buff[10];
    minute[1]=buff[11];
    second[0]=buff[12];
    second[1]=buff[13];

    close(fd);

    set_tm.tm_year=atoi(year)-1900;
    set_tm.tm_mon=atoi(month)-1;
    set_tm.tm_mday=atoi(day);
    set_tm.tm_hour=atoi(hour);
    set_tm.tm_min=atoi(minute);
    set_tm.tm_sec=atoi(second);

    timep = mktime(&set_tm);
	if(set_tm.tm_isdst>0)
		timep -= set_tm.tm_isdst*3600;
    tv.tv_sec = timep;
    tv.tv_usec = 0;

    if(settimeofday(&tv,NULL)<0)
    {
     printf("Set system datatime error!\n");
     //return -1;
    }

    printf("%s\n",buff);
    printf("%s,%s,%s,%s,%s,%s\n",year,month,day,hour,minute,second);

    return 0;
}
