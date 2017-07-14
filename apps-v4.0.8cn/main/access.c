#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "variation.h"
#include "datetime.h"

extern int caltype;		//计算方式，NA版和非NA版的区别
extern int HeartIntervalTime;

void write_control_file(void)
{
	FILE *fp;
	fp=fopen("/tmp/webcommand.conf","w"); //Necessary,else read_control will be error!
	fclose(fp);
}

void get_heart_interval_time()		//读取心跳时间，如果没有心跳时间文件则创建心跳文件（时间为300S）
{
	char interval_time[50];
	FILE *fp = fopen("/etc/yuneng/heart_interval_time.txt", "r");
	if(fp){
		fgets(interval_time, 50, fp);
		HeartIntervalTime = atoi(interval_time);
		fclose(fp);
	}else
	{
		fp = fopen("/etc/yuneng/heart_interval_time.txt", "w");
		fprintf(fp, "%d\n", 300);
		HeartIntervalTime = 300;
		fclose(fp);
	}
	printf("HeartIntervalTime:%d\n",HeartIntervalTime);
}

void write_parafile(void)		//写两个空白文件，否则刚启动时，无法打开实时数据页面
{
	FILE *fp;
	fp=fopen("/tmp/parameters.conf","w");
	fclose(fp);
	fp=fopen("/tmp/paratime.conf","w");
	fclose(fp);
}

void writesystempower(int system_p_display)	//把系统功率写入文件，home页面点击时会读此文件
{
	FILE *fp;
	fp=fopen("/tmp/system_p_display.conf","w");
	fprintf(fp,"%d",system_p_display);
	fclose(fp);
}

void write_real_systempower(int system_p_display)	//把系统功率写入文件，home页面点击时会读此文件
{
	FILE *fp;
	fp=fopen("/tmp/real_system_p_display.conf","w");
	fprintf(fp,"%d",system_p_display);
	fclose(fp);
}

void writecurrentnumber(int current_number)	//把当前逆变器的数量写入文件，home页面点击时会读此文件
{
	FILE *fp;
	fp=fopen("/tmp/current_number.conf","w");
	fprintf(fp,"%d",current_number);
	fclose(fp);
}

void writelastreporttime()			//把最后一次上报的时间写入文件，实时数据页面点击时会读此文件
{
	FILE *fp;
	fp=fopen("/tmp/lastreporttime.conf","w");
	fprintf(fp,"-");
	fclose(fp);
}

int initialize(void)				//初始设置函数，调用了上述的函数
{
	write_control_file();
	write_parafile();
	writecurrentnumber(0);
	writesystempower(0);
	writelastreporttime();
	get_heart_interval_time();
	//writepresetdata();
}

int get_timeout(void)				//读取逆变器和ECU数据通信时的超时时间
{
	FILE *fp;
	int timeout=2;

	fp=fopen("/etc/yuneng/timeout.conf","r");
	fscanf(fp,"%d",&timeout);

	printdecmsg("timeout", timeout);

	fclose(fp);

	return timeout;
}

int getecuid(char ecuid[13])			//读取ECU的ID
{
	FILE *fp;
	fp=fopen("/etc/yuneng/ecuid.conf","r");
	fgets(ecuid,13,fp);
	fclose(fp);

	print2msg("ECU ID", ecuid);

	return strlen(ecuid);
}

int writeconnecttime(void)			//保存最后一次连接上服务器的时间，在home页面上会显示
{
	char connecttime[20]={'\0'};
	FILE *fp;
	
	getcurrenttime(connecttime);
	fp=fopen("/etc/yuneng/connect_time.conf","w");
	fprintf(fp,"%s",connecttime);
	fclose(fp);
	
	return 0;
}

int gettransflag(void)				//读取通信方式的配置文件，如果读到字符“1”，则使用GPRS通信，否则使用以太网通信
{
	int flag=0;
	FILE *fp;
	
	fp = fopen("/etc/yuneng/gprs.conf", "r");
	flag=fgetc(fp)-0x30;
	fclose(fp);
	
	return flag;
}

void clean_stopflag(void)			//用于半夜重启功能，清空标志，改为手动输入逆变器ID
{
	FILE *fp;

	fp = fopen("/etc/yuneng/stopflag.conf", "w");
	fputs("0", fp);
	fclose(fp);
}

int getplcautoflag(void)			//用于半夜重启功能，如果remonitor程序已经将stopflag.conf值置为1，则清零；否则不清
{
	FILE *fp;
	char flag = '0';

	fp = fopen("/etc/yuneng/stopflag.conf", "r");
	flag = fgetc(fp);
	fclose(fp);
	if('1' == flag){
		clean_stopflag();
		return 0;
	}
	
	fp = fopen("/etc/yuneng/autoflag.conf", "r");
	flag = fgetc(fp);
	fclose(fp);
	
	if('0' == flag)
		return 0;
	else
		return 1;
}

int writetime(char *time)			//paratime.conf文件保存逆变器最后一次上报数据的时间，实时数据页面会读取这时间
{
	FILE *fp;
	
	fp = fopen("/tmp/paratime.conf","w");
	fprintf(fp,"%s\n",time);
	fclose(fp);
	
	return 0;
}

int displaygfdi(struct inverter_info_t *firstinverter)		//显示gfdi，20120409
{
	FILE *fp;
	int i;

	struct inverter_info_t *inverter = firstinverter;
	fp = fopen("/tmp/gfdiresult.txt", "w");
	for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++){
		if('1' == inverter->flag){
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

int display_connection(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	int i;

	struct inverter_info_t *inverter = firstinverter;
	fp = fopen("/tmp/connectresult.txt", "w");
	for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++){
		if('1' == inverter->flag){
			if(1 == inverter->flagyc500){
				if('1' == inverter->status_web[12])
					fprintf(fp, "Turned Off\n");
				else
					fprintf(fp, "Turned On\n");
			}
			else{
				if(inverter->op <= 1)
					fprintf(fp, "Turned Off\n");
				else
					fprintf(fp, "Turned On\n");
			}
		}
		else
			fprintf(fp, "-\n");
		inverter++;
	}
	fclose(fp);
}

/*实时数据保存在/tmp/parameters.conf文件中；最近一次上传数据的时间保存在/tmp/lastreporttime.conf文件中，实时数据页面读取这些数据*/
int displayonweb(struct inverter_info_t *firstinverter, char *sendcommanddatatime)		//sendcommanddatatime：最近一次上传数据的时间
{
	int i;
	char datatime[20] = {'\0'};
	char temp[10] = {'\0'};
	FILE *fp;
	struct inverter_info_t *inverter = firstinverter;

	/*fp=fopen("/tmp/parameters.conf","w");
	for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++){
		if('1' == inverter->flag)
			fprintf(fp,"%d,%.1f,%d,%d\n",inverter->op,inverter->np,inverter->nv,inverter->it);
		else
			fprintf(fp,"-1,-1,-1,-1\n");
		inverter++;
	}
	fclose(fp);
    
	inverter = firstinverter;
	fp = fopen("/tmp/lastreporttime.conf", "w");
	for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++){
		if((inverter->lastreporttime[0]=='\0')&&(inverter->lastreporttime[1]=='\0'))
			fprintf(fp, "-\n");
		else{
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
			sprintf(temp, "%d", inverter->lastreporttime[0]);
			if(inverter->lastreporttime[0]<10){
				datatime[11] = '0';
				datatime[12] = temp[0];
			}
			else
    				strcat(datatime, temp);
			datatime[13] = ':';
			memset(temp, '\0', 10);
			sprintf(temp, "%d", inverter->lastreporttime[1]);
			if(inverter->lastreporttime[1]<10){
				datatime[14] = '0';
				datatime[15] = temp[0];
			}
    			else
				strcat(datatime, temp);
			datatime[16] = ':';
			datatime[17] = sendcommanddatatime[12];
			datatime[18] = sendcommanddatatime[13];

			fprintf(fp, "%s\n", datatime);
		}
		inverter++;
	}
	fclose(fp);*/

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
	for(i=0; (i<MAXINVERTERCOUNT) && (12 == strlen(inverter->inverterid)); i++){
		if(1 == inverter->flagyc500){
			if('1' == inverter->flag){
				fprintf(fp,"%s-A, %d, %.1f, %d, %d, %s\n", inverter->inverterid, inverter->op, inverter->np, inverter->nv, inverter->it, datatime);
				fprintf(fp,"%s-B, %d, %.1f, %d, %d, %s\n", inverter->inverterid, inverter->opb, inverter->np, inverter->nv, inverter->it, datatime);
			}
			else{
				fprintf(fp,"%s-A,  ,  ,  ,  , -\n", inverter->inverterid);
				fprintf(fp,"%s-B,  ,  ,  ,  , -\n", inverter->inverterid);
			}
		}
		else{
			if('1' == inverter->flag)
				fprintf(fp,"%s, %d, %.1f, %d, %d, %s\n", inverter->inverterid, inverter->op, inverter->np, inverter->nv, inverter->it, datatime);
			else
				fprintf(fp,"%s,  ,  ,  ,  , -\n", inverter->inverterid);
		}
		inverter++;
	}
	fclose(fp);

	display_connection(firstinverter);
	displaygfdi(firstinverter);

	return 0;
}

/*从逆变器读取到的配置参数保存在/tmp/presetdata.txt文件中，“参数配置”页面被点击时会读取此文件*/
int display_presetdata(struct inverter_info_t *inverter)
{
	int i;
	FILE *fp;
	
	fp = fopen("/etc/yuneng/presetdata.txt", "w");

	/*if(caltype){
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){
			if((-1 == inverter->prevl2) && (-1 == inverter->prevu2))
				fprintf(fp, "-, -\n");
			else
				fprintf(fp, "%d, %d\n", inverter->prevl2, inverter->prevu2);
			inverter++;
		}
	}
	else{*/
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){
			if((-1 == inverter->prevl2) && (-1 == inverter->prevu2))
				fprintf(fp, "-, -, -, -\n");
			else
				fprintf(fp, "%d, %d, %d, %d, %d\n", inverter->prevl2, inverter->prevu2, inverter->prefl, inverter->prefu, inverter->prert);
			inverter++;
		}
	//}

	fclose(fp);
	
	return 0;
}

/*域配置标志保存在/etc/yuneng/reconfigtn.conf文件中，如果为1，需要重配置逆变器（在页面上输入逆变器ID后触发）*/
int get_domainconfig_flag()
{
	FILE *fp;
	int flag;
	char tmp;
	
	fp = fopen("/etc/yuneng/reconfigtn.conf", "r");
	tmp  = fgetc(fp);
	if(tmp == '0')
		flag = 0;
	else
		flag = 1;
	fclose(fp);
	
	return flag;
}

/*读取版本号，判断是否为NA版*/
int get_version(void)		//判断是否为NA，如果是NA，改变电压值计算公式
{
	FILE *fp;
	char buff[50] = {'\0'};
//	char temp[50] = {'\0'};
	
	fp = fopen("/etc/yuneng/area.conf", "r");
	if(fp){
		fgets(buff, 50, fp);
		fclose(fp);
	}
	
	if('\n' == buff[strlen(buff)-1])
		buff[strlen(buff)-1] = '\0';
//	strcpy(temp, &buff[strlen(buff)-2]);
	if(0 == strncmp(buff, "MX", 2))
		return 2;
	else if((0 == strcmp(buff, "NA")) || (0 == strncmp(&buff[strlen(buff)-6], "NA-120", 6)))			//如果是NA返回1，否则返回0，NA-120和MX是同一个东西；
		return 1;
	else
		return 0;
}

int get_timezone(void)		//获取时间标志，改过时区为1， 没改为0
{
	FILE *fp;
	char zone[50] = {'\0'};

	fp = fopen("/etc/yuneng/timezone.conf", "r");
	fgets(zone, 50, fp);
	fclose(fp);

	if(!strncmp(zone, "Asia/Shanghai", 13))
		return 0;
	else
		return 1;
}

void turnoffall(void)
{
	FILE *fp;

	fp = fopen("/tmp/connect.conf", "w");
	fprintf(fp, "disconnect all");
	fclose(fp);
}

int write_gfdi_status(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	int i;
	struct inverter_info_t *inverter = firstinverter;
	char write_buff[65535] = {'\0'};

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
	{
		//if('0' != inverter->flag)
		//{
			strcat(write_buff, inverter->inverterid);
			write_buff[strlen(write_buff)] = inverter->last_gfdi_flag;
			strcat(write_buff, "END\n");
		//}
	}

	fp = fopen("/etc/yuneng/last_gfdi_status.txt", "w");
	if(fp)
	{
		fputs(write_buff, fp);
		fclose(fp);
	}

	return 0;
}

int write_turn_on_off_status(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	int i;
	struct inverter_info_t *inverter = firstinverter;
	char write_buff[65535] = {'\0'};

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
	{
		//if('0' != inverter->flag)
		//{
			strcat(write_buff, inverter->inverterid);
			write_buff[strlen(write_buff)] = inverter->last_turn_on_off_flag;
			strcat(write_buff, "END\n");
		//}
	}

	fp = fopen("/etc/yuneng/last_turn_on_off_status.txt", "w");
	if(fp)
	{
		fputs(write_buff, fp);
		fclose(fp);
	}

	return 0;
}

int read_gfdi_turn_on_off_status(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	int i;
	char buff[256];
	struct inverter_info_t *inverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
	{
		inverter->last_gfdi_flag = '0';
		inverter->last_turn_on_off_flag = '0';
	}

	fp = fopen("/etc/yuneng/last_gfdi_status.txt", "r");
	if(fp)
	{
		while(1)
		{
			memset(buff, '\0', sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if(0 == strlen(buff))
				break;
			else
			{
				inverter = firstinverter;
				for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
				{
					if(!strncmp(inverter->inverterid, buff, 12))
					{
						if('1' == buff[12])
							inverter->last_gfdi_flag = '1';
						else
							inverter->last_gfdi_flag = '0';
						break;
					}
				}
			}
		}
		fclose(fp);
	}
	else
	{
		inverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
		{
			inverter->last_gfdi_flag = '0';
		}
	}

	inverter = firstinverter;
	fp = fopen("/etc/yuneng/last_turn_on_off_status.txt", "r");
	if(fp)
	{
		while(1)
		{
			memset(buff, '\0', sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if(0 == strlen(buff))
				break;
			else
			{
				inverter = firstinverter;
				for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
				{
					if(!strncmp(inverter->inverterid, buff, 12))
					{
						if('1' == buff[12])
							inverter->last_turn_on_off_flag = '1';
						else
							inverter->last_turn_on_off_flag = '0';
						break;
					}
				}
			}
		}
		fclose(fp);
	}
	else
	{
		inverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
		{
			inverter->last_turn_on_off_flag = '0';
		}
	}

	return 0;
}

int show_data_on_lcd(int power, float energy, int count, int maxcount)
{
	FILE *fp;

	fp = fopen("/tmp/real_time_data.txt", "w");
	if(fp){
		fprintf(fp, "%d\n", power);
		fprintf(fp, "%f\n", energy);
		fprintf(fp, "%d\n", count);
		fprintf(fp, "%d\n", maxcount);
		fclose(fp);
	}
}
