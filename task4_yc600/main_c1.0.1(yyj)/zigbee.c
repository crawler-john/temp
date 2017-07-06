#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include "variation.h"
#include "debug.h"
#include "sqlite3.h"
#include "ema_control.h"
#include <string.h>
#include "debug.h"

#define BAUDRATE B57600
#define MODEMDEVICE "/dev/ttyO2"

extern ecu_info ecu;
extern sqlite3 *db;
int zbmodem;				//zigbee串口
extern int processpower(inverter_info *firstinverter);

void clear_zbmodem(void)		//清空串口缓冲区的数据
{
	tcflush(zbmodem,TCIOFLUSH);
	sleep(1);
}

int reset_zigbee()
{
	int fd_reset;

	fd_reset = open("/dev/reset",O_RDONLY);
	ioctl(fd_reset,0,NULL);	// 2.GPIO reset
	close(fd_reset);

	printmsg("Reset has been opened");

	return fd_reset;
}

int openzigbee(void)		//打开串口
{
	struct termios newtio;
	zbmodem = open(MODEMDEVICE, O_RDWR | O_NOCTTY);		//打开串口
	if(zbmodem < 0){
		perror("MODEMDEVICE");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 1;
	newtio.c_cc[VMIN] = 100;
	tcflush(zbmodem, TCIFLUSH);
	tcsetattr(zbmodem, TCSANOW, &newtio);

	return zbmodem;
}

int zigbee_reset(void)		//复位zigbee模块
{
    int fd_reset;

    fd_reset=open("/dev/reset",O_RDONLY);
    ioctl(fd_reset,0,NULL);	// 2.GPIO reset
    close(fd_reset);
    printmsg("Reset Zigbee");
    sleep(10);

    return 0;
}


int zb_shortaddr_cmd(int shortaddr, char *buff, int length)		//zigbee 短地址包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = shortaddr>>8;
	sendbuff[6]  = shortaddr;
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

	printhexmsg("zb_shortaddr_cmd", sendbuff, 15);


	usleep(50000);
	if(0!=shortaddr)
	{
		write(zbmodem, sendbuff, length+15);
		return 1;
	}
	else
		return -1;
}

int zb_shortaddr_reply(char *data,int shortaddr,char *id)			//读取逆变器的返回帧,短地址模式
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int ret, temp_size,size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	if(select(zbmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
	//	inverter->signalstrength=0;
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
		printhexmsg("Reply", data_all, temp_size);
		sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==shortaddr/256)&&(data_all[3]==shortaddr%256)&&(data_all[5]==0xA5)&&(0==strcmp(id,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}
	}
}


int zb_get_reply(char *data,inverter_info *inverter)			//读取逆变器的返回帧
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int ret, temp_size,size;
	fd_set rd;
	struct timeval timeout;
	
	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	
	if(select(zbmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
		inverter->signalstrength=0;
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
		printhexmsg("Reply", data_all, temp_size);
		sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(0==strcmp(inverter->id,inverterid)))
		{
			inverter->raduis=data_all[5];
			inverter->signalstrength=data_all[4];
			return size;
		}
		else
		{
			inverter->signalstrength=0;
			return -1;
		}
	}
}


int zb_get_reply_update_start(char *data,inverter_info *inverter)			//读取逆变器远程更新的Update_start返回帧，ZK，返回响应时间定为10秒
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int ret, temp_size,size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	if(select(zbmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
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
		printhexmsg("Reply", data_all, temp_size);
		sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==strcmp(inverter->id,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}
	}
}

int zb_get_reply_restore(char *data,inverter_info *inverter)			//读取逆变器远程更新失败，还原指令后的返回帧，ZK，因为还原时间比较长，所以单独写一个函数
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int ret, temp_size,size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = 200;
	timeout.tv_usec = 0;

	if(select(zbmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
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
		printhexmsg("Reply", data_all, temp_size);
		sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);

		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==strcmp(inverter->id,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}
	}
}

int zb_get_reply_from_module(char *data)			//读取zigbee模块的返回帧
{
	int ret, size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	if(select(zbmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
		return -1;
	}
	else
	{
		size = read(zbmodem, data, 255);
		printhexmsg("Reply", data, size);
		return size;
	}
}

int zb_get_id(char *data)			//获取逆变器ID
{
	int ret, size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;

	if(select(zbmodem+1, &rd, NULL , NULL, &timeout) <= 0){
		printmsg("Get id time out");
		return -1;
	}
	else{
		size = read(zbmodem, data, 11);
		printhexmsg("Reply", data, size);

		return size;
	}
}

int zb_turnoff_limited_rptid(int short_addr,inverter_info *inverter)			//关闭限定单个逆变器上报ID功能
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	int check=0;
	char data[256];


	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x08;
	sendbuff[5]  = short_addr>>8;
	sendbuff[6]  = short_addr;
	sendbuff[7]  = 0x08;//panid
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x19;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	if(0!=inverter->shortaddr)
	{
		printmsg("Turn off limited report id");
		write(zbmodem, sendbuff, 15);
		printhexmsg("sendbuff",sendbuff,15);
		ret = zb_get_reply_from_module(data);
		if((11 == ret)&&(0xA5 == data[2])&&(0xA5 == data[3]))
		{
			if(inverter->zigbee_version!=data[9])
			{
				inverter->zigbee_version = data[9];
				update_zigbee_version(inverter);
			}
			return 1;
		}
		else
			return -1;
	}
	else
		return -1;

}

int zb_turnoff_rptid(int short_addr)			//关闭单个逆变器上报ID功能
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;

	printmsg("Turn off report id");

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x08;
	sendbuff[5]  = short_addr>>8;
	sendbuff[6]  = short_addr;
	sendbuff[7]  = 0x08;//panid
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x19;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	write(zbmodem, sendbuff, 15);
//	ret = zb_get_reply(data);
//	if((11 == ret)&&(0xA5 == data[2])&&(0xA5 == data[3]))
		return 1;
//	else
//		return -1;
}


int zb_get_inverter_shortaddress_single(inverter_info *inverter)			//获取单台指定逆变器短地址,ZK
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;
	char inverterid[13] = {'\0'};

	printmsg("Get inverter shortaddresssingle");

	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0E;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;//panid
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;

	sendbuff[15]=((inverter->id[0]-0x30)*16+(inverter->id[1]-0x30));
	sendbuff[16]=((inverter->id[2]-0x30)*16+(inverter->id[3]-0x30));
	sendbuff[17]=((inverter->id[4]-0x30)*16+(inverter->id[5]-0x30));
	sendbuff[18]=((inverter->id[6]-0x30)*16+(inverter->id[7]-0x30));
	sendbuff[19]=((inverter->id[8]-0x30)*16+(inverter->id[9]-0x30));
	sendbuff[20]=((inverter->id[10]-0x30)*16+(inverter->id[11]-0x30));

//	strcpy(&sendbuff[15],inverter->id);


	write(zbmodem, sendbuff, 21);
	printhexmsg("sendbuff",sendbuff,21);
	ret = zb_get_reply_from_module(data);

	sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data[4],data[5],data[6],data[7],data[8],data[9]);

	if((11 == ret)&&(0xFF == data[2])&&(0xFF == data[3])&&(0==strcmp(inverter->id,inverterid)))
	{
		inverter->shortaddr = data[0]*256 + data[1];
		update_inverter_addr(inverter->id,inverter->shortaddr);
		return 1;
	}
	else
		return -1;

}

//组网(令逆变器上报ID，并对存在于ID表中的逆变器进行绑定)
int zb_turnon_limited_rtpid(inverter_info *firstinverter)
{
    char sql[1024] = {'\0'};
    char *zErrMsg = 0;
    int nrow, ncolumn;
    char **azResult;
    int time_start;

	char sendbuff[512] = {'\0'};
	char data[256];
	char tmp_inverterid[256] = {'\0'};
	int i, m = 0;
	int real_count = 0;	//已经获取到的逆变器数量
	int short_addr;
	inverter_info *inverter = firstinverter;

	strcpy(sql, "SELECT id FROM id");
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);

	for (m=0; m<2; m++) {

		//设置信道
		process_channel(nrow, azResult);

		//设置PANID（假设在同一信道下）
		zb_restore_ecu_panid_0xffff(ecu.channel); //PANID为0xFFFF（万能发送）,信道为配置文件中的信道
		for (i=1; i<=nrow; i++) {
			if (NULL != azResult[i] && strlen(azResult[i])) {
				zb_change_inverter_channel_one(azResult[i], ecu.channel);
			}
		}
		zb_change_ecu_panid(); //设置ECU：PANID为ECU的MAC地址后四位，信道为配置文件中的信道

		//发送广播上报逆变器ID的命令
		printmsg("Turn on report id limited");
		clear_zbmodem();
		sendbuff[0]  = 0xAA;
		sendbuff[1]  = 0xAA;
		sendbuff[2]  = 0xAA;
		sendbuff[3]  = 0xAA;
		sendbuff[4]  = 0x01;
		sendbuff[5]  = 0x00;
		sendbuff[6]  = 0x00;
		sendbuff[7]  = 0x00;
		sendbuff[8]  = 0x00;
		sendbuff[9]  = 0x00;
		sendbuff[10] = 0x00;
		sendbuff[11] = 0xA0;
		sendbuff[12] = 0xA5;
		sendbuff[13] = 0xA5;
		sendbuff[14] = 0x00;
		write(zbmodem, sendbuff, 15);
		printhexmsg("SEND", sendbuff, 15);

		//限定上报逆变器ID的时间60秒,并将上报上来的短地址存入数据库
		time_start = time(NULL);
		while ((time(NULL) - time_start) <= 60) {
			//若读取到短地址的逆变器数量达到数据库中ID号数量时，就立即退出上报过程
			if (real_count >= nrow) break;

			//接收上报上来的逆变器ID
			memset(data, '\0', sizeof(data));
			memset(tmp_inverterid, '\0', sizeof(tmp_inverterid));
			if ((11 == zb_get_id(data)) && (0xFF == data[2]) && (0xFF == data[3])) {
				//解析出逆变器ID
				for (i=0; i<6; i++) {
					tmp_inverterid[2*i] = (data[i+4]>>4) + 0x30;
					tmp_inverterid[2*i+1] = (data[i+4]&0x0f) + 0x30;
				}
				print2msg("tmp_inverterid",tmp_inverterid);

				//将收到的逆变器ID与数据库中的逆变器ID进行匹配
				for (i=1; i<=nrow; i++) {
					if (!strcmp(azResult[i], tmp_inverterid)) {
						//解析出短地址并存入数据库
						short_addr = data[0]*256 + data[1];
						if (1 == update_inverter_addr(tmp_inverterid, short_addr)) {
							//清空存入数据库的逆变器ID，避免重复
							memset(azResult[i], '\0', strlen(azResult[i]));
							real_count++;
							printdecmsg("real_count", real_count);
							printdecmsg("nrow", nrow);
						}
						break;
					}
				}
			}
		}
		//若读取到短地址的逆变器数量达到数据库中ID号数量时，就立即退出上报过程
		if (real_count >= nrow) {
			break;
		}
		else if (m < 1) {
			//处理未获取到短地址的逆变器：将其信道切换到ECU的信道
			zb_change_channel(nrow, azResult);
		}
	}
	sqlite3_free_table(azResult);

	//关闭单个逆变器上报ID功能+绑定操作
	strcpy(sql, "SELECT short_address FROM id");
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	for (i=1; i<=nrow; i++) {
		if (NULL != azResult[i]) {
			zb_off_report_id_and_bind(atoi(azResult[i]));
		}
	}
	sqlite3_free_table(azResult);

	//重新获取一遍逆变器信息并返回逆变器数量
	return get_id_from_db(firstinverter);
}

int zb_turnon_rtpid(inverter_info *firstinverter)			//开启逆变器自动上报ID
{
	char sendbuff[512] = {'\0'};
	char data[256];
	int time_start;
	int i, j, count=0;
	int check=0;
	int short_addr;
	char inverterid[256] = {'\0'};

	printmsg("Turn on report id");

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x02;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;



	write(zbmodem, sendbuff, 15);

	time_start=time(NULL);
	while((time(NULL)-time_start)<=600)		//上报时间10分钟
	{
		memset(data, '\0', sizeof(data));
		if((11 == zb_get_id(data)) && (0xFF == data[2]) && (0xFF == data[3]))
		{
			short_addr = data[0] * 256 + data[1];
			for(i=0; i<6; i++){
				inverterid[2*i] = (data[i+4]>>4) + 0x30;
				inverterid[2*i+1] = (data[i+4]&0x0f) + 0x30;
			}
			print2msg("inverterid",inverterid);
			save_inverter_id(inverterid,short_addr);
			usleep(30000);
			zb_turnoff_rptid(short_addr);
			memset(inverterid, '\0', sizeof(inverterid));
		}
	}

	count=get_id_from_db(firstinverter);
	return count;
}

int zb_change_inverter_panid_broadcast(void)	//广播改变逆变器的PANID,ZK
{
	char sendbuff[512] = {'\0'};
	char data[256];
	int ret;
	int i;
	int check=0;

	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x03;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	write(zbmodem, sendbuff, 15);
	printhexmsg("sendbuff",sendbuff,15);

	return 1;
}


int zb_change_inverter_panid_single(inverter_info *inverter)	//单点改变逆变器的PANID和信道,ZK
{
	char sendbuff[512] = {'\0'};
	char data[256];
	int ret;
	int i;
	int check=0;

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter->id[0]-0x30)*16+(inverter->id[1]-0x30));
	sendbuff[16]=((inverter->id[2]-0x30)*16+(inverter->id[3]-0x30));
	sendbuff[17]=((inverter->id[4]-0x30)*16+(inverter->id[5]-0x30));
	sendbuff[18]=((inverter->id[6]-0x30)*16+(inverter->id[7]-0x30));
	sendbuff[19]=((inverter->id[8]-0x30)*16+(inverter->id[9]-0x30));
	sendbuff[20]=((inverter->id[10]-0x30)*16+(inverter->id[11]-0x30));

	write(zbmodem, sendbuff, 21);
	printhexmsg("sendbuff",sendbuff,21);

	sleep(1);
	return 1;

}

int zb_restore_inverter_panid_channel_single_0x8888_0x10(inverter_info *inverter)	//单点还原逆变器的PANID到0X8888和信道0X10,ZK
{
	char sendbuff[512] = {'\0'};
	char data[256];
	int ret;
	int i;
	int check=0;

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x88;
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x10;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter->id[0]-0x30)*16+(inverter->id[1]-0x30));
	sendbuff[16]=((inverter->id[2]-0x30)*16+(inverter->id[3]-0x30));
	sendbuff[17]=((inverter->id[4]-0x30)*16+(inverter->id[5]-0x30));
	sendbuff[18]=((inverter->id[6]-0x30)*16+(inverter->id[7]-0x30));
	sendbuff[19]=((inverter->id[8]-0x30)*16+(inverter->id[9]-0x30));
	sendbuff[20]=((inverter->id[10]-0x30)*16+(inverter->id[11]-0x30));

	write(zbmodem, sendbuff, 21);
	printhexmsg("sendbuff",sendbuff,21);

	sleep(1);
	return 1;

}

//设置ECU的PANID和信道
int zb_change_ecu_panid(void)
{
	unsigned char sendbuff[16] = {'\0'};
	char recvbuff[256] = {'\0'};
	int i;
	int check=0;

	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;
	write(zbmodem, sendbuff, 15);
	printhexmsg("Set ECU PANID and Channel", sendbuff, 15);

	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		sleep(2); //延时2秒，因为设置完ECU的信道和PANID后会发6个FF
		return 1;
	}

	return -1;
}

int zb_restore_ecu_panid_0x8888(void)			//恢复ECU的PANID为0x8888,ZK
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x88;
	sendbuff[8]  = 0x88;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	write(zbmodem, sendbuff, 15);
	printhexmsg("sendbuff",sendbuff,15);
	ret = zb_get_reply_from_module(data);
	if((3 == ret)&&(0xAB == data[0])&&(0xCD == data[1])&&(0xEF == data[2]))
		return 1;
	else
		return -1;
}

//设置ECU的PANID为0xFFFF,信道为指定信道(注：向逆变器发送设置命令时，需将ECU的PANID设为0xFFFF)
int zb_restore_ecu_panid_0xffff(int channel)
{
	unsigned char sendbuff[15] = {'\0'};
	char recvbuff[256];
	int i;
	int check=0;

	//向ECU发送命令
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0xFF;
	sendbuff[8]  = 0xFF;
	sendbuff[9]  = channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;
	write(zbmodem, sendbuff, 15);
	printhexmsg("Change ECU channel (PANID:0xFFFF)", sendbuff, 15);

	//接收反馈
	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		sleep(2);
		return 1;
	}

	return -1;
}

int zb_send_cmd(inverter_info *inverter, char *buff, int length)		//zigbee包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = inverter->shortaddr>>8;
	sendbuff[6]  = inverter->shortaddr;
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

	printdecmsg("shortaddr",inverter->shortaddr);
	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}

	usleep(100000);
	if(0!=inverter->shortaddr)
	{
		write(zbmodem, sendbuff, length+15);
		//printhexmsg("Send", sendbuff, length+15);
		return 1;
	}
	else
		return -1;
}

int zb_broadcast_cmd(char *buff, int length)		//zigbee广播包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
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

	write(zbmodem, sendbuff, length+15);

	return 1;
}

//请求逆变器的机型码和版本号
int zb_query_inverter_info(inverter_info *inverter)
{
	int i = 0, ret;
	char sendbuff[256];
	char recvbuff[256];
	
	clear_zbmodem();
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDC;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xE2;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	zb_send_cmd(inverter, sendbuff, i);
	print2msg("Query Inverter's Model and Version", inverter->id);

	if ((16 == zb_get_reply(recvbuff, inverter))
			&& (0xFB == recvbuff[0])
			&& (0xFB == recvbuff[1])
			&& (0x09 == recvbuff[2])
			&& (0xDC == recvbuff[3])
			//&& (0xDC == recvbuff[5])
			&& (0xFE == recvbuff[14])
			&& (0xFE == recvbuff[15])) {
		inverter->model = recvbuff[4];
		inverter->version = (recvbuff[5]*256 + recvbuff[6])*1000+(recvbuff[8]*256+recvbuff[9]);
		return 1;
	}

	return -1;
}

int zb_query_data(inverter_info *inverter)		//请求逆变器实时数据
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	int signalstrength;
	
	print2msg("Query inverter data",inverter->id);
	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xBB;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC1;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((88 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[86])&&(0xFE == data[87]))
	{
		inverter->no_getdata_num = 0;	//一旦接收到数据就清0，ZK
		inverter->dataflag = 1;	//接受到数据就置为1

		if(7==inverter->model)
			resolvedata_600(&data[4], inverter);

//		if(1==inverter->model)
//			resolvedata_250(&data[4], inverter);
//		else if(2==inverter->model)
//			resolvedata_250(&data[4], inverter);
//		else if(3==inverter->model)
//			resolvedata_500(&data[4], inverter);
//		else if(4==inverter->model)
//			resolvedata_500(&data[4], inverter);
		else if(5==inverter->model)
			resolvedata_1000(&data[4], inverter);
		else if(6==inverter->model)
			resolvedata_1000(&data[4], inverter);
		else
			{;}

		return 1;
	}
	else if((68 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[86])&&(0xFE == data[87]))
	{
		inverter->no_getdata_num = 0;	//一旦接收到数据就清0，ZK
		inverter->dataflag = 1;	//接受到数据就置为1

		resolvedata_600_new(&data[4], inverter);
	}
	else
	{
		inverter->dataflag = 0;		//没有接受到数据就置为0
		return -1;
	}
}

int zb_test_communication(void)		//zigbee测试通信有没有断掉
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;


	printmsg("test zigbee communication");
	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0D;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	write(zbmodem, sendbuff, 15);
	ret = zb_get_reply_from_module(data);
	if((3 == ret)&&(0xAB == data[0])&&(0xCD == data[1])&&(0xEF == data[2]))
		return 1;
	else
		return -1;

}


int zb_set_protect_parameter(inverter_info *inverter, char *protect_parameter)		//参数修改CC指令
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = protect_parameter[0];
	sendbuff[i++] = protect_parameter[1];
	sendbuff[i++] = protect_parameter[2];
	sendbuff[i++] = protect_parameter[3];
	sendbuff[i++] = protect_parameter[4];
	sendbuff[i++] = protect_parameter[5];
	sendbuff[i++] = protect_parameter[6];
	sendbuff[i++] = protect_parameter[7];
	sendbuff[i++] = protect_parameter[8];
	sendbuff[i++] = protect_parameter[9];
	sendbuff[i++] = protect_parameter[10];
	sendbuff[i++] = protect_parameter[11];
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	print2msg(inverter->id,"Set protect parameters");
	printhexmsg("Set protect parameters", sendbuff, i);
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;

}

int zb_query_protect_parameter(inverter_info *inverter, char *protect_data_DD_reply)		//存储参数查询DD指令
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	print2msg(inverter->id, "Query protect parameter");
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(protect_data_DD_reply,inverter);
	if((33 == ret) && (0xDD == protect_data_DD_reply[3]) && (0xFB == protect_data_DD_reply[0]) && (0xFB == protect_data_DD_reply[1]) && (0xFE == protect_data_DD_reply[31]) && (0xFE == protect_data_DD_reply[32]))
		return 1;
	if((58 == ret) &&
				(0xFB == protect_data_DD_reply[0]) &&
				(0xFB == protect_data_DD_reply[1]) &&
				(0xDA == protect_data_DD_reply[3]) &&
				(0xFE == protect_data_DD_reply[56]) &&
				(0xFE == protect_data_DD_reply[57]))
		return 1;
	else
		return -1;
}

int zb_afd_broadcast(void)		//AFD广播指令
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xAF;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);

	return 1;
}

int zb_turnon_inverter_broadcast(void)		//开机指令广播,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA1;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_boot_single(inverter_info *inverter)		//开机指令单播,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC1;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_shutdown_broadcast(void)		//关机指令广播,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA2;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_shutdown_single(inverter_info *inverter)		//关机指令单播,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC2;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_boot_waitingtime_single(inverter_info *inverter)		//开机等待时间启动控制单播，OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xCD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_clear_gfdi_broadcast(void)		//清除GFDI广播,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xAF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_clear_gfdi(inverter_info *inverter)		//清除GFDI,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xCF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_ipp_broadcast(void)		//IPP广播
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA5;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}
int zb_ipp_single(inverter_info *inverter)		//IPP单播
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC5;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_frequency_protectime_broadcast(void)		//欠频保护时间广播
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC5;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}
int zb_frequency_protectime_single(inverter_info *inverter)		//欠频保护时间单播
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC6;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x04;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_voltage_protectime_broadcast(void)		//欠压保护时间广播
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC9;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}
int zb_voltage_protectime_single(inverter_info *inverter)		//欠压保护时间单播
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xCA;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x04;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int process_gfdi(inverter_info *firstinverter)
{
	int i,j;
	FILE *fp;
	char command[256] = {'\0'};
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/process_gfdi.conf", "r");
	while(1){
		curinverter = firstinverter;
		memset(command, '\0', 256);
		fgets(command, 256, fp);
		if(!strlen(command))
			break;
		if('\n' == command[strlen(command)-1])
			command[strlen(command)-1] = '\0';
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++){
			if(!strcmp(command, curinverter->id))
			{
				j=0;
				while(j<3)
				{
					if(1 == zb_clear_gfdi(curinverter))
					{
						print2msg("Clear GFDI", curinverter->id);
						break;
					}
					j++;
				}
				break;
			}
			curinverter++;
		}
	}

	fclose(fp);
	fp = fopen("/tmp/process_gfdi.conf", "w");
	fclose(fp);

	return 0;
}

int compare_protect_data(char *set_protect_data, char *actual_protect_data)				//比较逆变器返回的预设值和页面上输入的预设值
{
	int i;

	for(i=0; i<12; i++)
	{
		if(set_protect_data[i] != actual_protect_data[i])
			return -1;
	}

	return 0;
}

int resolve_presetdata(inverter_info *inverter, char * protect_data_result)	//解析逆变器返回的参数保护值
{
	float temp;

	/*if(1 == ecu.type)
		temp = (presetdata[0]*256 + presetdata[1])/2.90345;
	else
		temp = (presetdata[0]*256 + presetdata[1])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_vl1 = (int)temp +1;
	else
		inverter->protect_vl1 = (int)temp;

	if(1 == ecu.type)
		temp = (presetdata[2]*256 + presetdata[3])/2.90345;
	else
		temp = (presetdata[2]*256 + presetdata[3])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_vu1 = (int)temp +1;
	else
		inverter->protect_vu1 = (int)temp;*/

	/*
	if(1 == ecu.type)
		temp = (protect_data_result[4]*256 + protect_data_result[5])/2.90345;
	else
		temp = (protect_data_result[4]*256 + protect_data_result[5])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_min = (int)temp +1;
	else
		inverter->protect_voltage_min = (int)temp;

	if(1 == ecu.type)
		temp = (protect_data_result[6]*256 + protect_data_result[7])/2.90345;
	else
		temp = (protect_data_result[6]*256 + protect_data_result[7])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_max = (int)temp +1;
	else
		inverter->protect_voltage_max = (int)temp;

	if(1 == ecu.type)
		inverter->protect_frequency_min = 600-protect_data_result[8];
	else
		inverter->protect_frequency_min = 500-protect_data_result[8];
	if(1 == ecu.type)
		inverter->protect_frequency_max = 600+protect_data_result[9];
	else
		inverter->protect_frequency_max = 500+protect_data_result[9];
	inverter->recovery_time = protect_data_result[10]*256 + protect_data_result[11];
*/
	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))	//电压保护下限
		temp = (protect_data_result[3]*65536 + protect_data_result[4]*256 + protect_data_result[5])/26204.64;
	if((5==inverter->model)||(6==inverter->model))
		temp = (protect_data_result[3]*65536 + protect_data_result[4]*256 + protect_data_result[5])/11614.45;

	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_min = (int)temp +1;
	else
		inverter->protect_voltage_min = (int)temp;

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))	//电压保护上限
		temp = (protect_data_result[6]*65536 + protect_data_result[7]*256 + protect_data_result[8])/26204.64;
	if((5==inverter->model)||(6==inverter->model))
		temp = (protect_data_result[6]*65536 + protect_data_result[7]*256 + protect_data_result[8])/11614.45;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_max = (int)temp +1;
	else
		inverter->protect_voltage_max = (int)temp;

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))
		inverter->protect_frequency_min = 223750/(protect_data_result[9]*256 + protect_data_result[10]);
	if((5==inverter->model)||(6==inverter->model))
		inverter->protect_frequency_min = 256000/(protect_data_result[9]*256 + protect_data_result[10]);

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))
		inverter->protect_frequency_max = 223750/(protect_data_result[11]*256 + protect_data_result[12]);
	if((5==inverter->model)||(6==inverter->model))
		inverter->protect_frequency_max = 256000/(protect_data_result[11]*256 + protect_data_result[12]);

	inverter->recovery_time = protect_data_result[13]*256 + protect_data_result[14];

	return 1;
}

int process_protect_data(inverter_info *firstinverter)
{
	FILE *fp;
	int flag = 0;
	int i,j;
	char buff[256] = {'\0'};
	char protect_data_yc500_yc200[20]={0};		//保存ECU发送给逆变器的预设值（在参数配置页面上输入的参数经转换）
	char protect_data_yc1000[20]={0};
	inverter_info *curinverter = firstinverter;
	char protect_data_DD_reply[256]={'\0'};


	fp = fopen("/tmp/presetdata.conf", "r");


	if(fp){
		fgets(buff, 255, fp);
		fclose(fp);
	}

	if(!strlen(buff))
		flag = 0;
	if('0' == buff[0])
		flag = 0;
	if('1' == buff[0])
		flag = 1;
	if('2' == buff[0])
		flag = 2;


	if(1 == flag)
	{

		get_protect_data_from_db(protect_data_yc500_yc200,protect_data_yc1000);

		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)
		{
			if(((1==curinverter->model)||(2==curinverter->model)||(3==curinverter->model)||(4==curinverter->model))&&(curinverter->model!=0))
			{
				for(j=0;j<3;j++)
				{
					if(1 == zb_set_protect_parameter(curinverter,protect_data_yc500_yc200))
					{
						print2msg("set_protect_parameter_successful", curinverter->id);

						if(1 == zb_query_protect_parameter(curinverter, protect_data_DD_reply))
						{
							resolve_and_update_inverter_protect_parameter(curinverter, &protect_data_DD_reply[4]);

							if(0 == compare_protect_data(protect_data_yc500_yc200, &protect_data_DD_reply[7]))	//读取成功，且与页面上输入的值相等
							{
								print2msg("compare_protect_parameter_successful", curinverter->id);
								break;
							}

						}
					}
				}
			}
			else if(((5==curinverter->model)||(6==curinverter->model))&&(curinverter->model!=0))
			{
				for(j=0;j<3;j++)
				{
					if(1 == zb_set_protect_parameter(curinverter,protect_data_yc1000))
					{
						print2msg("set_protect_parameter_successful", curinverter->id);
						if(1 == zb_query_protect_parameter(curinverter, protect_data_DD_reply))
						{
							resolve_and_update_inverter_protect_parameter(curinverter, &protect_data_DD_reply[4]);
							if(0 == compare_protect_data(protect_data_yc1000, &protect_data_DD_reply[7]))		//读取成功，且与页面上输入的值相等
							{
								print2msg("compare_protect_parameter_successful", curinverter->id);
								break;
							}
						}
					}
				}
			}
			else
			{
				;
			}
		}

		save_protect_result(firstinverter);

	//	query_protect_data_from_inverter(firstinverter, protect_data_yc500_yc200,protect_data_yc1000);
	//	display_protect_result(firstinverter);


		fp = fopen("/tmp/presetdata.conf", "w");
		fprintf(fp, "0");
		fclose(fp);
	}


	if(2 == flag)
	{
		if(1 == zb_query_protect_parameter(curinverter, protect_data_DD_reply))
		{
			resolve_and_update_inverter_protect_parameter(curinverter, &protect_data_DD_reply[4]);
		}

		save_protect_result(firstinverter);

		fp = fopen("/tmp/presetdata.conf", "w");
		fprintf(fp, "0");
		fclose(fp);
	}




	return 0;
}


int process_turn_on_off(inverter_info *firstinverter)
{
	int i, j;
	FILE *fp;
	char command[256] = {'\0'};
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/connect.conf", "r");
	if(!fp)
		return -1;
	while(1){
		curinverter = firstinverter;
		memset(command, '\0', 256);
		fgets(command, 256, fp);
		if(!strlen(command))
			break;
		if('\n' == command[strlen(command)-1])
			command[strlen(command)-1] = '\0';
		if(!strncmp(command, "connect all", 11)){
			zb_turnon_inverter_broadcast();
			printmsg("turn on all");
			break;
		}
		if(!strncmp(command, "disconnect all", 14)){
			zb_shutdown_broadcast();
			printmsg("turn off all");
			break;
		}

		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++){
			if(!strncmp(command, curinverter->id, 12))
			{
				j = 0;
				if('c' == command[12])
				{
					while(j<3)
					{
						if(1 == zb_boot_single(curinverter))
						{
							print2msg("turn on", curinverter->id);
							break;
						}
						j++;
					}
				}
				if('d' == command[12])
				{
					while(j<3)
					{
						if(1 == zb_shutdown_single(curinverter))
						{
							print2msg("turn off", curinverter->id);
							break;
						}
						j++;
					}
				}
			}
			curinverter++;
		}
	}

	fclose(fp);
	fp = fopen("/tmp/connect.conf", "w");
	fclose(fp);

	return 0;
}

int process_quick_boot(inverter_info *firstinverter)
{
	int i;
	FILE *fp;
	char flag_quickboot = '0';				//快速启动标志
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/quick_boot.conf", "r");

	if(fp)
	{
		flag_quickboot = fgetc(fp);
		fclose(fp);
		if('1' == flag_quickboot)
		{
			curinverter = firstinverter;
			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
			{
				zb_boot_waitingtime_single(curinverter);
				print2msg("quick boot",curinverter->id);
				curinverter++;
			}
		}
		fp = fopen("/tmp/quick_boot.conf", "w");
		if(fp)
		{
			fputs("0",fp);
			fclose(fp);
		}
	}
	return 0;
}

int process_ipp(inverter_info *firstinverter)
{
	int i, j;
	FILE *fp;
	char command[256] = {'\0'};
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/ipp.conf", "r");

	if(fp)
	{
		while(1)
		{
			curinverter = firstinverter;
			memset(command, '\0', 256);
			fgets(command, 256, fp);
			if(!strlen(command))
				break;
			if('\n' == command[strlen(command)-1])
				command[strlen(command)-1] = '\0';
			if(!strncmp(command, "set ipp all", 11)){
				zb_ipp_broadcast();
				printmsg("set ipp all");
				break;
			}

			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++){
				if(!strcmp(command, curinverter->id))
				{
					j=0;
					while(j<3)
					{
						if(1 == zb_ipp_single(curinverter))
						{
							print2msg("set ipp single", curinverter->id);
							break;
						}
						j++;
					}
					break;
				}
				curinverter++;
			}
		}

		fclose(fp);
	}
	fp = fopen("/tmp/ipp.conf", "w");
	fclose(fp);

	return 0;
}

int save_A145_inverter_to_all()
{//*******//here
	char sql[1024] = {'\0'};
	char *zErrMsg = 0;
	char **azResult;
	int nrow, ncolumn, item,i,length;


	memset(sql, '\0', sizeof(sql));
	strcpy(sql,"SELECT * FROM inverter_process_result WHERE item=145 AND flag=3");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(nrow>0)
	{
		length=19*nrow+51;
		char tm[16]={'\0'};
		char tm1[3]={'\0'};
		get_time(tm, tm1);
		printmsg(tm);
		memset(sql,'\0',1024);
		sprintf(sql,"REPLACE INTO process_result (item,result,flag) VALUES(145,'APS13%05dA145AAA0%.12s%04d%.14sEND",length,ecu.id,nrow,tm);
		for(i=1;i<nrow+1;i++)
		{
			strcat(sql,azResult[i*ncolumn+2]);
		}
		strcat(sql,"',1)");
		print2msg("sql",sql);
		sqlite3_exec(db, sql , 0, 0, &zErrMsg);
		memset(sql,'\0',1024);
		sprintf(sql,"DELETE FROM inverter_process_result WHERE item=145 AND flag=3");
		sqlite3_exec(db, sql , 0, 0, &zErrMsg);

		sqlite3_free_table( azResult );
	}


	return 0;
}

int process_all(inverter_info *firstinverter)
{
	open_led_comm();
	set_cc_ct();                //ct环设置
	do_cc(firstinverter);		//防逆流
	processpower(firstinverter);			//设置功率预设值,ZK,3.10有改动
//	process_gfdi(firstinverter);			//清GFDI标志
//	process_protect_data(firstinverter);	//设置预设值
	process_turn_on_off(firstinverter);		//开关机
	process_quick_boot(firstinverter);		//快速启动
	process_ipp(firstinverter);				//IPP设定
	process_ird_all(firstinverter);
	process_ird(firstinverter);
	turn_on_off(firstinverter);								//开关机,ZK,3.10所加
	clear_gfdi(firstinverter);								//清GFDI标志,ZK,3.10所加
	set_protection_parameters(firstinverter);				//设置预设值广播,ZK,3.10所加
	set_protection_parameters_inverter_one(firstinverter);  //设置预设值单点,ZK,3.10所加
	save_A145_inverter_to_all();
	return 0;
}

int getalldata(inverter_info *firstinverter,int time_linux)		//获取每个逆变器的数据
{
	int i, j,ret;
	inverter_info *curinverter = firstinverter;
	int count=0, syspower=0;
	float curenergy=0;
	FILE *fp;

	for(i=0;i<3;i++)
	{
		if(-1==zb_test_communication())
			zigbee_reset();
		else
			break;
	}
	calibration_time_broadcast(firstinverter, time_linux); 	//YC1000补报：广播校准时间
	for(j=0; j<5; j++)
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)			//每个逆变器最多要5次数据
		{
			process_all(firstinverter);
			if((0 == curinverter->dataflag) && (0 != curinverter->shortaddr))
			{
				if(1 != curinverter->bindflag)
				{
					if(1 == zb_turnoff_limited_rptid(curinverter->shortaddr,curinverter))
					{
						curinverter->bindflag = 1;			//绑定逆变器标志位置1
						update_inverter_bind_flag(curinverter);
					}
				}

				if((0 == curinverter->model) )//&& (1 == curinverter->bindflag))
				{
					if(1 == zb_query_inverter_info(curinverter))
						update_inverter_model_version(curinverter);
				}

				if((0 != curinverter->model) )//&& (1 == curinverter->bindflag))
				//if(1)
				{
					zb_query_data(curinverter);
					usleep(200000);
				}
			}
			curinverter++;
		}
	}

	do_uncc(firstinverter);                 //
	ecu.polling_total_times++;				//ECU总轮询加1 ,ZK

	fp = fopen("/tmp/disconnect.txt", "w");
	if(fp)
	{
		curinverter = firstinverter;
		for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)						//统计当前一天逆变器与ECU没通信上的总次数， ZK
		{
			if((0 == curinverter->dataflag)&&(12 == strlen(curinverter->id)))
			{
				curinverter->disconnect_times++;
				fprintf(fp, "%s-%d-%d\n", curinverter->id,curinverter->disconnect_times,ecu.polling_total_times);
			}
			else if((1 == curinverter->dataflag)&&(12 == strlen(curinverter->id)))
			{
				fprintf(fp, "%s-%d-%d\n", curinverter->id,curinverter->disconnect_times,ecu.polling_total_times);
			}
		}
		fclose(fp);
	}

	curinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)		//统计连续没有获取到数据的逆变器 ZK，一旦接受到数据，此变量会清0
	{
		if((0 == curinverter->dataflag)&&(12 == strlen(curinverter->id)))
			curinverter->no_getdata_num++;
	}

	curinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++){							//统计当前多少个逆变器
		if((1 == curinverter->dataflag)&&(12 == strlen(curinverter->id)))
			count++;
	}
	ecu.count = count;

	curinverter = firstinverter;
	for(syspower=0, i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++){		//计算当前一轮系统功率
		if(1 == curinverter->dataflag){
			syspower += curinverter->op;
			syspower += curinverter->opb;
			syspower += curinverter->opc;
			syspower += curinverter->opd;
		}
	}
	ecu.system_power = syspower;

	curinverter = firstinverter;
	for(curenergy=0, i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++){		//计算当前一轮发电量
		if(1 == curinverter->dataflag){
			if((curinverter->model==5)||(curinverter->model==6)||(curinverter->model==7)) //yc1000主页上显示输入电量为历史发电量
			{
				curenergy += curinverter->curgeneration;
				curenergy += curinverter->curgenerationb;
				curenergy += curinverter->curgenerationc;
				curenergy += curinverter->curgenerationd;
			}
			else{
				curenergy += curinverter->output_energy;
				curenergy += curinverter->output_energyb;
				curenergy += curinverter->output_energyc;
			}
		}
	}
	ecu.current_energy = curenergy;

	update_tmpdb(firstinverter);

	fp=fopen("/tmp/id_without_bind.txt","w");								//为了统计显示有短地址但是没有绑定的逆变器ID
	if(fp)
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)
		{
			if((1!=curinverter->bindflag)&&(0!=curinverter->shortaddr))
			{
				fputs(curinverter->id,fp);
				fputs("\n",fp);
			}
		}
		fclose(fp);
	}

	fp = fopen("/tmp/signalstrength.txt", "w");
	if(fp)
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)	 //统计每台逆变器的信号强度， ZK
		{
			fprintf(fp, "%s%X\n", curinverter->id,curinverter->signalstrength);
		}
		fclose(fp);
	}

	fp = fopen("/tmp/raduis.txt", "w");
	if(fp)
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)	 //统计每台逆变器的信号强度， ZK
		{
			fprintf(fp, "%s%X\n", curinverter->id,curinverter->raduis);
		}
		fclose(fp);
	}

	write_gfdi_status(firstinverter);
	write_turn_on_off_status(firstinverter);
	save_turn_on_off_changed_result(firstinverter);
	save_gfdi_changed_result(firstinverter);

	return ecu.count;
}

int get_inverter_shortaddress(inverter_info *firstinverter)		//获取没有数据的逆变器的短地址
{
	int i;
	inverter_info *curinverter = firstinverter;
	unsigned short current_panid;				//Zigbee即时的PANID
	int flag = 0;


	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
	{
		if((0==curinverter->shortaddr)||(curinverter->no_getdata_num>5))
		{
			flag=1;
			break;
		}
		curinverter++;
	}

	if(1==flag)
	{
		if(1==zb_restore_ecu_panid_0xffff(0x10))		//把ECU的PANID置为默认的0xffff
			current_panid = 0xffff;
		printdecmsg("PANID",current_panid);
		sleep(5);


		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
		{
			if((0==curinverter->shortaddr)||(curinverter->no_getdata_num>5))
			{
				zb_change_inverter_panid_single(curinverter);		//把没数据的逆变器PANID修改成ECU的PANID
				usleep(500000);
			}
			curinverter++;
		}

		if(1==zb_change_ecu_panid())						//把ECU的panid改成此台ECU原本的panid
			current_panid = ecu.panid;
		printdecmsg("PANID",current_panid);
		sleep(5);

		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
		{
			printdecmsg("no_getdata_num",curinverter->no_getdata_num);
			if((0==curinverter->shortaddr)||(curinverter->no_getdata_num>5))
			{
				zb_get_inverter_shortaddress_single(curinverter);
			}
			curinverter++;
		}
	}
	return 1;
}


int bind_nodata_inverter(inverter_info *firstinverter)		//绑定没有数据的逆变器,并且获取短地址
{
	int i;
	inverter_info *curinverter = firstinverter;

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
	{
		if(curinverter->no_getdata_num>0)
		{
//			zb_turnoff_limited_rptid(curinverter->shortaddr,curinverter);
			zb_get_inverter_shortaddress_single(curinverter);
		}
		curinverter++;
	}
	return 0;
}

//单点改变逆变器的PANID为ECU的MAC地址后四位，信道为指定信道(注：需要将ECU的PANID改为0xFFFF(万能发送))
int zb_change_inverter_channel_one(char *inverter_id, int channel)
{
	char sendbuff[512] = {'\0'};
	int i;
	int check=0;

	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter_id[0]-0x30)*16+(inverter_id[1]-0x30));
	sendbuff[16]=((inverter_id[2]-0x30)*16+(inverter_id[3]-0x30));
	sendbuff[17]=((inverter_id[4]-0x30)*16+(inverter_id[5]-0x30));
	sendbuff[18]=((inverter_id[6]-0x30)*16+(inverter_id[7]-0x30));
	sendbuff[19]=((inverter_id[8]-0x30)*16+(inverter_id[9]-0x30));
	sendbuff[20]=((inverter_id[10]-0x30)*16+(inverter_id[11]-0x30));
	write(zbmodem, sendbuff, 21);
	printhexmsg("Change Inverter Channel (one)", sendbuff, 21);

	sleep(1); //此处延时必须大于1秒
	return 0;
}

//改变传入逆变器ID的信道
int zb_change_channel(int num, char **ids)
{
	int i, j;
	char ecu_channel[16];

	clear_zbmodem();
	ecu_channel[0] = 0x0B;
	ecu_channel[1] = 0x0C;
	ecu_channel[2] = 0x0D;
	ecu_channel[3] = 0x0E;
	ecu_channel[4] = 0x0F;
	ecu_channel[5] = 0x10;
	ecu_channel[6] = 0x11;
	ecu_channel[7] = 0x12;
	ecu_channel[8] = 0x13;
	ecu_channel[9] = 0x14;
	ecu_channel[10] = 0x15;
	ecu_channel[11] = 0x16;
	ecu_channel[12] = 0x17;
	ecu_channel[13] = 0x18;
	ecu_channel[14] = 0x19;
	ecu_channel[15] = 0x1A;

	//改变每台逆变器的信道
	for (i=0; i<16; i++) {
		zb_restore_ecu_panid_0xffff(ecu_channel[i]); //改变ECU信道
		for (j=1; j<=num; j++) {
			if (NULL != ids[j] && strlen(ids[j])) {
				print2msg("inverter", ids[j]);
				zb_change_inverter_channel_one(ids[j], ecu.channel);
			}
		}
	}

	//设置ECU的信道为配置文件中的信道
	zb_change_ecu_panid();

	return 0;
}

//还原逆变器信道
int zb_reset_channel(int num, char **ids)
{
	int i, j;
	char ecu_channel[16];

	clear_zbmodem();
	ecu_channel[0] = 0x0B;
	ecu_channel[1] = 0x0C;
	ecu_channel[2] = 0x0D;
	ecu_channel[3] = 0x0E;
	ecu_channel[4] = 0x0F;
	ecu_channel[5] = 0x10;
	ecu_channel[6] = 0x11;
	ecu_channel[7] = 0x12;
	ecu_channel[8] = 0x13;
	ecu_channel[9] = 0x14;
	ecu_channel[10] = 0x15;
	ecu_channel[11] = 0x16;
	ecu_channel[12] = 0x17;
	ecu_channel[13] = 0x18;
	ecu_channel[14] = 0x19;
	ecu_channel[15] = 0x1A;

	//还原每台逆变器的信道
	for (i=0; i<16; i++) {
		zb_restore_ecu_panid_0xffff(ecu_channel[i]); //改变ECU信道
		for (j=1; j<=num; j++) {
			if (NULL != ids[j] && strlen(ids[j])) {
				print2msg("inverter", ids[j]);
				zb_change_inverter_channel_one(ids[j], 0x10);
			}
		}
	}

	//设置ECU的信道为默认信道
	ecu.channel = 0x10;
	zb_change_ecu_panid();

	return 0;
}

//关闭逆变器ID上报 + 绑定逆变器
int zb_off_report_id_and_bind(int short_addr)
{
	int times = 3;
	char sendbuff[16] = {'\0'};
	char recvbuff[256] = {'\0'};
	int i;
	int check=0;

	do {
		//发送关闭逆变器ID上报+绑定操作
		clear_zbmodem();
		sendbuff[0]  = 0xAA;
		sendbuff[1]  = 0xAA;
		sendbuff[2]  = 0xAA;
		sendbuff[3]  = 0xAA;
		sendbuff[4]  = 0x08;
		sendbuff[5]  = short_addr>>8;
		sendbuff[6]  = short_addr;
		sendbuff[7]  = 0x00; //PANID(逆变器不解析)
		sendbuff[8]  = 0x00; //PANID(逆变器不解析)
		sendbuff[9]  = 0x00; //信道(逆变器不解析)
		sendbuff[10] = 0x00; //功率(逆变器不解析)
		sendbuff[11] = 0xA0;
		for(i=4;i<12;i++)
			check=check+sendbuff[i];
		sendbuff[12] = check/256;
		sendbuff[13] = check%256;
		sendbuff[14] = 0x00;
		write(zbmodem, sendbuff, 15);
		printhexmsg("Bind ZigBee", sendbuff, 15);

		//接收逆变器应答(短地址，ZigBee版本号，信号强度)
		if ((11 == zb_get_reply_from_module(recvbuff))
				&& (0xA5 == recvbuff[2])
				&& (0xA5 == recvbuff[3])) {
			update_turned_off_rpt_flag(short_addr, (int)recvbuff[9]);
			update_bind_zigbee_flag(short_addr);
			printmsg("Bind Successfully");
			return 1;
		}
	}while(--times);

	return 0;
}

int zigbeeRecvMsg(char *data, int timeout_sec)
{
	int count;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

	if (select(zbmodem+1, &rd, NULL, NULL, &timeout) <= 0) {
		printmsg("Get reply time out");
		return -1;
	} else {
		count = read(zbmodem, data, 255);
		printhexmsg("Reply", data, count);
		return count;
	}
}

