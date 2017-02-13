#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>	//ifr
#include <sys/ioctl.h>	//SIOCGIFADDR
#include <netinet/in.h>
#include <arpa/inet.h>
#include "variation.h"
#include <string.h>

extern ecu_info ecu;
void get_ip(char ip_buff[16])					//获取ECU的IP
{
	struct ifreq ifr;
	int inet_sock;
	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, "eth0");

	if(ioctl(inet_sock,SIOCGIFADDR,&ifr)<0)
		;
	else
		strcpy(ip_buff,inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	close(inet_sock);
}

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


void show_systempower_onweb(int system_p_display)	//把系统功率写入文件，home页面点击时会读此文件
{
	FILE *fp;
	fp=fopen("/tmp/system_p_display.conf","w");
	fprintf(fp,"%d",system_p_display);
	fclose(fp);
}


void show_currentnumber_onweb(int current_number)	//把当前逆变器的数量写入文件，home页面点击时会读此文件
{
	FILE *fp;
	fp=fopen("/tmp/current_number.conf","w");
	fprintf(fp,"%d",current_number);
	fclose(fp);
}

int display_connection(inverter_info *firstinverter)
{
	FILE *fp;
	int i;

	inverter_info *inverter = firstinverter;
	fp = fopen("/tmp/connectresult.txt", "w");
	if(fp)
	{
		for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++)
		{
			if(1 == inverter->dataflag)
			{
				/*
				if(1 == inverter->model)
				{
					if('1' == inverter->status_web[12])
						fprintf(fp, "Turned Off\n");
					else
						fprintf(fp, "Turned On\n");
				}
				else
				{
					if(inverter->op <= 1)
						fprintf(fp, "Turned Off\n");
					else
						fprintf(fp, "Turned On\n");
				}
				*/
				if('1' == inverter->status_web[18])
					fprintf(fp, "Turned Off\n");
				else
					fprintf(fp, "Turned On\n");
			}
			else
				fprintf(fp, "-\n");
			inverter++;
		}
		fclose(fp);
	}
}

int displaygfdi(inverter_info *firstinverter)		//显示gfdi，20120409
{
	FILE *fp;
	int i;

	inverter_info *inverter = firstinverter;
	fp = fopen("/tmp/gfdiresult.txt", "w");
	for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++){
		if(1 == inverter->dataflag){
			if('1' == inverter->status_web[11])
				fprintf(fp, "GFDI Locked\n");
			else
				fprintf(fp, "Normal\n");
		}
		else
			fprintf(fp, "-\n");

		inverter++;
	}
	fclose(fp);
}
/*
int display_protect_result(inverter_info *inverter)
{
	int i;
	FILE *fp;

	fp = fopen("/etc/yuneng/presetdata.txt", "w");
	for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++, inverter++){
		if((-1 == inverter->protect_voltage_min) && (-1 == inverter->protect_voltage_max))
			fprintf(fp, "-, -, -, -\n");
		else
			fprintf(fp, "%d, %d, %d, %d, %d\n", inverter->protect_voltage_min, inverter->protect_voltage_max, inverter->protect_frequency_min, inverter->protect_frequency_max, inverter->recovery_time);
	}
	fclose(fp);

	return 1;
}
*/
int displayonweb(inverter_info *firstinverter, char *sendcommanddatatime)		//sendcommanddatatime：最近一次上传数据的时间
{
	int i;
	char datatime[20] = {'\0'};
	char temp[10] = {'\0'};
	FILE *fp;
	inverter_info *inverter = firstinverter;

	memset(datatime, '\0', 20);

	datatime[0] = sendcommanddatatime[0];
	datatime[1] = sendcommanddatatime[1];
	datatime[2] = sendcommanddatatime[2];
	datatime[3] = sendcommanddatatime[3];
	datatime[4] = '-';
	datatime[5] = sendcommanddatatime[4];
	datatime[6] = sendcommanddatatime[5];
	datatime[7] = '-';
	datatime[8] = sendcommanddatatime[6];
	datatime[9] = sendcommanddatatime[7];
	datatime[10] = ' ';
	datatime[11] = sendcommanddatatime[8];
	datatime[12] = sendcommanddatatime[9];
	datatime[13] = ':';
	datatime[14] = sendcommanddatatime[10];
	datatime[15] = sendcommanddatatime[11];
	datatime[16] = ':';
	datatime[17] = sendcommanddatatime[12];
	datatime[18] = sendcommanddatatime[13];

	fp = fopen("/tmp/parameters.conf","w");
	if(fp)
	{
		for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++)
		{

			if((1 == inverter->model)||(2 == inverter->model))
			{				//ZK,YC250
				if(1 == inverter->dataflag)
				{
					fprintf(fp,"%s, %d, %.1f, %d, %d, %s\n", inverter->inverterid, inverter->op, inverter->gf, inverter->gv, inverter->it, datatime);
				}
				else
				{
					fprintf(fp,"%s,  ,  ,  ,  , -\n", inverter->inverterid);
				}
			}
			else if((4 == inverter->model)||(3 == inverter->model))
			{		//ZK,YC500
				if(1 == inverter->dataflag)
				{
					fprintf(fp,"%s-A, %d, %.1f, %d, %d, %s\n", inverter->inverterid, inverter->op, inverter->gf, inverter->gv, inverter->it, datatime);
					fprintf(fp,"%s-B, %d, %.1f, %d, %d, %s\n", inverter->inverterid, inverter->opb, inverter->gf, inverter->gv, inverter->it, datatime);
				}
				else
				{
					fprintf(fp,"%s-A,  ,  ,  ,  , -\n", inverter->inverterid);
					fprintf(fp,"%s-B,  ,  ,  ,  , -\n", inverter->inverterid);
				}
			}
			else if((5 == inverter->model)||(6 == inverter->model))
			{		//YC1000
				if( 1 == inverter->dataflag)
				{
					fprintf(fp,"%s-1, %d, %.1f, A: %d, %d, %s\n", inverter->inverterid, inverter->op, inverter->gf, inverter->gv, inverter->it, datatime);
					fprintf(fp,"%s-2, %d, %.1f, B: %d, %d, %s\n", inverter->inverterid, inverter->opb, inverter->gf, inverter->gvb, inverter->it, datatime);
					fprintf(fp,"%s-3, %d, %.1f, C: %d, %d, %s\n", inverter->inverterid, inverter->opc, inverter->gf, inverter->gvc, inverter->it, datatime);
					fprintf(fp,"%s-4, %d, %.1f, -, %d, %s\n", inverter->inverterid, inverter->opd, inverter->gf, inverter->it, datatime);
				}
				else
				{
					fprintf(fp,"%s-1,  ,  ,  ,  , -\n", inverter->inverterid);
					fprintf(fp,"%s-2,  ,  ,  ,  , -\n", inverter->inverterid);
					fprintf(fp,"%s-3,  ,  ,  ,  , -\n", inverter->inverterid);
					fprintf(fp,"%s-4,  ,  ,  ,  , -\n", inverter->inverterid);
				}
			}
			else
			{
				if(1 == inverter->dataflag)
				{
					fprintf(fp,"%s, %d, %.1f, %d, %d, %s\n", inverter->inverterid, inverter->op, inverter->gf, inverter->gv, inverter->it, datatime);
				}
				else
				{
					fprintf(fp,"%s,  ,  ,  ,  , -\n", inverter->inverterid);
				}
			}
			inverter++;
		}
	


		fclose(fp);
	}
	display_connection(firstinverter);
	displaygfdi(firstinverter);

	return 0;
}

int display_optimizer(inverter_info *firstinverter, char *sendcommanddatatime)		//sendcommanddatatime：最近一次上传数据的时间
{
	int i;
	char datatime[20] = {'\0'};
	char temp[10] = {'\0'};
	FILE *fp;
	inverter_info *inverter = firstinverter;

	memset(datatime, '\0', 20);

	datatime[0] = sendcommanddatatime[0];
	datatime[1] = sendcommanddatatime[1];
	datatime[2] = sendcommanddatatime[2];
	datatime[3] = sendcommanddatatime[3];
	datatime[4] = '-';
	datatime[5] = sendcommanddatatime[4];
	datatime[6] = sendcommanddatatime[5];
	datatime[7] = '-';
	datatime[8] = sendcommanddatatime[6];
	datatime[9] = sendcommanddatatime[7];
	datatime[10] = ' ';
	datatime[11] = sendcommanddatatime[8];
	datatime[12] = sendcommanddatatime[9];
	datatime[13] = ':';
	datatime[14] = sendcommanddatatime[10];
	datatime[15] = sendcommanddatatime[11];
	datatime[16] = ':';
	datatime[17] = sendcommanddatatime[12];
	datatime[18] = sendcommanddatatime[13];

	fp = fopen("/tmp/optimizer_last_data.txt","w");
	if(fp)
	{
		for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++)
		{

			if(1 == inverter->dataflag)
			{
				fprintf(fp,"%s, %s, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %d\n", inverter->inverterid, datatime, inverter->output_voltage,inverter->output_current, inverter->output_power, inverter->input_voltage_pv1, inverter->input_current_pv1, inverter->input_power_pv1, inverter->input_voltage_pv2, inverter->input_current_pv2, inverter->input_power_pv2, inverter->temperature);
			}
			else
			{
				fprintf(fp,"%s,  ,  ,  ,  ,  ,  ,  ,  ,  ,  ,-\n", inverter->inverterid);
			}

			inverter++;
		}

		fclose(fp);
	}

	return 0;
}

//获取ECU的有线IP和无线IP
void get_ip_of_all(char ip_buff[2][20])
{
	struct ifreq ifr;
	int inet_sock;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	//获取有线IP
	strcpy(ifr.ifr_name, "eth0");
	strcpy(ip_buff[0], "L: ");
	if (ioctl(inet_sock,SIOCGIFADDR,&ifr)<0) {
//		strcat(ip_buff[0]," NO IP Address");
	}
	else{
		strcat(ip_buff[0],inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	}
	//获取无线IP
	strcpy(ifr.ifr_name, "wlan0");
	strcpy(ip_buff[1], "W: ");
	if (ioctl(inet_sock,SIOCGIFADDR,&ifr)<0) {
//		strcat(ip_buff[1]," NO IP Address");
	}
	else{
		strcat(ip_buff[1],inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	}

	close(inet_sock);
}

//提示用户输入逆变器ID，并交替显示IP地址
void display_input_id(void)
{
	static char display[2][20] = {" Input Optimizer ID", "    on Local Web"};
	char ip[2][20] = {'\0'};
	int fd_lcd;

	fd_lcd = open("/dev/lcd",O_WRONLY);

	//打印有线无线IP地址
	get_ip_of_all(ip);
	ioctl(fd_lcd, 0x01, 0);
	ioctl(fd_lcd, 0x80, 0);
	write(fd_lcd, ip[0], strlen(ip[0]));
	ioctl(fd_lcd, 0xC0, 0);
	write(fd_lcd, ip[1], strlen(ip[1]));
	sleep(5);

	//打印“Input Inverter ID on Local Web”
	ioctl(fd_lcd, 0x01, 0);
	ioctl(fd_lcd, 0x80, 0);
	write(fd_lcd, display[0], strlen(display[0]));
	ioctl(fd_lcd, 0xC0, 0);
	write(fd_lcd, display[1], strlen(display[1]));

	close(fd_lcd);
}

//在液晶屏和Web页面上更新显示参数
void display_on_lcd_and_web()
{
	FILE *fp;

	fp = fopen("/tmp/real_time_data.txt", "w");
	if (fp) {
		fprintf(fp, "%d\n", ecu.system_power);
		fprintf(fp, "%f\n", ecu.life_energy);
		fprintf(fp, "%d\n", ecu.count);
		fprintf(fp, "%d\n", ecu.total);
		fclose(fp);
	}
	show_systempower_onweb(ecu.system_power);	//页面上显示系统当前功率
	show_currentnumber_onweb(ecu.count);		//页面上显示当前连接数
}
