#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <net/if.h>

//#define DEBUG 0
//#define DEBUGLOG 0
char ip[20]={'\0'};

void printmsg(char *msg)		//打印字符串
{
#ifdef DEBUG
	printf("button==>%s!\n", msg);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/button.log", "a");
	if(fp)
	{
		fprintf(fp, "%s!\n", msg);
		fclose(fp);
	}
#endif
}

void printdecmsg(char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	printf("button==>%s: %d!\n", msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/button.log", "a");
	if(fp)
	{
		fprintf(fp, "%s: %d!\n", msg, data);
		fclose(fp);
	}
#endif
}

int get_button_status()
{
	int fd, ret;
	char buff[2]={'1'};

	fd = open("/dev/button",O_RDONLY);
	if(fd){
		ret = read(fd, buff, 1);
		close(fd);
	}

	if('1' == buff[0])
		return 1;
	else
		return 0;
}

void get_ip(char ip_buff[18])					//获取ECU的IP
{
	static int ip_type = 1;
	struct ifreq ifr;
	int inet_sock;
	FILE *fp;
	char wifi_val[5]={'\0'};
	int flag_wifi=1;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	fp = fopen("/etc/yuneng/wifi.conf","r");
    if (fp) {
    	fgets(wifi_val,5,fp);
    	fclose(fp);
      	if(!strncmp(wifi_val,"0",1))
       		flag_wifi=0;
    }
	//选取网络接口
	ip_type = !ip_type;
	if (ip_type) {
		if(flag_wifi==0){
			strcpy(ifr.ifr_name, "eth0");
			strcpy(ip_buff, "L:");
		}
		else{
		strcpy(ifr.ifr_name, "wlan0");
		strcpy(ip_buff, "W:");
		}
	}
	else {
		strcpy(ifr.ifr_name, "eth0");
		strcpy(ip_buff, "L:");
	}

	//获取IP地址
	if (ioctl(inet_sock,SIOCGIFADDR,&ifr)<0) {
//		strcat(ip_buff," NO IP Address");
	}
	else{
		strcat(ip_buff,inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	}

	close(inet_sock);	
}

void get_wlan0_ip(char ip_buff[16])					//获取ECU的IP
{
	struct ifreq ifr;
	int inet_sock;
	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, "wlan0");

	if(ioctl(inet_sock,SIOCGIFADDR,&ifr)<0)
		;
	else{
		strcpy(ip_buff,inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	}
	close(inet_sock);
}

int get_connect_status(char *displayconnect)
{
	FILE *fp;
	char status[32];

	fp = fopen("/etc/yuneng/gprs.conf", "r");
	if(fp){
		fgets(status, sizeof(status), fp);
		fclose(fp);
		if('1' == status[0]){
			strcpy(displayconnect, "G");
			return 0;
		}
	}

	fp = fopen("/tmp/connectemaflag.txt", "r");
	if(fp)
	{
		fgets(status, sizeof(status), fp);
		fclose(fp);
		if(!strcmp("connected", status))
			strcpy(displayconnect, "+W");
		if(!strcmp("disconnected", status))
			strcpy(displayconnect, "-W");
	}
	else
		strcpy(displayconnect, "-W");
}

int get_data(int *power, float *energy, int *count, int *maxcount)
{
	FILE *fp;
	char buff[256];

	fp = fopen("/tmp/real_time_data.txt", "r");
	if(fp){
		fgets(buff, sizeof(buff), fp);
		*power = atoi(buff);
		fgets(buff, sizeof(buff), fp);
		*energy = atof(buff);
		fgets(buff, sizeof(buff), fp);
		*count = atoi(buff);
		fgets(buff, sizeof(buff), fp);
		*maxcount = atoi(buff);
		fclose(fp);
	}
}

int show_exit()
{
	int fdlcd;
    char buflcd[21]={'\0'};

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd, 0x01, 0);
		ioctl(fdlcd, 0x80, 0);
		strcpy(buflcd, "Exit Menu");
		write(fdlcd, buflcd, strlen(buflcd));
		close(fdlcd);
	}
}

int show_grid_environment()
{
	int fdlcd;
    char buflcd[21]={'\0'};

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd, 0x01, 0);
		ioctl(fdlcd, 0x80, 0);
		strcpy(buflcd, "Signal Level");
		write(fdlcd, buflcd, strlen(buflcd));
		close(fdlcd);
	}
}

int show_grid_environment_value()
{
	int fdlcd, i, level=0;
    char buflcd[21]={'\0'};
    char value[32]={'\0'};
    char buff[32]={'\0'};
    FILE *fp;

    fp = fopen("/etc/yuneng/plc_grid_quality.txt", "r");
    if(fp){
    	fgets(value, sizeof(value), fp);
    	fclose(fp);
    }

    if(atoi(value) > 5)
    	level = 5;
    else
    	level = atoi(value);
    strcpy(buff, "[");
    for(i=0; i<level; i++)
    	strcat(buff, "=");

    for(i=level; i<5; i++)
    	strcat(buff, " ");
    strcat(buff, "]");

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd, 0x01, 0);
		ioctl(fdlcd, 0x80, 0);
		strcpy(buflcd, "Level:");
		write(fdlcd, buflcd, strlen(buflcd));

		ioctl(fdlcd, 0xc0, 0);
		write(fdlcd, buff, strlen(buff));

		sleep(3);
		close(fdlcd);
	}
}

int show_status()
{
	int fdlcd;
    char buflcd[21]={'\0'};

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd, 0x01, 0);
		ioctl(fdlcd, 0x80, 0);
		strcpy(buflcd,"Status");
		write(fdlcd, buflcd, strlen(buflcd));
		close(fdlcd);
	}
}

int show_count()
{
	int fdlcd;
    char buflcd[21]={'\0'};
    int displaysyspower=0;
    float displayltg=0;
    int displaycount=0;
    int displaymaxcount=0;

    get_data(&displaysyspower, &displayltg, &displaycount, &displaymaxcount);

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd,0x01,0);
		ioctl(fdlcd,0x83,0);

		strcpy(buflcd,"Connected");
		write(fdlcd,buflcd,strlen(buflcd));
		ioctl(fdlcd,0x8e,0);
		sprintf(buflcd, "%03d", displaycount);
		write(fdlcd,buflcd,strlen(buflcd));

		ioctl(fdlcd,0xc3,0);
		strcpy(buflcd,"Total:");
		write(fdlcd,buflcd,strlen(buflcd));
		ioctl(fdlcd,0xce,0);
		sprintf(buflcd, "%03d", displaymaxcount);
		write(fdlcd,buflcd,strlen(buflcd));
		sleep(3);

		close(fdlcd);
	}
}

int show_wlan0_ip()
{
	int fdlcd;
    char buflcd[21]={'\0'};

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd, 0x01, 0);
		ioctl(fdlcd, 0x80, 0);
		strcpy(buflcd,"Wlan IP");
		write(fdlcd, buflcd, strlen(buflcd));
		close(fdlcd);
	}
}

int show_wlan0_ip_value()
{
	int fdlcd;
	char buflcd[21]={'\0'};
	char ip[21]={'\0'};

	get_wlan0_ip(ip);

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd,0x01,0);
		ioctl(fdlcd,0x80,0);

		strcpy(buflcd,"Wlan IP:");
		write(fdlcd,buflcd,strlen(buflcd));

		ioctl(fdlcd,0xc0,0);
		strcpy(buflcd, ip);
		write(fdlcd,buflcd,strlen(buflcd));
		sleep(3);

		close(fdlcd);
	}
}

int show_turn_on_off()
{
	int fdlcd;
    char buflcd[21]={'\0'};

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd, 0x01, 0);
		ioctl(fdlcd, 0x80, 0);
		strcpy(buflcd,"Turn off all");
		write(fdlcd, buflcd, strlen(buflcd));
		close(fdlcd);
	}
}

int show_cancel()
{
	int fdlcd;
    char buflcd[21]={'\0'};

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd, 0x01, 0);
		ioctl(fdlcd, 0x80, 0);
		strcpy(buflcd, "Cancel");
		write(fdlcd, buflcd, strlen(buflcd));
		close(fdlcd);
	}
}

int show_ok()
{
	int fdlcd;
    char buflcd[21]={'\0'};

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd, 0x01, 0);
		ioctl(fdlcd, 0x80, 0);
		strcpy(buflcd, "OK");
		write(fdlcd, buflcd, strlen(buflcd));
		close(fdlcd);
	}
}

int display_data()
{
    int fdlcd;
    char buflcd[21]={'\0'};
    char temp[10]={'\0'};
    char displayconnect[5]={'\0'};
    int displaysyspower=0;
    float displayltg=0;
    int displaycount=0;
    int displaymaxcount=0;

    get_ip(ip);
    get_connect_status(displayconnect);
    get_data(&displaysyspower, &displayltg, &displaycount, &displaymaxcount);

	fdlcd = open("/dev/lcd", O_WRONLY);
	if(fdlcd){
		ioctl(fdlcd,0x01,0);
		ioctl(fdlcd,0x80,0);

		write(fdlcd, ip, strlen(ip));		//显示IP地址

		if (1 == strlen(displayconnect)) {
			ioctl(fdlcd, 0x93, 0);//G
		} else {
			ioctl(fdlcd, 0x92, 0);//+/-W
		}
		strcpy(buflcd, displayconnect);
		write(fdlcd, buflcd, strlen(buflcd));				//显示+/-WEB

		ioctl(fdlcd,0xc0,0);
		if(displaysyspower<10000)
			sprintf(temp, "%dW", displaysyspower);
		else{
			displaysyspower = displaysyspower / 1000;
			sprintf(temp, "%dKW", displaysyspower);
		}
		strcpy(buflcd,temp);
		write(fdlcd,buflcd,strlen(buflcd));			//显示功率

		ioctl(fdlcd,0xc6,0);
		if(displayltg<1000)
			sprintf(temp, "%.2fkWh", displayltg);
		else if(displayltg<1000000){
			displayltg = displayltg / 1000;
			sprintf(temp, "%.2fMWh", displayltg);
		}
		else{
			displayltg = displayltg / 1000000;
			sprintf(temp, "%.2fGWh", displayltg);
		}
		strcpy(buflcd,temp);
		write(fdlcd,buflcd,strlen(buflcd));			//显示电量

		if(displaycount!=displaymaxcount){
			sprintf(temp, "%03d!", displaycount);
			ioctl(fdlcd,0xd0,0);
		}
		else{
			sprintf(temp, "%03d", displaycount);
			ioctl(fdlcd,0xd1,0);
		}

		strcpy(buflcd,temp);
		write(fdlcd,buflcd,strlen(buflcd));			//显示逆变器数量
	}
	close(fdlcd);
}

void turnoffall(void)
{
	FILE *fp;

	fp = fopen("/tmp/connect.conf", "w");
	fprintf(fp, "disconnect all");
	fclose(fp);
}

int process_level1()
{
	struct timeval start_time, end_time;
	int ret=0;

	gettimeofday(&start_time, NULL);
	while(1){
		gettimeofday(&end_time, NULL);
		if(!get_button_status()){
			switch((((end_time.tv_sec-start_time.tv_sec)*1000000 + end_time.tv_usec-start_time.tv_usec)/1000000)%4){
			case 0:
				show_exit();
				break;
			case 1:
				show_grid_environment();
				break;
			case 2:
				show_status();
				break;
			case 3:
				show_turn_on_off();
				break;
			default:
				;
			}
		}
		else{
			switch((((end_time.tv_sec-start_time.tv_sec)*1000000 + end_time.tv_usec-start_time.tv_usec)/1000000)%4){
			case 0:
				break;
			case 1:
				show_grid_environment_value();
				break;
			case 2:
				show_count();
				break;
			case 3:
				ret = 2;
				break;
			default:
				;
			}
			return ret;
		}
		usleep(50000);
	}
}

int process_turn_on_off()
{
	struct timeval start_time, end_time;

	gettimeofday(&start_time, NULL);
	while(1){
		gettimeofday(&end_time, NULL);
		if(!get_button_status()){
			switch((((end_time.tv_sec-start_time.tv_sec)*1000000 + end_time.tv_usec-start_time.tv_usec)/1000000)%2){
			case 0:
				show_cancel();
				break;
			case 1:
				show_ok();
				break;
			default:
				;
			}
		}
		else{
			switch((((end_time.tv_sec-start_time.tv_sec)*1000000 + end_time.tv_usec-start_time.tv_usec)/1000000)%2){
			case 0:
				break;
			case 1:
				sleep(3);
				turnoffall();
				break;
			default:
				;
			}
			return 0;
		}
		usleep(1000);
	}
}

int main(void)
{
	int level=0;		//控制一级菜单和二级菜单
	int menu=0;			//控制菜单
	struct timeval last_show_data_time, current_data_time, enter_level2_time;

	display_data();
	gettimeofday(&last_show_data_time, NULL);

	while(1)
	{
		if(!get_button_status()){		//按下
			if(2 == level){
				process_turn_on_off();
				level = 0;
				gettimeofday(&last_show_data_time, NULL);
				display_data();
			}
			else{
				level = process_level1();
				if(2 == level)
					gettimeofday(&enter_level2_time, NULL);
				if(!level){
					level = 0;
					gettimeofday(&last_show_data_time, NULL);
					display_data();
				}
			}
		}
		else{
			gettimeofday(&current_data_time, NULL);
			if(2 == level){
				if((current_data_time.tv_sec-enter_level2_time.tv_sec) > 60){
					level = 0;
					gettimeofday(&last_show_data_time, NULL);
					display_data();
				}
			}
			else{
				if((current_data_time.tv_sec-last_show_data_time.tv_sec) > 60){
					display_data();
					last_show_data_time.tv_sec = current_data_time.tv_sec;
					last_show_data_time.tv_usec = current_data_time.tv_usec;
				}
			}
		}

		usleep(500000);
	}

	return 0;
}
