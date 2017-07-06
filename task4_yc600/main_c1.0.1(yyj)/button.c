#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>  //open(lcd)
#include "variation.h"
#include <string.h>

extern ecu_info ecu;
extern void get_ip(char ip_buff[16]);

int shutdown_control = 0;		//关机标志

int displaycount=0;			//液晶屏显示当前在线数
int displaymaxcount=0;			//液晶屏显示最大逆变器数
int displaysyspower=0;			//液晶屏显示当前系统功率
char displayip[20]={'\0'};		//液晶屏显示ECU IP Adress
float displayltg=0;			//液晶屏显示历史发电量
char displayconnect[5]={'\0'};			//液晶屏显示+/-WEB

void turnoffall(void)
{
	FILE *fp;

	fp = fopen("/tmp/connect.conf", "w");
	fprintf(fp, "disconnect all");
	fclose(fp);
}

int disparameters()		//显示参数
{
	FILE *fp;
	char status[256];

	get_ip(displayip);
	
	fp = fopen("/tmp/connectemaflag.txt", "r");
	if(fp)
	{
		fgets(status, sizeof(status), fp);
		fclose(fp);
		if(!strcmp("connected", status))
			strcpy(displayconnect, "+WEB");
		if(!strcmp("disconnected", status))
			strcpy(displayconnect, "-WEB");


	}
	else
	{
		strcpy(displayconnect, "-WEB");
	}

	displaysyspower = ecu.system_power;
	displayltg = ecu.life_energy;				//液晶屏上显示的历史发电量
	displaycount = ecu.count;					//液晶屏上显示的当前在线数
	displaymaxcount = ecu.total;				//液晶屏上显示的系统最大逆变器数
	show_systempower_onweb(ecu.system_power);			//页面上显示系统当前功率
	show_currentnumber_onweb(ecu.count);				//页面上显示当前连接数
}

void reset_shutdown(void)					//按钮菜单选中“shut down”后，再选择确定或取消，如果没选择，60秒后自动退出菜单
{
	sleep(60);
	shutdown_control=0;
}

void button(void)						//按钮功能
{
	FILE *fp;
	int fd,fdlcd;
	char buff[1];
	char buflcd[21]={'\0'};
	struct timeval tpstart,tpend,tptmp;
	tpstart.tv_sec=0;
	tpstart.tv_usec=0;
	tptmp.tv_sec=0;
	tptmp.tv_usec=0;
	int timeuse=0,timetmp=0;
	float thousand=1000,million=1000000;
    
	int command = 0;
    
	char temp[10]={'\0'};
	int system_lp=0;
	float lifetime_lpower=0;

	pthread_t shutdown_id;
	pthread_attr_t shutdown_attr;
	int shutdown_ret;

	get_ip(displayip);

	fd=open("/dev/button", O_RDONLY);
	fdlcd=open("/dev/lcd", O_WRONLY);

	while(1)
	{
		read(fd,buff,1);
		if(buff[0]=='1')
		{
			if(tpstart.tv_sec)
			{
				gettimeofday(&tpend,NULL);
				timeuse=(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_usec-tpstart.tv_usec)/1000000;
				switch((timeuse/2-1)%4)
				{
				case 0:
					if(shutdown_control==1)
					{
						printmsg("Cancel");
						shutdown_control=0;sleep(1);break;
					}
					else
					{
						printmsg("Exit Menu");
						sleep(1);
						ioctl(fdlcd,0x01,0);
						break;
					}
				case 1:
					if(shutdown_control==1)
					{
						printmsg("OK");
						turnoffall();
						command=2;
						shutdown_control=0;
						sleep(1);
						break;
					}
					else
					{
						printmsg("Search Device");
						exit(0);
					}
				case 2:
					if(shutdown_control==1)
					{
						printmsg("Cancel");
						shutdown_control=0;
						sleep(1);
						break;
					}
					else
					{
						printmsg("Status");
						ioctl(fdlcd,0x01,0);
						ioctl(fdlcd,0x83,0);

						strcpy(buflcd,"Connected");
						write(fdlcd,buflcd,strlen(buflcd));
						ioctl(fdlcd,0x8e,0);
						if(displaycount<10)
							sprintf(temp, "00%d", displaycount);
						else if(displaycount<100)
							sprintf(temp, "0%d", displaycount);
						else
							sprintf(temp, "%d", displaycount);
						strcpy(buflcd,temp);
						write(fdlcd,buflcd,strlen(buflcd));
						ioctl(fdlcd,0xc3,0);

						strcpy(buflcd,"Total:");
						write(fdlcd,buflcd,strlen(buflcd));
						ioctl(fdlcd,0xce,0);
						if(displaymaxcount<10)
							sprintf(temp, "00%d", displaymaxcount);
						else if(displaymaxcount<100)
							sprintf(temp, "0%d", displaymaxcount);
						else
							sprintf(temp, "%d", displaymaxcount);

						strcpy(buflcd,temp);
						write(fdlcd,buflcd,strlen(buflcd));
						sleep(3);
						ioctl(fdlcd,0x01,0);
						break;
					}
				case 3:
					if(shutdown_control==1)
					{
						printmsg("OK");
						turnoffall();
						command=2;
						shutdown_control=0;
						sleep(1);
						break;
					}
					else
					{
						pthread_attr_init(&shutdown_attr);
						pthread_attr_setdetachstate(&shutdown_attr,PTHREAD_CREATE_DETACHED);
						shutdown_ret=pthread_create(&shutdown_id,&shutdown_attr,(void *) reset_shutdown,NULL);
						if(shutdown_ret!=0)
						{
							printf("Create pthread error!");
						}
						printmsg("Shutdown!");
						shutdown_control=1;
						ioctl(fdlcd,0x01,0);
						break;
					}
				default:
					printmsg("No command!");
					break;
				}
				tpstart.tv_sec=0;
				tpstart.tv_usec=0;
				tptmp.tv_sec=0;
				tptmp.tv_usec=0;
				timetmp=0;
			}
			else
			{
				if(shutdown_control==1)
				{
					ioctl(fdlcd,0x01,0);
					ioctl(fdlcd,0x80,0);
					strcpy(buflcd,"Please choose");
					write(fdlcd,buflcd,strlen(buflcd));
					ioctl(fdlcd,0xc0,0);
					strcpy(buflcd,"OK or Cancel!");
					write(fdlcd,buflcd,strlen(buflcd));
				}
				else
				{
					ioctl(fdlcd,0x01,0);
					ioctl(fdlcd,0x80,0);
          
					write(fdlcd,displayip,strlen(displayip));
					ioctl(fdlcd,0x90,0);
					strcpy(buflcd, displayconnect);
					write(fdlcd,buflcd,strlen(buflcd));
					ioctl(fdlcd,0xc0,0);
					if(displaysyspower<10000)
						sprintf(temp, "%dW", displaysyspower);
					else
					{
						system_lp=displaysyspower/thousand;
						sprintf(temp, "%dKW", system_lp);
					}
					strcpy(buflcd,temp);
					write(fdlcd,buflcd,strlen(buflcd));
					ioctl(fdlcd,0xc6,0);

					if(displayltg<1000)
						sprintf(temp, "%.2fkWh", displayltg);
					else if(displayltg<1000000)
					{
						lifetime_lpower=displayltg/thousand;
						sprintf(temp, "%.2fMWh", lifetime_lpower);
					}
					else
					{
						lifetime_lpower=displayltg/million;
						sprintf(temp, "%.2fGWh", lifetime_lpower);
					}
					strcpy(buflcd,temp);
					write(fdlcd,buflcd,strlen(buflcd));
					if(displaycount==0)
					{
						if(displaycount!=displaymaxcount)
						{
							ioctl(fdlcd,0xd0,0);
							sprintf(temp, "000!");
						}
						else
						{
							ioctl(fdlcd,0xd1,0);
							sprintf(temp, "000");
						}
					}
					else if(displaycount<10)
					{
						if(displaycount!=displaymaxcount)
						{
							ioctl(fdlcd,0xd0,0);
							sprintf(temp, "00%d!", displaycount);
						}
						else
						{
							ioctl(fdlcd,0xd1,0);
							sprintf(temp, "00%d", displaycount);
						}
					}
					else if(displaycount<100)
					{
						if(displaycount!=displaymaxcount)
						{
							ioctl(fdlcd,0xd0,0);
							sprintf(temp, "0%d!", displaycount);
						}
						else
						{
							ioctl(fdlcd,0xd1,0);
							sprintf(temp, "0%d", displaycount);
						}
					}
					else
					{
						if(displaycount!=displaymaxcount)
						{
							ioctl(fdlcd,0xd0,0);
							sprintf(temp, "%d!", displaycount);
						}
						else
						{
							ioctl(fdlcd,0xd1,0);
							sprintf(temp, "%d", displaycount);
						}
					}
					strcpy(buflcd,temp);
					write(fdlcd,buflcd,strlen(buflcd));
				}
			}
		}
		else
		{
			if(tpstart.tv_sec==0)
				gettimeofday(&tpstart,NULL);
			else
			{
				gettimeofday(&tptmp,NULL);
				timetmp=(1000000*(tptmp.tv_sec-tpstart.tv_sec)+(tptmp.tv_usec-tpstart.tv_usec))/1000000;
				switch((timetmp/2-1)%4)
				{
				case 0:
					if(shutdown_control==1)
					{
						ioctl(fdlcd,0x01,0);
						ioctl(fdlcd,0x80,0);
						strcpy(buflcd,"Cancel");
						write(fdlcd,buflcd,strlen(buflcd));
						break;
					}
					else{
						ioctl(fdlcd,0x01,0);
						ioctl(fdlcd,0x80,0);
						strcpy(buflcd,"Exit Menu");
						write(fdlcd,buflcd,strlen(buflcd));
						break;
					}
				case 1:
					if(shutdown_control==1)
					{
						ioctl(fdlcd,0x01,0);
						ioctl(fdlcd,0x80,0);
						strcpy(buflcd,"OK");
						write(fdlcd,buflcd,strlen(buflcd));
						break;
					}
					else{
						ioctl(fdlcd,0x01,0);
						ioctl(fdlcd,0x80,0);
						strcpy(buflcd,"Search Device");
						write(fdlcd,buflcd,strlen(buflcd));
						break;
					}
				case 2:
					if(shutdown_control==1)
					{
						ioctl(fdlcd,0x01,0);
						ioctl(fdlcd,0x80,0);
						strcpy(buflcd,"Cancel");
						write(fdlcd,buflcd,strlen(buflcd));
						break;
					}
					else{
						ioctl(fdlcd,0x01,0);
						strcpy(buflcd,"Status");
						ioctl(fdlcd,0x80,0);
						write(fdlcd,buflcd,strlen(buflcd));
						break;
					}
				case 3:
					if(shutdown_control==1)
					{
						ioctl(fdlcd,0x01,0);
						ioctl(fdlcd,0x80,0);
						strcpy(buflcd,"OK");
						write(fdlcd,buflcd,strlen(buflcd));
						break;
					}
					else{
						ioctl(fdlcd,0x01,0);
						strcpy(buflcd,"Turn off all");
						ioctl(fdlcd,0x80,0);
						write(fdlcd,buflcd,strlen(buflcd));
						strcpy(buflcd,"inverters");
						ioctl(fdlcd,0xC0,0);
						write(fdlcd,buflcd,strlen(buflcd));
						break;
					}
				default:
					break;
				}
			}
		}
		usleep(10000);
	}
}

void button_pthread(void)			//开启按键菜单功能（即开启按键的独立线程）
{
	pthread_t button_id;
	int button_ret=0;

	button_ret=pthread_create(&button_id,NULL,(void *) button,NULL);
	if(button_ret!=0)
	{
		printf ("Create pthread error!");
	}
}
