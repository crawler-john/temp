/*****************************************************************************
name:manager
date:2010.11
description:manage all application on ECU.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

void display_init(void)
{
	int fd_lcd;
	char display[21]={'\0'};
	int i;

	for(i=0; i<3; i++){
		fd_lcd=open("/dev/lcd",O_WRONLY);
		ioctl(fd_lcd,0x01,0);
		strcpy(display,"Loading...");
		ioctl(fd_lcd,0x85,0);
		write(fd_lcd,display,strlen(display));
		close(fd_lcd);
		sleep(1);
	}
}

int createfile(void)
{
	FILE *fp;

	fp = fopen("/tmp/process_gfdi.conf", "w");
	fclose(fp);
	fp = fopen("/tmp/gfdiresult.txt", "w");
	fclose(fp);
	fp = fopen("/tmp/presetdata.conf", "w");
	fclose(fp);
	fp = fopen("/tmp/connectresult.txt", "w");
	fclose(fp);
	fp = fopen("/tmp/connect.conf", "w");
	fclose(fp);
	fp = fopen("/tmp/setmaxpower.conf", "w");
	fclose(fp);
	fp = fopen("/tmp/setfixpower.conf", "w");
	fclose(fp);

	return 0;
}

int main(void)
{
    int dhcp_flag;
    char flag = '1';
    char temp[5] = {'\0'};
    char wifi[5] = {'\0'};
    FILE *fp,*fpw;

	createfile();
	display_init();
    system("/home/applications/macapp");
    sleep(5);

    fp = fopen("/etc/yuneng/dhcp.conf","r");
    if (fp) {
    	fgets(temp,5,fp);
    	flag = temp[0];
    	fclose(fp);
    }
    if ('0' == flag) {
    	//设置静态IP
		system("/etc/rcS.d/S40networking stop");
		sleep(1);
		system("/etc/rcS.d/S40networking start");
		system("cp /etc/yuneng/resolv.conf /etc/");
	}

	system("/home/applications/rtc_app &");		//初始化系统时间
    system("/home/applications/network.exe &");
    system("/usr/local/lighttpd/sbin/lighttpd -f /usr/local/lighttpd/config/lighttpd.conf");				//开启页面服务器
    system("/home/applications/idwriter &");		//ID烧写程序
    system("/home/applications/ntpapp.exe &");		//NTP管理程序
    system("/home/applications/monitor.exe &");		//main.exe监控程序
    system("/home/applications/clientmonitor &");	//client监控程序
    system("/home/applications/gprsmonitor &");	//client监控程序
    system("/home/applications/resmonitor &");		//半夜重启main.exe程序
    system("/home/applications/buttonreset &");		//恢复出厂设置程序
    system("/home/applications/updatemanager &");	//自动更新监控程序
	system("/home/applications/phone_server_monitor &");	//自动更新监控程序
	system("/home/applications/control_client_monitor &");
	system("/home/applications/single_update_monitor &");
	system("/home/applications/autoupdate_restart &");
	system("/home/applications/diagnosis_network &");
	fpw = fopen("/etc/yuneng/wifi.conf","r");
    if (fpw) {
    	fgets(wifi,5,fpw);
    	fclose(fpw);
       	if(!strncmp("0",wifi,1))
       		return 0;
    }
   	system("/home/applications/wifi_init &");		//WIFI初始化
   	system("/home/applications/wifind &");			//WIFI自动修复
    return 0;
}
