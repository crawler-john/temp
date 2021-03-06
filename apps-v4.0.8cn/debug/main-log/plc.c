#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#include "sqlite3.h"
#include "variation.h"
#include "debug.h"
#include "database.h"
#include "fill_up_data.h"

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyO2"	//9G45上的UART1
#define _POSIX_SOURCE 1
#define MAXSIZE 56000
#define sendsize 800

extern int plcmodem;		//PLC的文件描述符
extern int caltype;		//计算方式，NA版和非NA版的区别
extern unsigned char ccuid[7];		//ECU3501的ID
extern sqlite3 *db;			//数据库
extern sqlite3 *tmpdb;
extern int afdflag;	//逆变器AFD对齐

extern int processpower(struct inverter_info_t *firstinverter);

int openplc(void)		//打开串口
{
	int fd, res, i=0;
	char buff[MAXSIZE]={'\0'};
	struct termios newtio;
	
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);		//打开串口
	if(fd<0){
		perror("MODEMDEVICE");
	}
	
	printmsg("Open plc modem successfully");
	
	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 51;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	
	return fd;
}

/*int config_domain(int plcmodem, char *ccuid, struct inverter_info_t *inverter)		//系统中的逆变器逐个配置
{
	int i;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){
		sendconfigdomaincmd(plcmodem, ccuid, inverter->tnuid);
		inverter++;
	}
	
	return 0;
}

int configtn(int modemfd, char *ccid, char *tnid)		//最多配置逆变器3遍
{
	int res=0, sendtimes=0;

	for(sendtimes=0; sendtimes<3; sendtimes++){
		//res = sendconfiginfo(modemfd, ccid, tnid);
		res = sendconfigdomaincmd(modemfd, ccid, tnid);
		if(1 == res)
			return 1;
	}
	
	return -1;
}*/

int configalltn(int modemfd, char *ccid, struct inverter_info_t *firstinverter)		//配置系统中所有的逆变器，先每个配置一遍，然后失败的逆变器单独每个最多配置3遍
{
	int i, times=0;
	struct inverter_info_t *inverter = firstinverter;
	
	for(i=0; i<MAXINVERTERCOUNT; i++){
		if(12 == strlen(inverter->inverterid)){
			if(0 == inverter->configflag){
				//if(1==configtn(modemfd, ccid, inverter->tnuid))
				if(1==sendconfigdomaincmd(modemfd, ccid, inverter->tnuid)){
					inverter->configflag=1;
					update_flag(db, inverter);
					print2msg(inverter->inverterid, "configed successfully");
				}
				else{
					print2msg(inverter->inverterid, "failed to config");
				}
			}
		}
		inverter++;
	}
	
	inverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++){
		if(12 == strlen(inverter->inverterid)){
			if(0 == inverter->configflag)
				for(times=0; times<3; times++)
					//if(1==configtn(modemfd, ccid, inverter->tnuid)){
					if(1==sendconfigdomaincmd(modemfd, ccid, inverter->tnuid)){
						inverter->configflag=1;
						update_flag(db, inverter);
						print2msg(inverter->inverterid, "configed successfully");
						break;
					}
		}
		inverter++;
	}
	
	return 0;
}

int save_gfdi_changed_result(struct inverter_info_t *firstinverter)
{
	struct inverter_info_t *inverter = firstinverter;
	int i;
	char gfdi_changed_result[65535]={'\0'};

	memset(gfdi_changed_result, '\0', sizeof(gfdi_changed_result));
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
	{
		if(1 == inverter->gfdi_changed_flag)
		{
			sprintf(gfdi_changed_result, "%s%cEND", inverter->inverterid, inverter->last_gfdi_flag);
			save_inverter_parameters_result(inverter, 115, gfdi_changed_result);
		}
	}

	return 0;
}

int save_turn_on_off_changed_result(struct inverter_info_t *firstinverter)
{
	struct inverter_info_t *inverter = firstinverter;
	int i;
	char turn_on_off_changed_result[65535]={'\0'};

	memset(turn_on_off_changed_result, '\0', sizeof(turn_on_off_changed_result));
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
	{
		if(1 == inverter->turn_on_off_changed_flag)
		{
			sprintf(turn_on_off_changed_result, "%s%cEND", inverter->inverterid, inverter->last_turn_on_off_flag);
			save_inverter_parameters_result(inverter, 116, turn_on_off_changed_result);
		}
	}

	return 0;
}

int process_gfdi(struct inverter_info_t *firstinverter)
{
	int i;
	FILE *fp;
	char command[256] = {'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	fp = fopen("/tmp/process_gfdi.conf", "r");
	while(1){
		curinverter = firstinverter;
		memset(command, '\0', 256);
		fgets(command, 256, fp);
		if(!strlen(command))
			break;
		if('\n' == command[strlen(command)-1])
			command[strlen(command)-1] = '\0';
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){
			if(!strcmp(command, curinverter->inverterid)){
				send_clean_gfdi(curinverter);
				print2msg(curinverter->inverterid, "Clear GFDI");
			}
			curinverter++;
		}
	}

	fclose(fp);
	fp = fopen("/tmp/process_gfdi.conf", "w");
	fclose(fp);
}

int process_presetdata(sqlite3 *db, struct inverter_info_t *firstinverter)
{
	FILE *fp;
	int flag = 0;
	char buff[256] = {'\0'};
	char presetdata[20]={0};		//保存ECU发送给逆变器的预设值（在参数配置页面上输入的参数经转换）

	fp = fopen("/tmp/presetdata.conf", "r");
	fgets(buff, 255, fp);
	fclose(fp);

	if(!strlen(buff))
		flag = 0;
	if('0' == buff[0])
		flag = 0;
	if('1' == buff[0])
		flag = 1;
	if('2' == buff[0])
		flag = 2;

	if(1 == flag){
		getpresetdata(db, presetdata, caltype);
		sendpresetdatatoallcmd(plcmodem, ccuid, presetdata);
		sleep(10);
		readpresetdata(plcmodem, ccuid, firstinverter, presetdata, caltype);

		fp = fopen("/tmp/presetdata.conf", "w");
		fprintf(fp, "0");
		fclose(fp);
	}
	if(2 == flag)
	{
		read_protect_parameters(plcmodem, ccuid, firstinverter, caltype);

		fp = fopen("/tmp/presetdata.conf", "w");
		fprintf(fp, "0");
		fclose(fp);
	}

	return 0;
}

int process_connect(struct inverter_info_t *firstinverter)
{
	int i, j;
	FILE *fp;
	char command[256] = {'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	fp = fopen("/tmp/connect.conf", "r");
	if(fp)
	{
		while(1){
			curinverter = firstinverter;
			memset(command, '\0', 256);
			fgets(command, 256, fp);
			if(!strlen(command))
				break;
			if('\n' == command[strlen(command)-1])
				command[strlen(command)-1] = '\0';
			if(!strncmp(command, "connect all", 11)){
				wakeup_broadcast();
				break;
			}
			if(!strncmp(command, "disconnect all", 14)){
				shutdown_broadcast();
				break;
			}

			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){
				if(!strncmp(command, curinverter->inverterid, 12)){
					j = 0;
					if('c' == command[12]){
						while(j<3){
							if(1 == wakeup(curinverter))
								break;
							j++;
						}
					}
					if('d' == command[12]){
						while(j<3){
							if(1 == shutdown(curinverter))
								break;
							j++;
						}
					}
				}
				curinverter++;
			}
		}

		fclose(fp);
	}

	fp = fopen("/tmp/connect.conf", "w");
	fclose(fp);
}

int process_afd(void)
{
	if(1 == afdflag){
		afd_broadcast();
		afdflag = 0;
	}
}

int process_all(struct inverter_info_t *firstinverter)
{
	int i;


	//process_gfdi(firstinverter);			//清GFDI标志
	//process_presetdata(db, firstinverter);		//设置预设值
	//process_connect(firstinverter);			//开关机
	//process_afd();
}

int getalldata(struct inverter_info_t *firstinverter, char *time, int time_linux)		//获取每个逆变器的数据
{
	int i, j, currentcount = 0;
	struct inverter_info_t *curinverter = firstinverter;

	sendbroadcastask(plcmodem, ccuid, time);
	sleep(5);
	calibration_time_broadcast(firstinverter, time_linux);			//
	sleep(10);

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){		//每个逆变器要一次数据
		process_all(firstinverter);
		curinverter->curflag = '0';
		if(0 == curinverter->configflag){
			if(1==sendconfigdomaincmd(plcmodem, ccuid, curinverter->tnuid)){
				curinverter->configflag=1;
				update_flag(db, curinverter);
			}
		}
		sendaskcmd(curinverter, time);
		curinverter++;
	}

	for(j=0; j<7; j++){
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){		//每个逆变器最多要5次数据
			process_all(firstinverter);
			if('0' == curinverter->curflag)
				sendaskcmd(curinverter, time);
			curinverter++;
		}
	}

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){		//清标志
		if('0' == curinverter->curflag)
			curinverter->flag = '0';
		curinverter++;
	}

	//curinverter = firstinverter;
	//lenddata(curinverter);

	curinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++){							//统计当前多少个逆变器
		if((curinverter->flag=='1')&&(12==strlen(curinverter->inverterid)))
			currentcount++;
		curinverter++;
	}
	
	write_gfdi_status(firstinverter);
	write_turn_on_off_status(firstinverter);
//	save_turn_on_off_changed_result(firstinverter);
//	save_gfdi_changed_result(firstinverter);

	update_tmpdb(tmpdb, firstinverter);

	save_time_to_database(firstinverter,time_linux);

	return currentcount;
}

int check_in800(struct inverter_info_t *inverter)		//有一批生产的800个逆变器程序上存在问题，ECU判断逆变器是否属于这800个，如果是则返回1，否则返回-1
{
	if(((strcmp(inverter->inverterid, "102000000001")>=0)&&(strcmp(inverter->inverterid, "102000000300")<=0))||((strcmp(inverter->inverterid, "104000000001")>=0)&&(strcmp(inverter->inverterid, "104000000500")<=0)))
		return 1;
	else
		return -1;
}

/*int resolvedata(char *inverter_data, struct inverter_info_t *inverter, int type)		//解析数据及状态
{
	unsigned char data[20];
	int i;
	//inverter->flag = '1';
	int seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*825/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*275/4096;
	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;

	if(1 == check_in800(inverter)){					//800个逆变器程序问题，使用以下计算方法
		inverter->op=(int)(inverter->dv*inverter->di*1.05/100.0);
		inverter->thistime = time(NULL);
		inverter->curgeneration = (float)inverter->op * (float)(inverter->thistime - inverter->lasttime)/ 3600.0 /1000.0;
		inverter->lasttime = inverter->thistime;
	}
	else{
		for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
		}
		seconds = inverter_data[18]*256 + inverter_data[19];
		data[1] &= 0x0f;
		inverter->op = (int)((data[1]*256 + data[2])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/seconds);
		inverter->thistime = time(NULL);
		inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
		inverter->lasttime = inverter->thistime;
	}
	if(1 == type)
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;
	
	inverter->status[0]=((inverter_data[10]>>7)&0x01)+0x30;
	inverter->status[1]=((inverter_data[10]>>6)&0x01)+0x30;
	inverter->status[2]=((inverter_data[10]>>5)&0x01)+0x30;
	inverter->status[3]=((inverter_data[10]>>4)&0x01)+0x30;
	inverter->status[4]=((inverter_data[10]>>3)&0x01)+0x30;
	inverter->status[5]=((inverter_data[10]>>2)&0x01)+0x30;
	inverter->status[6]=((inverter_data[10]>>1)&0x01)+0x30;
	inverter->status[7]=(inverter_data[10]&0x01)+0x30;
	inverter->status[8]=((inverter_data[11]>>7)&0x01)+0x30;
	inverter->status[9]=((inverter_data[11]>>6)&0x01)+0x30;
	inverter->status[10]=((inverter_data[11]>>5)&0x01)+0x30;
	
	inverter->lastreporttime[0] = inverter_data[20];
	inverter->lastreporttime[1] = inverter_data[21];
	
	return 1;
}*/

/*int resolvedata_B(char *inverter_data, struct inverter_info_t *inverter, int type)		//解析数据及状态
{
	unsigned char data[20];
	int i;
	//inverter->flag = '1';
	int seconds;

	inverter->dvb=(inverter_data[0]*256+inverter_data[1])*825/4096;
	inverter->dib=(inverter_data[2]*256+inverter_data[3])*275/4096;
	inverter->npb=(inverter_data[4]*256+inverter_data[5])/10.0;

	/*if(1 == check_in800(inverter)){					//800个逆变器程序问题，使用以下计算方法
		inverter->op=(int)(inverter->dv*inverter->di*1.05/100.0);
		inverter->thistime = time(NULL);
		inverter->curgeneration = (float)inverter->op * (float)(inverter->thistime - inverter->lasttime)/ 3600.0 /1000.0;
		inverter->lasttime = inverter->thistime;
	}*/
	//else{
		/*for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
		}
		seconds = inverter_data[18]*256 + inverter_data[19];
		data[1] &= 0x0f;
		inverter->opb = (int)((data[1]*256 + data[2])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/seconds);
		inverter->thistimeb = time(NULL);
		inverter->curgenerationb = (float)(inverter->opb) * (float)(inverter->thistimeb - inverter->lasttimeb) / 3600.0 /1000.0;
		inverter->lasttimeb = inverter->thistimeb;
	//}
	if(1 == type)
		inverter->nvb=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nvb=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->itb=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;
	
	inverter->statusb[0]=((inverter_data[10]>>7)&0x01)+0x30;
	inverter->statusb[1]=((inverter_data[10]>>6)&0x01)+0x30;
	inverter->statusb[2]=((inverter_data[10]>>5)&0x01)+0x30;
	inverter->statusb[3]=((inverter_data[10]>>4)&0x01)+0x30;
	inverter->statusb[4]=((inverter_data[10]>>3)&0x01)+0x30;
	inverter->statusb[5]=((inverter_data[10]>>2)&0x01)+0x30;
	inverter->statusb[6]=((inverter_data[10]>>1)&0x01)+0x30;
	inverter->statusb[7]=(inverter_data[10]&0x01)+0x30;
	inverter->statusb[8]=((inverter_data[11]>>7)&0x01)+0x30;
	inverter->statusb[9]=((inverter_data[11]>>6)&0x01)+0x30;
	inverter->statusb[10]=((inverter_data[11]>>5)&0x01)+0x30;
	
	inverter->lastreporttimeb[0] = inverter_data[20];
	inverter->lastreporttimeb[1] = inverter_data[21];
	
	return 1;
}*/

int transtotnuid(struct inverter_info_t *inverter)			//把逆变器的ID号转换成逆变器3501的ID
{
	int i, j;
	
	for(i=0; i<MAXINVERTERCOUNT; i++){
		if(12 == strlen(inverter->inverterid)){
			for(j=0; j<6; j++)
				inverter->tnuid[j] = ((inverter->inverterid[2*j]-0x30)<<4) + (inverter->inverterid[2*j+1]-0x30);
			printhexmsg("tnuid", inverter->tnuid, 6);
		}
		inverter++;
	}

	return 0;
}

int shaixuan(struct inverter_info_t *firstinverter)		//自动上报功能中使用此函数，自动上报过程中，有些逆变器会上报好几次ID，ECU在所有逆变器上报完成后，调用此函数，把重复的ID删除
{
	int i, j;
	struct inverter_info_t *inverter = firstinverter;
	
	for(i=0; i<MAXINVERTERCOUNT; i++){			//先把每个数组中的ID比较，如果有重复的ID，则删除掉，只剩下第一个
		if(12 == strlen(inverter[i].inverterid)){
			for(j=i+1; j<MAXINVERTERCOUNT; j++){
				if(!strcmp(inverter[i].inverterid, inverter[j].inverterid)){
					memset(inverter[j].inverterid, '\0', 12);
					memset(inverter[j].tnuid, '\0', 6);
				}
			}
		}
	}
	
	inverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++){			//在上面的循环结束后，有些ID数组已经空出来，ID号之间可能会空，把ID号全部往前移动，把ID号之间的空白去除
		if(12 != strlen(inverter[i].inverterid)){
			for(j=i+1; j<MAXINVERTERCOUNT; j++){
				if(12 == strlen(inverter[j].inverterid)){
					inverter[i].inverterid[0] = inverter[j].inverterid[0];
					inverter[i].inverterid[1] = inverter[j].inverterid[1];
					inverter[i].inverterid[2] = inverter[j].inverterid[2];
					inverter[i].inverterid[3] = inverter[j].inverterid[3];
					inverter[i].inverterid[4] = inverter[j].inverterid[4];
					inverter[i].inverterid[5] = inverter[j].inverterid[5];
					inverter[i].inverterid[6] = inverter[j].inverterid[6];
					inverter[i].inverterid[7] = inverter[j].inverterid[7];
					inverter[i].inverterid[8] = inverter[j].inverterid[8];
					inverter[i].inverterid[9] = inverter[j].inverterid[9];
					inverter[i].inverterid[10] = inverter[j].inverterid[10];
					inverter[i].inverterid[11] = inverter[j].inverterid[11];
					
					inverter[i].tnuid[0] = inverter[j].tnuid[0];
					inverter[i].tnuid[1] = inverter[j].tnuid[1];
					inverter[i].tnuid[2] = inverter[j].tnuid[2];
					inverter[i].tnuid[3] = inverter[j].tnuid[3];
					inverter[i].tnuid[4] = inverter[j].tnuid[4];
					inverter[i].tnuid[5] = inverter[j].tnuid[5];
					
					memset(inverter[j].inverterid, '\0', 12);
					memset(inverter[j].tnuid, '\0', 6);
					
					break;
				}
			}
		}
	}
}

int transtoinverterid(struct inverter_info_t *inverter)		//把逆变器3501的UID转换成逆变器的ID
{
	int i=0, j=0;
	for(j=0; j<MAXINVERTERCOUNT; j++){
		if((0 != inverter->tnuid[0])||
			(0 != inverter->tnuid[1])||
			(0 != inverter->tnuid[2])||
			(0 != inverter->tnuid[3])||
			(0 != inverter->tnuid[4])||
			(0 != inverter->tnuid[5])){
			for(i=0; i<6; i++){
				inverter->inverterid[2*i] = (inverter->tnuid[i]>>4) + 0x30;
				inverter->inverterid[2*i+1] = (inverter->tnuid[i]&0x0f) + 0x30;
			}
		}
		inverter++;
	}
	
	return 0;
}

int manualsetid(int modemfd, unsigned char *ccuid, struct inverter_info_t *firstinverter, sqlite3 *db)		//手动输入ID时作用
{
	int maxcount=0;
	char flag='0';									//判断是否需要重新配置，每次手动输入ID后，都需要重新配置
	FILE *fp;
	
	do{										//从数据库中读取输入的ID，如果没独到，等10秒钟后再读，知道读取到有ID为止
		maxcount = gettnuid(db, firstinverter);
		if(0==maxcount){
			display_input_id();
			sleep(5);
		}
	}while(0==maxcount);
	transtotnuid(firstinverter);							//把逆变器的ID号转换为逆变器3501的UID
	
	/*fp = fopen("/etc/yuneng/reconfigtn.conf", "r");					//读取标志，如果为‘1’，则配置逆变器
	flag = fgetc(fp);
	fclose(fp);
	
	if('1' == flag){
		//sendgridcmd(modemfd, ccuid);
		sleep(1);*/
		configalltn(modemfd, ccuid, firstinverter);
		//config_domain(modemfd, ccuid, firstinverter);
		/*fp = fopen("/etc/yuneng/reconfigtn.conf", "w");
		fputc('0', fp);
		fclose(fp);
	}*/
	
	return maxcount;								//返回逆变器数
}

int autorptid(int modemfd, unsigned char *ccuid, struct inverter_info_t *firstinverter, sqlite3 *db)		//自动上报逆变器ID时的操作
{
	int count=0, i, j;
	int res=0;
	struct inverter_info_t *inverter = firstinverter;
	struct inverter_info_t *tempinverter = firstinverter;
	struct termios newtio;
	//while(-1 == sendgridcmd(modemfd, ccuid));
	
	//for(j=0; (j<3)&&(count<1); j++){										//最多自动上报3次，如果还是没有逆变器上报ID，则进入下一个步骤
		//count = sendrptidcmd(modemfd, ccuid, inverter);

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 1;
	tcflush(modemfd, TCIFLUSH);
	tcsetattr(modemfd, TCSANOW, &newtio);

	//sendgridcmd(modemfd, ccuid);									//发送一个组网命令（广播）
	//sleep(10);
	//send_allowed_report_cmd(modemfd, ccuid);							//发送一个允许上报ID的广播命令
	send_0_domain_cmd(modemfd, ccuid);								//发送一个0域名广播命令
	sleep(10);
	for(i=0; i<3; i++){										//让逆变器上报3次
		//send_allowed_report_cmd(modemfd, ccuid);								//发送一个上报的ID的命令
		sendrptidcmd(modemfd, ccuid);
	
		while(1){										//不停地读取逆变器3501的UID
			res = read_tnid(modemfd, ccuid, inverter);
			if(1 == res)
				inverter++;
			else
				break;
		}
		
		for(j=0; j<MAXINVERTERCOUNT; j++){							//禁止已经上报ID的逆变器再上报ID
			if((0x00!=tempinverter->tnuid[0])||(0x00!=tempinverter->tnuid[1])||(0x00!=tempinverter->tnuid[2])||
				(0x00!=tempinverter->tnuid[3])||(0x00!=tempinverter->tnuid[4])||(0x00!=tempinverter->tnuid[5]))
				send_stop_report_cmmd(modemfd, ccuid, tempinverter->tnuid);		//发送禁止逆变器上报ID的命令
			tempinverter++;
		}
		tempinverter = inverter;
	}
	
	//send_allowed_report_cmd(modemfd, ccuid);							//等3次ID号上报完后，再发送一个广播命令开启所有逆变器的上报ID功能
	sleep(10);
	transtoinverterid(firstinverter);								//把逆变器3501的UID转换成逆变器的ID
	shaixuan(firstinverter);									//筛选
	insertid( db, firstinverter);									//把逆变器的ID插入到数据库中
	configalltn(modemfd, ccuid, firstinverter);							//配置所有的

	tempinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++){								//统计手动输入的ID数量
		if(12 == strlen(tempinverter->inverterid))
			count++;
		tempinverter++;
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 51;
	tcflush(modemfd, TCIFLUSH);
	tcsetattr(modemfd, TCSANOW, &newtio);
	
	return count;
}


int turnoffautorptid(int modemfd, unsigned char *ccuid, struct inverter_info_t *firstinverter)		//发送禁止逆变器上报ID的命令,ZK
{
	int count=0, i, j;
	int res=0;
	struct inverter_info_t *curinverter = firstinverter;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++,curinverter++){
		if((curinverter->turnoffautorpidflag != 1) && (curinverter->curacctime > 600))
		{
			for(j=0; j<3; j++)
			{
				if(1 == send_stop_report_cmmd(modemfd, ccuid, curinverter->tnuid))
				{
					curinverter->turnoffautorpidflag = 1;
					break;
				}
			}
		}
		//update_turnoff_rpt_id_flag(db,curinverter);
		printdecmsg(curinverter->inverterid, curinverter->turnoffautorpidflag);
	}
}

int turnonautorptid(int modemfd, unsigned char *ccuid, struct inverter_info_t *firstinverter,sqlite3 *db)//发送打开逆变器上报ID的命令，ZK
{
	int maxcount=0;
	int count=0, i, j;
	int res=0;
	struct inverter_info_t *curinverter = firstinverter;

	printmsg("Turn on report ID");

	getinverterid(db, firstinverter);
	transtotnuid(firstinverter);							//把逆变器的ID号转换为逆变器3501的UID

	printmsg("turnoffautorpidflag");
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++,curinverter++){
		for(j=0; j<3; j++)
		{
			if(1 == send_allow_report_cmmd(modemfd, ccuid, curinverter->tnuid))	//返回成功才把标志置为0，ZK
			{
				curinverter->turnoffautorpidflag = 0;
				break;
			}
		}
		printdecmsg(curinverter->inverterid, curinverter->turnoffautorpidflag);
	}

	curinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++,curinverter++){
		memset(curinverter->inverterid, '\0', INVERTERIDLEN);		//清空逆变器ID
		memset(curinverter->tnuid, '\0', TNUIDLENGTH);			//清空逆变器3501UID		
	}
}
/*int setallparam(int modemfd, char *ccid, char *tnid, char *data)
{
	int res, i, j, sendtimes, flag=0;
	char readbuff[512]={'\0'};
	char validdata[10]={'\0'};
	fd_set rd;
	struct timeval timeout;
	
	
	
	return flag;
}*/

int setautonet(int modemfd, char *ccid)			//设置自动组网
{
	int res, i, j, sendtimes, flag=0;
	char readbuff[512]={'\0'};
	fd_set rd;
	struct timeval timeout;
	
	for(sendtimes=0; sendtimes<3; sendtimes++){
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		
		//sendnetcmd(modemfd, ccid);

		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printdecmsg("Send LED command again", sendtimes+1);
			continue;
		}
		
		if(FD_ISSET(modemfd, &rd)){
			read(modemfd, readbuff, 255);
			if(strlen(readbuff)>0)
				return 1;
		}
	}
	
	return flag;
}



/*int sendpresetdatatoall(int modemfd, char *ccid, unsigned char *data)
{
	int presetflag=0;
	
	presetflag = getpresetflag();
	if(1 == presetflag)
		sendpresetdatatoallcmd(modemfd, ccid, data);
	
	return 0;
}*/

/*int autorptid(int modemfd, char *ccid, struct inverter_info_t *inverter)
{
	int ret=0, res, count=0, i, j;
	char readbuff[512]={'\0'};
	fd_set rd;
	struct timeval timeout;
	
	do{
		ret = setautonet(modemfd, ccid);
#ifdef DEBUFINFO
		printf("Send auto command!\n");
#endif
	}while(0==ret);
	//sendrptcmd(modemfd, ccid);
	
#ifdef DEBUFINFO
	printf("--------------\n");
#endif
	
	while(1){
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 50;
		timeout.tv_usec = 0;

		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
#ifdef DEBUGINFO
			printf("None inverter report ID\n");
#endif
			break;
		}
		if(FD_ISSET(modemfd, &rd)){
			read(modemfd, readbuff, 255);
			for(i=12, j=0; i<18; i++, j++)
				inverter->tnuid[j] = readbuff[i];
			transid(inverter);
			count++;
			inverter++;
			memset(readbuff, '\0', 50);
		}
	}
	
#ifdef DEBUGINFO
	printf("--------------\n");
#endif
	
	return count;
}*/

int comparepresetdata(char *presetdata, char *temppresetdata)				//比较逆变器返回的预设值和页面上输入的预设值
{
	int i;

	if(caltype){
		for(i=4; i<8; i++)
			if(presetdata[i] != temppresetdata[i])
				return -1;
	}
	else{
		for(i=4; i<10; i++)
			if(presetdata[i] != temppresetdata[i])
				return -1;
	}
	
	return 0;
}

int resolve_presetdata(struct inverter_info_t *inverter, char * presetdata, int type)	//解析预设值
{
	float temp;
	
	if(2 == type)
		temp = (presetdata[0]*256 + presetdata[1])/2.93;
	else if(1 == type)
		temp = (presetdata[0]*256 + presetdata[1])/2.90345;
	else
		temp = (presetdata[0]*256 + presetdata[1])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->prevl1 = (int)temp +1;
	else
		inverter->prevl1 = (int)temp;
	
	if(2 == type)
		temp = (presetdata[2]*256 + presetdata[3])/2.93;
	else if(1 == type)
		temp = (presetdata[2]*256 + presetdata[3])/2.90345;
	else
		temp = (presetdata[2]*256 + presetdata[3])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->prevu1 = (int)temp +1;
	else
		inverter->prevu1 = (int)temp;
	
	if(2 == type)
		temp = (presetdata[4]*256 + presetdata[5])/2.93;
	else if(1 == type)
		temp = (presetdata[4]*256 + presetdata[5])/2.90345;
	else
		temp = (presetdata[4]*256 + presetdata[5])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->prevl2 = (int)temp +1;
	else
		inverter->prevl2 = (int)temp;
	
	if(2 == type)
		temp = (presetdata[6]*256 + presetdata[7])/2.93;
	else if(1 == type)
		temp = (presetdata[6]*256 + presetdata[7])/2.90345;
	else
		temp = (presetdata[6]*256 + presetdata[7])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->prevu2 = (int)temp +1;
	else
		inverter->prevu2 = (int)temp;
	
	if(type)
		inverter->prefl = 600-presetdata[8];
	else
		inverter->prefl = 500-presetdata[8];
	if(type)
		inverter->prefu = 600+presetdata[9];
	else
		inverter->prefu = 500+presetdata[9];
	inverter->prert = presetdata[10]*256 + presetdata[11];
}

int set_no_presetdata(struct inverter_info_t *inverter)
{
	inverter->prevl1 = -1;
	inverter->prevu1 = -1;
	inverter->prevl2 = -1;
	inverter->prevu2 = -1;
	inverter->prefl = -1;
	inverter->prefu = -1;
	inverter->prert = -1;
}

int save_protect_result(struct inverter_info_t *firstinverter)
{
	struct inverter_info_t *inverter = firstinverter;
	int i, count=0;
	char protect_result[65535] = {'\0'};
	char inverter_result[64];
	char ecu_id[16];
	FILE *fp;
	//int max_voltage, min_voltage, max_frequency, min_frequency, boot_time;

	strcpy(protect_result, "APS13AAAAAA114AAA1");

	fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	strcat(protect_result, ecu_id);					//ECU的ID
	strcat(protect_result, "0000");					//逆变器个数
	strcat(protect_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(protect_result, "END");					//固定格式

	//get_protect_parameters(&max_voltage, &min_voltage, &max_frequency, &min_frequency, &boot_time);

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
	{
		if((-1 != inverter->prevl2) || (-1 != inverter->prevu2))
		{
			memset(inverter_result, '\0', sizeof(inverter_result));
			sprintf(inverter_result, "%s%03d%03d%03d%03d%05d", inverter->inverterid, inverter->prevl2, inverter->prevu2, inverter->prefl, inverter->prefu, inverter->prert);
			if(2 == caltype)
				strcat(inverter_result, "082118124155551600600649");
			else if(1 == caltype)
				strcat(inverter_result, "181239221298551600600649");
			else
				strcat(inverter_result, "149217221278451500500549");
			strcat(protect_result, inverter_result);
			strcat(protect_result, "END");
			count++;
		}
	}

	if(count>9999)
		count = 9999;

	protect_result[30] = count/1000 + 0x30;
	protect_result[31] = (count/100)%10 + 0x30;
	protect_result[32] = (count/10)%10 + 0x30;
	protect_result[33] = count%10 + 0x30;

	if(strlen(protect_result) > 10000)
		protect_result[5] = strlen(protect_result)/10000 + 0x30;
	if(strlen(protect_result) > 1000)
		protect_result[6] = (strlen(protect_result)/1000)%10 + 0x30;
	if(strlen(protect_result) > 100)
		protect_result[7] = (strlen(protect_result)/100)%10 + 0x30;
	if(strlen(protect_result) > 10)
		protect_result[8] = (strlen(protect_result)/10)%10 + 0x30;
	if(strlen(protect_result) > 0)
		protect_result[9] = strlen(protect_result)%10 + 0x30;

	strcat(protect_result, "\n");

	save_process_result(114, protect_result);

	return 0;
}

int readpresetdata(int modemfd, char *ccuid, struct inverter_info_t *firstinverter, char *presetdata, int type)		//读取逆变器的预设值
{
	int i, res, sendtimes;		//发送次数
	char readpresetdata[20]={'\0'};
	struct inverter_info_t *curinverter = firstinverter;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){
		res = askpresetdatacmd(curinverter, readpresetdata, 0x11);			//读取失败标志为‘B’
		if(-1 == res){
			set_no_presetdata(curinverter);
			curinverter->presetdataflag = 'B';
		}
		else if(0 == comparepresetdata(presetdata, readpresetdata)){						//读取成功，且与页面上输入的值相等
			resolve_presetdata(curinverter, readpresetdata, type);
			curinverter->presetdataflag = '0';
		}
		else{
			resolve_presetdata(curinverter, readpresetdata, type);
			curinverter->presetdataflag = 'A';								//读取成功，且与页面上输入的值不等
		}
		curinverter++;
	}
	
	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){					//如果标志为'A'，再最多设置3遍
		if(('A'==curinverter->presetdataflag))
			for(sendtimes=0; sendtimes<3; sendtimes++){
				if(1 == sendpresetdata(modemfd, ccuid, curinverter->tnuid, presetdata))
					break;
				else
					;
			}
		curinverter++;
	}
	
	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){					//把上面读取到‘A’逆变器再最多读取3遍
		if('A'==curinverter->presetdataflag)
			for(sendtimes=0; sendtimes<3; sendtimes++){
				res = askpresetdatacmd(curinverter, readpresetdata, 0x13);
				if(1 == res){
					resolve_presetdata(curinverter, readpresetdata, type);
					if(0 == comparepresetdata(presetdata, readpresetdata)){
						curinverter->presetdataflag = '0';
						break;
					}
					else
						curinverter->presetdataflag = 'A';
				}
				else{
					set_no_presetdata(curinverter);
					curinverter->presetdataflag = 'B';
				}
			}
		curinverter++;
	}
	
	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){					//把上面读取到‘B’逆变器再多读3遍
		if('B'==curinverter->presetdataflag)
			for(sendtimes=0; sendtimes<3; sendtimes++){
				res = askpresetdatacmd(curinverter, readpresetdata, 0x12);
				if(-1 == res){
					set_no_presetdata(curinverter);
					curinverter->presetdataflag = 'B';
				}
				else if(0 == comparepresetdata(presetdata, readpresetdata)){
					resolve_presetdata(curinverter, readpresetdata, type);
					curinverter->presetdataflag = '0';
					break;
				}
				else{
					resolve_presetdata(curinverter, readpresetdata, type);
					curinverter->presetdataflag = 'A';
				}
			}
		curinverter++;
	}
	display_presetdata(firstinverter);		//在页面上显示AB类
	save_protect_result(firstinverter);
	
	return 0;
}

int read_protect_parameters(int modemfd, char *ccuid, struct inverter_info_t *firstinverter, int type)		//读取逆变器的预设值
{
	int i, j, res;		//发送次数
	char readpresetdata[20]={'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
	{
		for(j=0; j<3; j++)
		{
			res = askpresetdatacmd(curinverter, readpresetdata, 0x11);			//读取失败标志为‘B’
			if(-1 == res){
				set_no_presetdata(curinverter);
			}
			else						//读取成功，且与页面上输入的值相等
			{
				resolve_presetdata(curinverter, readpresetdata, type);
				break;
			}
		}
	}

	display_presetdata(firstinverter);		//在页面上显示AB类
	save_protect_result(firstinverter);

	return 0;
}

int open_reset(void)
{
    int fd_reset;

    fd_reset=open("/dev/reset",O_RDONLY);
    ioctl(fd_reset,0,NULL);	// 2.GPIO reset

    printmsg("Reset has been opened");

    return fd_reset;
}

void close_reset(int fd_reset)
{
    close(fd_reset);

    printmsg("Reset has been closed");
}

int closeplc(int fd)					//关闭串口
{
	close(fd);
	
	return 0;
}
