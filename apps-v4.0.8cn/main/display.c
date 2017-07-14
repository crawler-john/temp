#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <socket.h>
#include "variation.h"

void display_init(void)			//在液晶屏上显示“Loading...”
{
	int fd_lcd;
	char display[21]={'\0'};

	fd_lcd=open("/dev/lcd",O_WRONLY);
	ioctl(fd_lcd,0x01,0);
	strcpy(display,"Loading...");
	ioctl(fd_lcd,0x85,0);
	write(fd_lcd,display,strlen(display));
	close(fd_lcd);
	sleep(1);
}

void display_scanning(void)		//在液晶屏上显示“Searching       V3.0”“139.168.200.XX”
{
	int fd_lcd;
	char display[32]={'\0'};
	char ip[16]={'\0'};
	char version[32]={'\0'};
	FILE *fp;
	int i=0, j,k;

	for(i=0; i<3; i++){
		fp=fopen("/etc/yuneng/version.conf","r");
		fgets(version, sizeof(version), fp);
		fclose(fp);
		if('\n' == version[strlen(version)-1])
			version[strlen(version)-1] = '\0';
		fd_lcd=open("/dev/lcd",O_WRONLY);
		ioctl(fd_lcd,0x01,0);
		strcpy(display,"Searching");
		k = 11-strlen(version);
		for(j=0; j<k; j++)
			strcat(display, " ");
		strcat(display,version);
		ioctl(fd_lcd,0x80,0);
		write(fd_lcd,display,strlen(display));
		memset(display,'\0',sizeof(display));
		//getformatetime(display);
		get_ip(ip);
		ioctl(fd_lcd,0xc0,0);
		write(fd_lcd,ip,strlen(ip));
		close(fd_lcd);

		sleep(1);
	}
}

//提示用户输入逆变器ID，并交替显示IP地址
void display_input_id(void)
{
	static char display[2][20] = {" Input Inverter ID", "    on Local Web"};
	char ip[2][20] = {'\0'};
	int fd_lcd;
	FILE *fp;
	char wifi_val[5]={'\0'};
	int flag_wifi=1;

	fd_lcd = open("/dev/lcd",O_WRONLY);

	fp = fopen("/etc/yuneng/wifi.conf","r");
    if (fp) {
    	fgets(wifi_val,5,fp);
    	fclose(fp);
      	if(!strncmp(wifi_val,"0",1))
       		flag_wifi=0;
    }

	//打印有线无线IP地址
	get_ip_of_all(ip);
	ioctl(fd_lcd, 0x01, 0);
	ioctl(fd_lcd, 0x80, 0);
	write(fd_lcd, ip[0], strlen(ip[0]));
	if(flag_wifi==1){
	ioctl(fd_lcd, 0xC0, 0);
	write(fd_lcd, ip[1], strlen(ip[1]));
	}
	sleep(5);

	//打印“Input Inverter ID on Local Web”
	ioctl(fd_lcd, 0x01, 0);
	ioctl(fd_lcd, 0x80, 0);
	write(fd_lcd, display[0], strlen(display[0]));
	ioctl(fd_lcd, 0xC0, 0);
	write(fd_lcd, display[1], strlen(display[1]));

	close(fd_lcd);
}


