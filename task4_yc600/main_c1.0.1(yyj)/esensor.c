#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include "variation.h"
#include "debug.h"
#include "sqlite3.h"
#include "ema_control.h"
#include <string.h>
#include "esensor.h"

#define BAUDRATE B57600
#define MODEMDEVICE "/dev/ttyO2"

extern ecu_info ecu;
extern sqlite3 *db;
int zbmodem;				//zigbee串口

extern int clear_zbmodem(void);


int init_esensor(esensor_info *esensor)
{
	int i;
	char flag_autorpt = '0';				//自动上报标志
	char flag_limitedid = '0';				//限定ID标志

	FILE *fp;
	esensor_info *curesensor = esensor;


	for(i=0; i<MAXINVERTERCOUNT; i++, curesensor++)
	{
		memset(curesensor->id, '\0', sizeof(curesensor->id));		//清空逆变器3501UID
		memset(curesensor->tnuid, '\0', sizeof(curesensor->tnuid));			//清空逆变器ID

		curesensor->model = 0;

		curesensor->dv=0;
		curesensor->di=0;
		curesensor->op=0;
		curesensor->curgeneration=0;
		curesensor->max_dv=0;
		curesensor->max_di=0;
		curesensor->curacctime=0;


		curesensor->dataflag = 0;		//上一轮有数据的标志置位
	//	curinverter->bindflag=0;		//绑定逆变器标志位置清0
		curesensor->no_getdata_num=0;	//ZK,清空连续获取不到数据的次数
		curesensor->disconnect_times=0;		//没有与逆变器通信上的次数清0， ZK
		curesensor->signalstrength=0;			//信号强度初始化为0
	}


	return 1;
}


int zb_send_esensor_cmd(esensor_info *esensor, char *buff, int length)		//zigbee eSensor包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = esensor->shortaddr>>8;
	sendbuff[6]  = esensor->shortaddr;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}

	usleep(50000);
	if(0!=esensor->shortaddr)
	{
		write(zbmodem, sendbuff, length+15);
		return 1;
	}
	else
		return -1;
}

int zb_get_esensor_reply(char *data,esensor_info *esensor)			//读取esensor的返回帧
{
	int i;
	char data_all[256];
	char esensorid[13] = {'\0'};
	int ret, temp_size,size;
	fd_set rd;
	struct timeval timeout;
	
	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	
	if(select(zbmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get esensor reply time out");
		esensor->signalstrength=0;
		return -1;
	}
	else
	{
		temp_size = read(zbmodem, data_all, 255);
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply_esensor", data_all, temp_size);
		sprintf(esensorid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==esensor->shortaddr/256)&&(data_all[3]==esensor->shortaddr%256)&&(data_all[5]==0xA5)&&(0==strcmp(esensor->id,esensorid)))
		{
			esensor->signalstrength=data_all[4];
			return size;
		}
		else
		{
			esensor->signalstrength=0;
			return -1;
		}
	}
}

int resolvedata_esensor(char *data, esensor_info *esensor)
{

	esensor->dv = (data[0] * 16777216 + data[1] * 65536 + data[2] * 256 + data[3]) / 10;
	esensor->di = (data[4] * 16777216 + data[5] * 65536 + data[6] * 256 + data[7]) / 1000;
	esensor->op = (data[8] * 16777216 + data[9] * 65536 + data[10] * 256 + data[11]) / 10;
	esensor->curgeneration = (data[12] * 16777216 + data[13] * 65536 + data[14] * 256 + data[15]) / 1000;
	esensor->energy = (data[16] * 16777216 + data[17] * 65536 + data[18] * 256 + data[19]) / 1000;
	esensor->max_dv = (data[20] * 16777216 + data[21] * 65536 + data[22] * 256 + data[23]) / 10;
	esensor->curacctime = ((data[24] * 16777216 + data[25] * 65536 + data[26] * 256 + data[27]) / 100) * 3600 + ((data[24] * 16777216 + data[25] * 65536 + data[26] * 256 + data[27]) % 100) * 60;
	esensor->curacctime = ((data[28] * 16777216 + data[29] * 65536 + data[30] * 256 + data[31]) / 100) * 3600 + ((data[28] * 16777216 + data[29] * 65536 + data[30] * 256 + data[31]) % 100) * 60;

	return 1;
}

int zb_query_esensor_data(esensor_info *esensor)		//请求逆变器实时数据
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	int signalstrength;
	
	print2msg("Query eSensor data",esensor->id);
	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[i++] = 0x55;
	sendbuff[i++] = 0xAA;
	sendbuff[i++] = 0xC3;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;

	
	zb_send_esensor_cmd(esensor, sendbuff, i);
	ret = zb_get_esensor_reply(data,esensor);
	if((88 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[86])&&(0xFE == data[87]))
	{
		esensor->no_getdata_num = 0;	//一旦接收到数据就清0，ZK
		esensor->dataflag = 1;	//接受到数据就置为1

		resolvedata_esensor(&data[4], esensor);

		return 1;
	}
	else
	{
		esensor->dataflag = 0;		//没有接受到数据就置为0
		return -1;
	}
}
