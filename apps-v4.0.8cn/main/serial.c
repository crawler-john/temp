/*
 * serial.c
 *
 *  Created on: 2012-11-15
 *      Author: aps
 *      使用方法：加在main.c文件开头处声明format()，加在button_pthread()语句后面添加turn_on_serial()，在resetinverter()前调用format()。
 *      		增加serial、set_serial两个页面程序和serial.conf配置文件
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <sqlite3.h>
#include "variation.h"

#define DEBUGINFO 0
#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyO3"

unsigned char sendbuff[65535];		//保存发送的数据帧
int sendlength;					//数据帧的长度
int serialfd;						//串口
int onflag = 0;						//开启串口通信功能，1表示开启，0表示关闭
int ecuaddress;					//ECU地址码
int baudrate;						//串口波特率
extern char ecuid[13];				//引用main.c中的ECU ID
extern sqlite3 *db;

int open_serial(void)
{
	int fd, res, i=0;
	struct termios newtio;
	
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);		//打开串口，9G45上的UART3
	if(fd<0){
		perror("MODEMDEVICE");
	}
	
	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = CS8 | CLOCAL | CREAD | CRTSCTS;
	if(2400 == baudrate)
		newtio.c_cflag |= B2400;
	if(4800 == baudrate)
		newtio.c_cflag |= B4800;
	if(9600 == baudrate)
		newtio.c_cflag |= B9600;
	if(19200 == baudrate)
		newtio.c_cflag |= B19200;
	if(38400 == baudrate)
		newtio.c_cflag |= B38400;
	if(57600 == baudrate)
		newtio.c_cflag |= B57600;
	if(115200 == baudrate)
		newtio.c_cflag |= B115200;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 1;
	newtio.c_cc[VMIN] = 8;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	
	return fd;
}

int check_crc(unsigned char *pchMsg, int wDataLen)
{
	unsigned short wCRCTalbeAbs[16] = {0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400};
	unsigned short wCRC = 0xFFFF;
	int i;
	unsigned char chChar, crcresult[2];
	for (i = 0; i < wDataLen; i++)
	{
		chChar = *pchMsg++;
		wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}
	crcresult[0] = wCRC & 0xff;
	crcresult[1] = (wCRC>>8)&0xff;
	printf("crcresult=%X, %X\n", crcresult[0], crcresult[1]);
	if(*(pchMsg++) == crcresult[0] && *(pchMsg) == crcresult[1])
		return 1;
	return 0;
}

void add_crc(unsigned char *pchMsg, int wDataLen)
{
	unsigned short wCRCTalbeAbs[16] = {0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400};
	unsigned short wCRC = 0xFFFF;
	int i;
	unsigned char chChar;
	for (i = 0; i < wDataLen; i++)
	{
		chChar = *pchMsg++;
		wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}
	*pchMsg++ = wCRC & 0xff;
	*pchMsg =  (wCRC>>8)&0xff;
}

void send_to_serial(void)
{
	write(serialfd, sendbuff, sendlength);
}

int sendbackinfo(char *sendbuff, int sendsize)
{
	write(serialfd, sendbuff, sendsize);
}

void sendrunmode()
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow, ncolumn, j=0;
	char flag=0;
	char sendbuff[256]={'\0'};

	sendbuff[j++] = ecuaddress;
	sendbuff[j++] = 0x02;
	sendbuff[j++] = 0x00;
	sendbuff[j++] = 0x01;

	strcpy(sql,"SELECT flag FROM power WHERE item=0;");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );

	print2msg("flag", azResult[1]);

	sendbuff[j++] = atoi(azResult[1]);
	sqlite3_free_table( azResult );

	add_crc(sendbuff, j);
	write(serialfd, sendbuff, j+2);
}

void turnoninverter()
{
	FILE *fp;

	fp = fopen("/tmp/connect.conf", "w");
	fprintf(fp, "connect all");
	fclose(fp);
}

void turnoffinverter()
{
	FILE *fp;

	fp = fopen("/tmp/connect.conf", "w");
	fprintf(fp, "disconnect all");
	fclose(fp);
}

void setmaxpower(int power)
{
	FILE *fp;

	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE power SET limitedpower=%d WHERE item>=0", power);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	memset(sql, '\0', 100);
	sprintf(sql,"UPDATE powerall SET maxpower=%d WHERE item=0", power);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	fp = fopen("/tmp/setmaxpower.conf", "w");
	fputs( "ALL", fp);
	fclose(fp);
}

void setfixpower(int power)
{
	FILE *fp;

	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE power SET stationarypower=%d WHERE item>=0", power);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	memset(sql, '\0', 100);
	sprintf(sql,"UPDATE powerall SET fixedpower=%d WHERE item=0", power);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	fp = fopen("/tmp/setfixpower.conf", "w");
	fputs( "ALL", fp);
	fclose(fp);
}

void setmaxsyspower(int power)
{
	FILE *fp;
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE powerall SET sysmaxpower=%d WHERE item>=0", power);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	fp = fopen("/tmp/setmaxpower.conf", "w");
	fputs( "MAX", fp);
	fclose(fp);
}

void setlowvol(int voltage)
{
	FILE *fp;
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE preset SET lv2=%d WHERE id=1 ", voltage);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	fp = fopen("/tmp/presetdata.conf", "w");
	fputc('1', fp);
	fclose(fp);
}

void setupvol(int voltage)
{
	FILE *fp;
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE preset SET uv2=%d WHERE id=1 ", voltage);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	fp = fopen("/tmp/presetdata.conf", "w");
	fputc('1', fp);
	fclose(fp);
}

void setlowfre(int frequency)
{
	FILE *fp;
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE preset SET lf=%d WHERE id=1 ", frequency);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	fp = fopen("/tmp/presetdata.conf", "w");
	fputc('1', fp);
	fclose(fp);
}

void setupfre(int frequency)
{
	FILE *fp;
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE preset SET uf=%d WHERE id=1 ", frequency);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	fp = fopen("/tmp/presetdata.conf", "w");
	fputc('1', fp);
	fclose(fp);
}

void listen_serial(void)
{
	char recvbuff[65535] = {'\0'};
	int res=0, recvsize=0;
	fd_set rd;
	struct timeval timeout;
	int i;

	while(1){
		FD_ZERO(&rd);
		FD_SET(serialfd, &rd);
		timeout.tv_sec = 0;
		timeout.tv_usec = 11.0/baudrate*3.5*1000000;
		recvsize = 0;
		memset(recvbuff, '\0', 65535);

		res = select(serialfd+1, &rd, NULL , NULL, &timeout);
		if(res>0){
			recvsize = read(serialfd, recvbuff, 255);
			printhexmsg("Read", recvbuff, recvsize);

			if(8 == recvsize)
				if(ecuaddress == recvbuff[0])
					if(check_crc(recvbuff, recvsize-2)){		//判断指令类型
						if((0x02==recvbuff[1])&&(0x00==recvbuff[2])&&(0x00==recvbuff[3]))		//读取所有逆变器数据
							send_to_serial();
						if((0x02==recvbuff[1])&&(0x00==recvbuff[2])&&(0x01==recvbuff[3]))		//读取最大功率跟踪模式、限制功率工作模式
							sendrunmode();
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x02==recvbuff[3])&&(0x00==recvbuff[4])&&(0x00==recvbuff[5])){		//设备启动
							turnoninverter();
							sendbackinfo(recvbuff, recvsize);
						}
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x02==recvbuff[3])&&(0x00==recvbuff[4])&&(0x01==recvbuff[5])){		//设备关闭
							turnoffinverter();
							sendbackinfo(recvbuff, recvsize);
						}
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x03==recvbuff[3])){		//设置最大功率跟踪模式
							setmaxpower(recvbuff[4]*256+recvbuff[5]);
							sendbackinfo(recvbuff, recvsize);
						}
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x04==recvbuff[3])){		//设置限制功率模式
							setfixpower(recvbuff[4]*256+recvbuff[5]);
							sendbackinfo(recvbuff, recvsize);
						}
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x05==recvbuff[3])){		//设置设备最大限制功率
							setmaxsyspower(recvbuff[4]*256+recvbuff[5]);
							sendbackinfo(recvbuff, recvsize);
						}
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x06==recvbuff[3])){		//设备欠压电压值
							setlowvol(recvbuff[4]*256+recvbuff[5]);
							sendbackinfo(recvbuff, recvsize);
						}
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x07==recvbuff[3])){		//设备过压电压值
							setupvol(recvbuff[4]*256+recvbuff[5]);
							sendbackinfo(recvbuff, recvsize);
						}
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x08==recvbuff[3])){		//设备欠频频率值
							setlowfre(recvbuff[4]*256+recvbuff[5]);
							sendbackinfo(recvbuff, recvsize);
						}
						if((0x05==recvbuff[1])&&(0x00==recvbuff[2])&&(0x09==recvbuff[3])){		//设备过频频率值
							setupfre(recvbuff[4]*256+recvbuff[5]);
							sendbackinfo(recvbuff, recvsize);
						}

					}
		}
	}
}

int format(struct inverter_info_t *firstinverter, char *datatime, int syspower, float curgeneration, float ltgeneration)		//curgen:当前发电量，ltgen:历史发电量
{
	int i, j=0, count=0;
	int tmp;
	struct inverter_info_t *curinverter = firstinverter;

	sendlength=0;

	if(onflag){											//只在串口功能开启时才转换协议
		memset(sendbuff, '\0', 65535);
		sendbuff[j++] = ecuaddress;
		sendbuff[j++] = 0x02;
		sendbuff[j++] = 0x00;
		sendbuff[j++] = 0x00;
		sendbuff[j++] = ((ecuid[0]-0x30)<<4)|(ecuid[1]-0x30);
		sendbuff[j++] = ((ecuid[2]-0x30)<<4)|(ecuid[3]-0x30);
		sendbuff[j++] = ((ecuid[4]-0x30)<<4)|(ecuid[5]-0x30);
		sendbuff[j++] = ((ecuid[6]-0x30)<<4)|(ecuid[7]-0x30);
		sendbuff[j++] = ((ecuid[8]-0x30)<<4)|(ecuid[9]-0x30);
		sendbuff[j++] = ((ecuid[10]-0x30)<<4)|(ecuid[11]-0x30);

		tmp = syspower*100;							//总功率
		sendbuff[j++] = tmp/16777216;
		tmp = tmp%16777216;
		sendbuff[j++] = tmp/65536;
		tmp = tmp%65536;
		sendbuff[j++] = tmp/256;
		sendbuff[j++] = tmp%256;

		printfloatmsg("curgeneration", curgeneration);
		printfloatmsg("ltgeneration", ltgeneration);

		tmp = curgeneration*1000000;
		sendbuff[j++] = tmp/16777216;
		tmp = tmp%16777216;
		sendbuff[j++] = tmp/65536;
		tmp = tmp%65536;
		sendbuff[j++] = tmp/256;
		sendbuff[j++] = tmp%256;

		tmp = ltgeneration*10;
		sendbuff[j++] = tmp/16777216;
		tmp = tmp%16777216;
		sendbuff[j++] = tmp/65536;
		tmp = tmp%65536;
		sendbuff[j++] = tmp/256;
		sendbuff[j++] = tmp%256;

		sendbuff[j++] = (datatime[0]-0x30)<<4 | (datatime[1]-0x30);
		sendbuff[j++] = (datatime[2]-0x30)<<4 | (datatime[3]-0x30);
		sendbuff[j++] = (datatime[4]-0x30)<<4 | (datatime[5]-0x30);
		sendbuff[j++] = (datatime[6]-0x30)<<4 | (datatime[7]-0x30);
		sendbuff[j++] = (datatime[8]-0x30)<<4 | (datatime[9]-0x30);
		sendbuff[j++] = (datatime[10]-0x30)<<4 | (datatime[11]-0x30);
		sendbuff[j++] = (datatime[12]-0x30)<<4 | (datatime[13]-0x30);

		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){
			if('1' == curinverter->curflag){
				if(1 == curinverter->flagyc500){
					sendbuff[j++] = curinverter->tnuid[0];
					sendbuff[j++] = curinverter->tnuid[1];
					sendbuff[j++] = curinverter->tnuid[2];
					sendbuff[j++] = curinverter->tnuid[3];
					sendbuff[j++] = curinverter->tnuid[4];
					sendbuff[j++] = curinverter->tnuid[5];
					sendbuff[j++] = 0x41;
					sendbuff[j++] = (int)(curinverter->dv)/256;
					sendbuff[j++] = (int)(curinverter->dv)%256;
					sendbuff[j++] = (int)(curinverter->di)/256;
					sendbuff[j++] = (int)(curinverter->di)%256;
					sendbuff[j++] = curinverter->op*100/256;
					sendbuff[j++] = curinverter->op*100%256;
					sendbuff[j++] = (int)(curinverter->np*10)/256;
					sendbuff[j++] = (int)(curinverter->np*10)%256;
					sendbuff[j++] = curinverter->it;
					sendbuff[j++] = curinverter->nv/256;
					sendbuff[j++] = curinverter->nv%256;
					sendbuff[j++] = curinverter->op*10.0/curinverter->nv;		//交流电流
					if('1' == curinverter->status[0])
						sendbuff[j] |= 0x80;
					if('1' == curinverter->status[1])
						sendbuff[j] |= 0x40;
					if('1' == curinverter->status[2])
						sendbuff[j] |= 0x20;
					if('1' == curinverter->status[3])
						sendbuff[j] |= 0x10;
					if('1' == curinverter->status[4])
						sendbuff[j] |= 0x08;
					if('1' == curinverter->status[5])
						sendbuff[j] |= 0x04;
					if('1' == curinverter->status[6])
						sendbuff[j] |= 0x02;
					if('1' == curinverter->status[7])
						sendbuff[j] |= 0x01;
					j++;
					if('1' == curinverter->status[8])
						sendbuff[j] |= 0x80;
					j++;
					tmp = curinverter->curgeneration*1000000;
					sendbuff[j++] = tmp/16777216;
					tmp = tmp%16777216;
					sendbuff[j++] = tmp/65536;
					tmp = tmp%65536;
					sendbuff[j++] = tmp/256;
					sendbuff[j++] = tmp%256;

					sendbuff[j++] = curinverter->tnuid[0];
					sendbuff[j++] = curinverter->tnuid[1];
					sendbuff[j++] = curinverter->tnuid[2];
					sendbuff[j++] = curinverter->tnuid[3];
					sendbuff[j++] = curinverter->tnuid[4];
					sendbuff[j++] = curinverter->tnuid[5];
					sendbuff[j++] = 0x42;
					sendbuff[j++] = (int)(curinverter->dvb)/256;
					sendbuff[j++] = (int)(curinverter->dvb)%256;
					sendbuff[j++] = (int)(curinverter->dib)/256;
					sendbuff[j++] = (int)(curinverter->dib)%256;
					sendbuff[j++] = curinverter->opb*100/256;
					sendbuff[j++] = curinverter->opb*100%256;
					sendbuff[j++] = (int)(curinverter->np*10)/256;
					sendbuff[j++] = (int)(curinverter->np*10)%256;
					sendbuff[j++] = curinverter->it;
					sendbuff[j++] = curinverter->nv/256;
					sendbuff[j++] = curinverter->nv%256;
					sendbuff[j++] = curinverter->op*10.0/curinverter->nv;		//交流电流
					if('1' == curinverter->statusb[0])
						sendbuff[j] |= 0x80;
					if('1' == curinverter->statusb[1])
						sendbuff[j] |= 0x40;
					if('1' == curinverter->statusb[2])
						sendbuff[j] |= 0x20;
					if('1' == curinverter->statusb[3])
						sendbuff[j] |= 0x10;
					if('1' == curinverter->statusb[4])
						sendbuff[j] |= 0x08;
					if('1' == curinverter->statusb[5])
						sendbuff[j] |= 0x04;
					if('1' == curinverter->statusb[6])
						sendbuff[j] |= 0x02;
					if('1' == curinverter->statusb[7])
						sendbuff[j] |= 0x01;
					j++;
					if('1' == curinverter->statusb[8])
						sendbuff[j] |= 0x80;
					j++;
					tmp = curinverter->curgenerationb*1000000;
					sendbuff[j++] = tmp/16777216;
					tmp = tmp%16777216;
					sendbuff[j++] = tmp/65536;
					tmp = tmp%65536;
					sendbuff[j++] = tmp/256;
					sendbuff[j++] = tmp%256;

					count += 2;
				}
				else{
					sendbuff[j++] = curinverter->tnuid[0];
					sendbuff[j++] = curinverter->tnuid[1];
					sendbuff[j++] = curinverter->tnuid[2];
					sendbuff[j++] = curinverter->tnuid[3];
					sendbuff[j++] = curinverter->tnuid[4];
					sendbuff[j++] = curinverter->tnuid[5];
					sendbuff[j++] = 0x00;
					sendbuff[j++] = (int)(curinverter->dv)/256;
					sendbuff[j++] = (int)(curinverter->dv)%256;
					sendbuff[j++] = (int)(curinverter->di)/256;
					sendbuff[j++] = (int)(curinverter->di)%256;
					sendbuff[j++] = curinverter->op*100/256;
					sendbuff[j++] = curinverter->op*100%256;
					sendbuff[j++] = (int)(curinverter->np*10)/256;
					sendbuff[j++] = (int)(curinverter->np*10)%256;
					sendbuff[j++] = curinverter->it;
					sendbuff[j++] = curinverter->nv/256;
					sendbuff[j++] = curinverter->nv%256;
					sendbuff[j++] = curinverter->op*10.0/curinverter->nv;		//交流电流
					if('1' == curinverter->status[0])
						sendbuff[j] |= 0x80;
					if('1' == curinverter->status[1])
						sendbuff[j] |= 0x40;
					if('1' == curinverter->status[2])
						sendbuff[j] |= 0x20;
					if('1' == curinverter->status[3])
						sendbuff[j] |= 0x10;
					if('1' == curinverter->status[4])
						sendbuff[j] |= 0x08;
					if('1' == curinverter->status[5])
						sendbuff[j] |= 0x04;
					if('1' == curinverter->status[6])
						sendbuff[j] |= 0x02;
					if('1' == curinverter->status[7])
						sendbuff[j] |= 0x01;
					j++;
					if('1' == curinverter->status[8])
						sendbuff[j] |= 0x80;
					j++;
					tmp = curinverter->curgeneration*1000000;
					sendbuff[j++] = tmp/16777216;
					tmp = tmp%16777216;
					sendbuff[j++] = tmp/65536;
					tmp = tmp%65536;
					sendbuff[j++] = tmp/256;
					sendbuff[j++] = tmp%256;

					count++;
				}
			}
			curinverter++;
		}

		sendbuff[2] = (j-4)/256;
		sendbuff[3] = (j-4)%256;
		add_crc(sendbuff, j);									//加入CRC校验

		sendlength = j+2;										//数据帧长度
	}
}

void serial(void)
{
	FILE *fp;
	char tmp[256];

	fp = fopen("/etc/yuneng/serial.conf", "r");
	fgets(tmp, 256, fp);
	fscanf(fp, "%d", &baudrate);
	fscanf(fp, "%d", &ecuaddress);
	fclose(fp);

	serialfd = open_serial();

	while(1){
		listen_serial();
	}
}

void serial_pthread(void)			//开启按键菜单功能（即开启按键的独立线程）
{
	pthread_t serial_id;
	int serial_ret=0;

	serial_ret=pthread_create(&serial_id,NULL,(void *) serial,NULL);
	if(serial_ret!=0)
		printmsg("Create pthread error");
}

void turn_on_serial(void)
{
	FILE *fp;
	char buff[256]={'\0'};

	fp = fopen("/etc/yuneng/serial.conf", "r");
	fgets(buff, 255, fp);
	fclose(fp);
	if(!strncmp(buff, "on", 2)){
		onflag = 1;
		serial_pthread();
	}
}
