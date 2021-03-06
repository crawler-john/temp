#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"
#include "debug.h"

extern int plcmodem;			//PLC的文件描述符
extern int caltype;			//计算方式，NA版和非NA版的区别
extern unsigned char ccuid[7];		//ECU3501的ID
extern FILE *debugfp;

/*******************获取ECU上3501的ID********************/
int getccuid(int modemfd, unsigned char *ccuid)		//modemfd：串口；ccuid：保存3501的uid
{
	unsigned char sendbuff[512]={'\0'};		//保存发送给3501模块的命令
	unsigned char readbuff[50]={'\0'};		//保存每次3501返回的数据，每次从串口读到一个到多个字节不等
	unsigned short check=0x00;			//保存通信协议中的校验和，根据实际命令计算得出，见说明
	unsigned char validdata[50]={'\0'};		//保存一条从串口中读取的完整的协议串，包括包头、包尾等信息的格式串，把每次readbuff读到的内容拼接起来的串
	char data[50]={'\0'};				//保存从串口独到的有效数据，已删除通信协议的包头、包尾、校验和等信息
	int i, j, recvbytes=0, res=0, validlength=0;	//i、j、res：用于for循环，validlength：有效数据的长度，即控制串口的读操作，每次从串口中读取validlength字节的数据，见协议
	fd_set rd;
	struct timeval timeout;				//用于设置超时时间（10s）
	
	sendbuff[0] = 0xfb;			//数据包头
	sendbuff[1] = 0xfb;			//数据包头
	sendbuff[2] = 0xfa;			//命令字
	sendbuff[3] = 0x00;			//数据长度
	sendbuff[4] = 0x06;			//数据长度
	sendbuff[5] = 0x00;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x01;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0xfe;
	sendbuff[14] = 0xfe;
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	//flush();
	res = write(modemfd, sendbuff, 15);		//通过串口发送sendbuff中的命令

	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);	//如果返回0说明过了超时时间还没有读到数据
		if(res<=0){
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);		//从串口读取数据，每次可能只独到一个到多个字节，并非完整一条协议串
		
			for(i=0; i<res; i++, validlength++)		//拼接成一条完整的协议串
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<26)				//读取3501uid的协议串，共26个字节，如果小于26个字节就继续读，等于26个字节就读完一整串
				continue;
			else if(26 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0xFA == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x11 == validdata[4])&&
						/*(0x00 == validdata[11])&&				//这6个字节随机，不一定是0
						(0x00 == validdata[12])&&
						(0x00 == validdata[13])&&
						(0x00 == validdata[14])&&
						(0x00 == validdata[15])&&
						(0x00 == validdata[16])&&*/
						(0xFA == validdata[17])&&
						(0x02 == validdata[18])&&
						(0x08 == validdata[19])&&
						(0x01 == validdata[20])&&
						(0x00 == validdata[21])&&
						(0xFE == validdata[24])&&
						(0xFE == validdata[25])){
					for(i=5, j=0; j<6; i++, j++)
						ccuid[j] = validdata[i];
						printhexmsg("ccuid", ccuid, 6);
						return 1;
					}
				else
					break;
			}
			else{
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}

	printhexmsg("Cannot get ccuid", validdata, 26);
	
	return -1;
}

/****************************发送配置逆变器的指令***********************************************/
/*int sendconfiginfo(int modemfd, char *ccid, char *tnid)			//终端初始配置
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	int validlength=0;
	unsigned short check=0x00;
	int i, res=0, recvbytes=0;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xfb;			//数据包头
	sendbuff[1] = 0xfb;			//数据包头
	sendbuff[2] = 0x11;			//命令字
	sendbuff[3] = 0x00;			//数据长度
	sendbuff[4] = 0x0d;			//数据长度
	sendbuff[5] = ccid[0];
	sendbuff[6] = ccid[1];
	sendbuff[7] = ccid[2];
	sendbuff[8] = ccid[3];
	sendbuff[9] = ccid[4];
	sendbuff[10] = ccid[5];
	sendbuff[11] = tnid[0];
	sendbuff[12] = tnid[1];
	sendbuff[13] = tnid[2];
	sendbuff[14] = tnid[3];
	sendbuff[15] = tnid[4];
	sendbuff[16] = tnid[5];
	sendbuff[17] = 0x07;
	
	for(i=2; i<18; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[18] = check>>8;
	sendbuff[19] = check;
	sendbuff[20] = 0xfe;
	sendbuff[21] = 0xfe;
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	res = write(modemfd, sendbuff, 22);
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0)
			break;
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<25)
				continue;
			else if(25 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x11 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x10 == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(tnid[0] == validdata[11])&&
						(tnid[1] == validdata[12])&&
						(tnid[2] == validdata[13])&&
						(tnid[3] == validdata[14])&&
						(tnid[4] == validdata[15])&&
						(tnid[5] == validdata[16])&&
						(0x07 == validdata[17])&&
						(0x02 == validdata[18])&&
						(0x01 == validdata[19])&&
						(0x02 == validdata[20])&&
						(0xFE == validdata[23])&&
						(0xFE == validdata[24])){
#ifdef DEBUGINFO
					printf("Config tnuid successfully!\n");
#endif
					return 1;
				}
				else{
#ifdef DEBUGINFO
					printf("Config tnuid failure!\n");
#endif
					break;
				}
			}
			else{
#ifdef DEBUGINFO
					printf("Config tnuid failure!\n");
#endif
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
	
	return -1;
}*/

int clearqueue(int modemfd, char *ccid)	//发送静默包
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check=0x00;
	int i, res;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xfb;			//数据包头
	sendbuff[1] = 0xfb;			//数据包头
	sendbuff[2] = 0x05;			//命令字
	sendbuff[3] = 0x00;			//数据长度
	sendbuff[4] = 0x0D;			//数据长度
	sendbuff[5] = ccid[0];
	sendbuff[6] = ccid[1];
	sendbuff[7] = ccid[2];
	sendbuff[8] = ccid[3];
	sendbuff[9] = ccid[4];
	sendbuff[10] = ccid[5];
	sendbuff[11] = 0x00;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0x00;
	sendbuff[14] = 0x00;
	sendbuff[15] = 0x00;
	sendbuff[16] = 0x00;
	sendbuff[17] = 0x01;
	
	for(i=2; i<18; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[18] = check>>8;
	sendbuff[19] = check;
	sendbuff[20] = 0xfe;
	sendbuff[21] = 0xfe;

	printhexmsg("Clear queue", sendbuff, 22);

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	res = write(modemfd, sendbuff, 22);
	sleep(10);

	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		printmsg("Clear queue timeout");
		return -1;
	}
	else{
		res = read(plcmodem, readbuff, 255);
		if((22 == res) && (0xFB == readbuff[0]) && (0xFB == readbuff[1]) && (0x11 == readbuff[2]) && (0x00 == readbuff[3]) && (0x0D == readbuff[4]) && (0x00 == readbuff[11]) && (0x00 == readbuff[12]) && (0x00 == readbuff[13]) && (0x00 == readbuff[14]) && (0x00 == readbuff[15]) && (0x00 == readbuff[16]) && (0x55 == readbuff[17]) && (0xFE == readbuff[20]) && (0xFE == readbuff[21])){
			printhexmsg("Clear queue successfully", readbuff, 22);
			return 1;
		}
		else{
			printmsg("DATA or length is incorrect");
			return -1;
		}
	}

	return 0;
}

int sendbroadcastask(int modemfd, char *ccid, char *time)	//发送广播命令，告诉所有逆变器ECU将要轮询数据，此命令无任何返回
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;

	clearqueue(modemfd, ccid);
	
	sendbuff[0] = 0xfb;			//数据包头
	sendbuff[1] = 0xfb;			//数据包头
	sendbuff[2] = 0x03;			//命令字
	sendbuff[3] = 0x00;			//数据长度
	sendbuff[4] = 0x12;			//数据长度
	sendbuff[5] = ccid[0];
	sendbuff[6] = ccid[1];
	sendbuff[7] = ccid[2];
	sendbuff[8] = ccid[3];
	sendbuff[9] = ccid[4];
	sendbuff[10] = ccid[5];
	sendbuff[11] = 0x00;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0x00;
	sendbuff[14] = 0x00;
	sendbuff[15] = 0x00;
	sendbuff[16] = 0x00;
	sendbuff[17] = 0x4f;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xaa;
	sendbuff[21] = time[0];
	sendbuff[22] = time[1];
	
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[23] = check>>8;
	sendbuff[24] = check;
	sendbuff[25] = 0xfe;
	sendbuff[26] = 0xfe;
	
	res = write(modemfd, sendbuff, 27);
	
	return 0;
}

int send_clean_gfdi(struct inverter_info_t *inverter)	//发送清除GFDI命令
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i,res;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = inverter->tnuid[0];		//TNID
	sendbuff[12] = inverter->tnuid[1];		//TNID
	sendbuff[13] = inverter->tnuid[2];		//TNID
	sendbuff[14] = inverter->tnuid[3];		//TNID
	sendbuff[15] = inverter->tnuid[4];		//TNID
	sendbuff[16] = inverter->tnuid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xA5;			//CMD
	sendbuff[21] = 0x11;

	for(i=2; i<22; i++)
		check = check + sendbuff[i];
	
	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL
	
	res = write(plcmodem, sendbuff, 26);
	sleep(5);
	
	return 0;
}

/********************************发送取数据指令，逆变器返回数据**********************************/
int sendaskcmd(struct inverter_info_t *inverter, char *time)		//ccid：ECU上3501的UID；tnid：逆变器上3501的UID；cmd：命令；data：保存逆变器返回的有效数据
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
	char data[50] = {'\0'};
	int i, j, res=0;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = inverter->tnuid[0];		//TNID
	sendbuff[12] = inverter->tnuid[1];		//TNID
	sendbuff[13] = inverter->tnuid[2];		//TNID
	sendbuff[14] = inverter->tnuid[3];		//TNID
	sendbuff[15] = inverter->tnuid[4];		//TNID
	sendbuff[16] = inverter->tnuid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xBB;			//CMD
	//if('1' == inverter->flag)
		sendbuff[21] = 0x11;
	//if('0' == inverter->flag)
	//	sendbuff[21] = 0x12;

	for(i=2; i<22; i++)
		check = check + sendbuff[i];
	
	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 26);

	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		inverter->curflag = '0';
		print2msg(inverter->inverterid, "Get data failure");
		return -1;
	}
	else{
		res = 0;
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		if((res == 47) && (readbuff[11] == inverter->tnuid[0]) && (readbuff[12] == inverter->tnuid[1]) && (readbuff[13] == inverter->tnuid[2]) && (readbuff[14] == inverter->tnuid[3]) && (readbuff[15] == inverter->tnuid[4]) && (readbuff[16] == inverter->tnuid[5]) && (0xFE == readbuff[45]) && (0xFE == readbuff[46])){		//是否47个字节，且是否是对应的逆变器
			inverter->flag = '1';
			inverter->curflag = '1';

			printhexmsg(inverter->inverterid, readbuff, 47);

			for(i=0, j=21; i<22; i++, j++){
				data[i] = readbuff[j];
			}

			if((0xBA == readbuff[20]) || (0xBB == readbuff[20]) || (0xBE == readbuff[20]) || (0xBF == readbuff[20])){
				if((data[20]!=time[0]) || (data[21]!=time[1])){
					inverter->curflag = '0';
					return 0;
				}
			}
			else {
				if((0x00 == data[20]) && (0x00 == data[21])){
					inverter->curflag = '0';
					return 0;
				}
			}
			if((data[18]==0) && (data[19]==0)){
				inverter->curflag = '0';
				return 0;
			}
			if(0xB4 == readbuff[20])
				resolvedata_yc250_100(data, inverter);	//解析YC250-100逆变器
			else if(0xBF == readbuff[20])
				resolvedata_yt(data, inverter);		//解析YT200-100型号的逆变器
			else if(0xBE == readbuff[20])
				resolvedata_yt_60(data, inverter);	//解析YT200-60
			else if(0xB3 == readbuff[20])
				resolvedata_yt280_60(data, inverter);	//解析YT280-60逆变器
			else if(0xB5 == readbuff[20])
				resolvedata_yt280_100(data, inverter);	//解析YT280-100逆变器
			else if(0xB8 == readbuff[20])
				resolvedata_b8(data, inverter);
			else if(0x00 == readbuff[20])
			{
				inverter->curflag = '0';
				return 0;
			}
			else if(0xFF == readbuff[20])
			{
				inverter->curflag = '0';
				return 0;
			}
			else
				resolvedata(data, inverter);		//解析非YT型号的逆变器

			sleep(1);
			inverter->flagyc500 = 0;
			backup_data(inverter);
			return 1;
		}
		else if((res == 51) && (readbuff[11] == inverter->tnuid[0]) && (readbuff[12] == inverter->tnuid[1]) && (readbuff[13] == inverter->tnuid[2]) && (readbuff[14] == inverter->tnuid[3]) && (readbuff[15] == inverter->tnuid[4]) && (readbuff[16] == inverter->tnuid[5]) && (0xFE == readbuff[49]) && (0xFE == readbuff[50])){		//YC500
			inverter->flag = '1';
			inverter->curflag = '1';

			printhexmsg(inverter->inverterid, readbuff, 51);

			for(i=0, j=21; i<26; i++, j++){
				data[i] = readbuff[j];
			}

			if((0xBA == readbuff[20]) || (0xBB == readbuff[20]) || (0xBE == readbuff[20]) || (0xBF == readbuff[20])){
				if((data[24]!=time[0]) || (data[25]!=time[1])){
					inverter->curflag = '0';
					return 0;
				}
			}
			else {
				if((0x00 == data[24]) && (0x00 == data[25])){
					inverter->curflag = '0';
					return 0;
				}
			}
			if((data[22]==0) && (data[23]==0)){
				inverter->curflag = '0';
				return 0;
			}
			if(0xB8 == readbuff[20])
				resolvedata_500_b8(data, inverter);
			else if(0xB7 == readbuff[20])
				resolvedata_500_b7(data, inverter);
			else if(0x00 == readbuff[20])
			{
				inverter->curflag = '0';
					return 0;
			}
			else if(0xFF == readbuff[20])
			{
				inverter->curflag = '0';
					return 0;
			}
			else
				resolvedata_500(data, inverter);

			sleep(1);
			inverter->flagyc500 = 1;
			backup_data(inverter);
			return 1;
		}
		else{
			inverter->curflag = '0';
		}
	}
	
	return 0;
}

/*int askthreetime(struct inverter_info_t *inverter)
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
	char data[50] = {'\0'};
	int i, j, k, l, res;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = inverter->tnuid[0];		//TNID
	sendbuff[12] = inverter->tnuid[1];		//TNID
	sendbuff[13] = inverter->tnuid[2];		//TNID
	sendbuff[14] = inverter->tnuid[3];		//TNID
	sendbuff[15] = inverter->tnuid[4];		//TNID
	sendbuff[16] = inverter->tnuid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xBB;			//CMD
	if('1' == inverter->flag)
		sendbuff[21] = 0x11;
	if('0' == inverter->flag)
		sendbuff[21] = 0x12;

	for(i=2; i<22; i++)
		check = check + sendbuff[i];
	
	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 26);

	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		inverter->curflag = '0';
		printf("%s-A: Get data failure!\n", inverter->inverterid);
		return -1;
	}
	else{
		res = 0;
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		if((res == 47) && (readbuff[11] == inverter->tnuid[0]) && (readbuff[12] == inverter->tnuid[1]) && (readbuff[13] == inverter->tnuid[2]) && (readbuff[14] == inverter->tnuid[3]) && (readbuff[15] == inverter->tnuid[4]) && (readbuff[16] == inverter->tnuid[5]) && (0xFE == readbuff[45]) && (0xFE == readbuff[46])){		//是否47个字节，且是否是对应的逆变器
			inverter->flag = '1';
			inverter->curflag = '1';
			printf("%s-A,%x:\n", inverter->inverterid, readbuff[20]);
			for(i=0; i<21; i++)
				printf("%x,", readbuff[i]);
			for(i=0, j=21; i<22; i++, j++){
				data[i] = readbuff[j];printf("%2x,", data[i]);}
			printf("\n");
			resolvedata(data, inverter);
			sleep(1);
			inverter->flagyc500 = 0;
			return 1;
		}
		else if((res == 51) && (readbuff[11] == inverter->tnuid[0]) && (readbuff[12] == inverter->tnuid[1]) && (readbuff[13] == inverter->tnuid[2]) && (readbuff[14] == inverter->tnuid[3]) && (readbuff[15] == inverter->tnuid[4]) && (readbuff[16] == inverter->tnuid[5]) && (0xFE == readbuff[49]) && (0xFE == readbuff[50])){
			inverter->flag = '1';
			inverter->curflag = '1';
			printf("%s-A,%x:\n", inverter->inverterid, readbuff[20]);
			for(i=0; i<21; i++)
				printf("%x,", readbuff[i]);
			for(i=0, j=21; i<26; i++, j++){
				data[i] = readbuff[j];printf("%2x,", data[i]);}
			printf("\n");
			resolvedata_500(data, inverter);
			sleep(1);
			inverter->flagyc500 = 1;
			return 1;
		}
		else{
			inverter->curflag = '0';
		}
	}
	
	return 0;
}*/

/********************发送A/B类报警命令***********************/
/*int sendalamcmd(int modemfd, char *ccid, char *tnid, unsigned char cmd)		//ccid,ECU上3501的ID；tnid,逆变器上3501的ID
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res=0;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x10;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = tnid[0];		//TNID
	sendbuff[12] = tnid[1];		//TNID
	sendbuff[13] = tnid[2];		//TNID
	sendbuff[14] = tnid[3];		//TNID
	sendbuff[15] = tnid[4];		//TNID
	sendbuff[16] = tnid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xD0;			//CMD
	
	for(i=2; i<21; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[21] = check >> 8;		//CHK
	sendbuff[22] = check;		//CHK
	sendbuff[23] = 0xFE;		//TAIL
	sendbuff[24] = 0xFE;		//TAIL
	
	res = write(modemfd, sendbuff, 25);
	
	return res;
}*/

/********************************向ECU上的3501发送自动组网命令************************************/
int sendgridcmd(int modemfd, char *ccid)		//0域名配置命令； ccid：ECU上3501的ID；函数用于自动上报逆变器ID的功能
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	unsigned short check=0x00;
	int i, j, res, error=0, validlength=0, recvbytes=0;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x1B;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x0D;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x01;
	
	for(i=2; i<18; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[18] = check >> 8;		//CHK
	sendbuff[19] = check;		//CHK
	sendbuff[20] = 0xFE;		//TAIL
	sendbuff[21] = 0xFE;		//TAIL
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	write(modemfd, sendbuff, 22);
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printmsg("Send grid command failure");
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<23)
				continue;
			else if(23 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x1B == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x0D == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(0x00 == validdata[11])&&
						(0x00 == validdata[12])&&
						(0x00 == validdata[13])&&
						(0x00 == validdata[14])&&
						(0x00 == validdata[15])&&
						(0x00 == validdata[16])&&
						(0x01 == validdata[17])&&
						(0x00 == validdata[18])&&
						(0xFE == validdata[20])&&
						(0xFE == validdata[21])){
					printmsg("Send grid command successfully");
					return 1;
				}
				else{
					printhexmsg("Grid command formation error", validdata, 23);
					break;
				}
			}
			else{
				printhexmsg("Grid command length error", validdata, 23);
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
		
	printmsg("Send grid command falure");
	
	return 0;
}

/***********************************向所有逆变器发送上报ID的命令*************************************/
int sendrptidcmd(int modemfd, char *ccid)		//上报单个逆变器的ID，函数无返回，用于自动上报逆变器ID的功能
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	unsigned short check=0x00;
	int i, j, res, error=0, validlength=0, recvbytes=0, count=0;
	fd_set rd;
	struct timeval timeout;
	
	printmsg("Allow inverter to report ID");

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x1A;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x0E;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x09;
	sendbuff[18] = 0x00;
	
	for(i=2; i<19; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[19] = check >> 8;		//CHK
	sendbuff[20] = check;		//CHK
	sendbuff[21] = 0xFE;		//TAIL
	sendbuff[22] = 0xFE;		//TAIL
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	
	write(modemfd, sendbuff, 23);
	
	/*while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res==0){
			printf("inverter count: %d\n", count);
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<26)
				continue;
			else if(26 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x11 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x11 == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(0x09 == validdata[17])&&
						(0xFF == validdata[18])&&
						(0xFF == validdata[19])){
					inverter->tnuid[0] = validdata[11];
					inverter->tnuid[1] = validdata[12];
					inverter->tnuid[2] = validdata[13];
					inverter->tnuid[3] = validdata[14];
					inverter->tnuid[4] = validdata[15];
					inverter->tnuid[5] = validdata[16];
					
					if((0x01 == validdata[20]) && (0x02 == validdata[21]))
						inverter->configflag = '1';
					
					inverter++;
					count++;
					
					continue;
				}
				else
					continue;
			}
			else{
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}*/
	
	return 0;
}

int read_tnid(int modemfd, char *ccid, struct inverter_info_t *inverter)		//读取逆变器上报的ID，并保存于inverter的结构体中;函数用于自动上报逆变器ID的功能
{
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	int i, j, res, error=0, validlength=0, recvbytes=0, count=0;
	fd_set rd;
	struct timeval timeout;
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
			
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<26)
				continue;
			else if(26 == recvbytes){
				if((0xFB == validdata[0])&&
					(0xFB == validdata[1])&&
					(0x11 == validdata[2])&&
					(0x00 == validdata[3])&&
					(0x11 == validdata[4])&&
					(ccid[0] == validdata[5])&&
					(ccid[1] == validdata[6])&&
					(ccid[2] == validdata[7])&&
					(ccid[3] == validdata[8])&&
					(ccid[4] == validdata[9])&&
					(ccid[5] == validdata[10])&&
					/*(tnid[0] == validdata[11])&&
					(tnid[1] == validdata[12])&&
					(tnid[2] == validdata[13])&&
					(tnid[3] == validdata[14])&&
					(tnid[4] == validdata[15])&&
					(tnid[5] == validdata[16])&&*/
					(0x09 == validdata[17])&&
					//(0xFF == validdata[18])&&
					//(0xFF == validdata[19])&&
					//(0x01 == validdata[20])&&
					//(0x02 == validdata[21])&&
					(0xFE == validdata[24])&&
					(0xFE == validdata[25])){
					for(i=0, j=11; i<6; i++, j++){
						inverter->tnuid[i] = validdata[j];
					}
					printhexmsg("Tnuid", inverter->tnuid, 6);
					return 1;
				}
				else{
					printhexmsg("Data formation error", validdata, 26);
					break;
				}
			}
			else{
				printhexmsg("Data length error", validdata, 26);
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 60;
		timeout.tv_usec = 0;
	}
	
	return -1;
}

int send_stop_report_cmmd(int modemfd, char *ccid, char * tnid)			//让逆变器停止上报ID，防止逆变器重复上报ID；ccid：ECU3501上的uid；tnid：逆变器上3501的uid;函数用于自动上报逆变器ID的功能
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	unsigned short check=0x00;
	int i, j, res, validlength=0, recvbytes=0;
	fd_set rd;
	struct timeval timeout;
	struct timeval tpstart,tpend;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x11;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x0D;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = tnid[0];		//TNID
	sendbuff[12] = tnid[1];		//TNID
	sendbuff[13] = tnid[2];		//TNID
	sendbuff[14] = tnid[3];		//TNID
	sendbuff[15] = tnid[4];		//TNID
	sendbuff[16] = tnid[5];		//TNID
	sendbuff[17] = 0x08;
	
	for(i=2; i<18; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[18] = check >> 8;		//CHK
	sendbuff[19] = check;		//CHK
	sendbuff[20] = 0xFE;		//TAIL
	sendbuff[21] = 0xFE;		//TAIL

	printhexmsg("Stop command", sendbuff, 22);
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	write(modemfd, sendbuff, 22);
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printdecmsg("Stop report ID failure! res", res);
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<22)
				continue;
			else if(22 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x11 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x0D == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(tnid[0] == validdata[11])&&
						(tnid[1] == validdata[12])&&
						(tnid[2] == validdata[13])&&
						(tnid[3] == validdata[14])&&
						(tnid[4] == validdata[15])&&
						(tnid[5] == validdata[16])&&
						(0x08 == validdata[17])&&
						(0xFE == validdata[20])&&
						(0xFE == validdata[21])){
					printmsg("Stop report ID successfully");
					return 1;
				}
				else{
					printhexmsg("Formation error", validdata, 22);
					break;
				}
			}
			else{
				printmsg("Length error");
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
	
	return -1;
}

int send_allow_report_cmmd(int modemfd, char *ccid, char * tnid)			//让逆变器可以上报ID；ccid：ECU3501上的uid；tnid：逆变器上3501的uid;函数用于自动上报逆变器ID的功能;ZK
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	unsigned short check=0x00;
	int i, j, res, validlength=0, recvbytes=0;
	fd_set rd;
	struct timeval timeout;
	struct timeval tpstart,tpend;
	


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x11;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x0D;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = tnid[0];		//TNID
	sendbuff[12] = tnid[1];		//TNID
	sendbuff[13] = tnid[2];		//TNID
	sendbuff[14] = tnid[3];		//TNID
	sendbuff[15] = tnid[4];		//TNID
	sendbuff[16] = tnid[5];		//TNID
	sendbuff[17] = 0x0A;
	
	for(i=2; i<18; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[18] = check >> 8;		//CHK
	sendbuff[19] = check;		//CHK
	sendbuff[20] = 0xFE;		//TAIL
	sendbuff[21] = 0xFE;		//TAIL

	printhexmsg("stop command", sendbuff, 22);
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	write(modemfd, sendbuff, 22);
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printdecmsg("Allow report ID failure! res", res);
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<22)
				continue;
			else if(22 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x11 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x0D == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(tnid[0] == validdata[11])&&
						(tnid[1] == validdata[12])&&
						(tnid[2] == validdata[13])&&
						(tnid[3] == validdata[14])&&
						(tnid[4] == validdata[15])&&
						(tnid[5] == validdata[16])&&
						(0x0A == validdata[17])&&
						(0xFE == validdata[20])&&
						(0xFE == validdata[21])){
					printmsg("Allow report ID successfully");
					return 1;
				}
				else{
					printhexmsg("Formation error", validdata, 22);
					break;
				}
			}
			else{
				printmsg("Length error");
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
	
	return -1;
}




int test_signal_strength(int modemfd, char *ccid)			//
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	unsigned short check=0x00;
	int i, j, res, validlength=0, recvbytes=0;
	fd_set rd;
	struct timeval timeout;
	struct timeval tpstart,tpend;
	

	sendbuff[0] = 0xFB;			//数据包头
	sendbuff[1] = 0xFB;			//数据包头
	sendbuff[2] = 0xFA;			//命令字
	sendbuff[3] = 0x00;			//数据长度
	sendbuff[4] = 0x06;			//数据长度
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	for(i=2; i<11; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[11] = check >> 8;		//CHK
	sendbuff[12] = check;		//CHK
	sendbuff[13] = 0xFE;		//TAIL
	sendbuff[14] = 0xFE;		//TAIL


	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	write(modemfd, sendbuff, 15);
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printdecmsg("Signal strength test failure! res", res);
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<24)
				continue;
			else if(24 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0xFA == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x0F == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(0x00 == validdata[11])&&
						(0x00 == validdata[12])&&
						(0x00 == validdata[13])&&
						(0x00 == validdata[14])&&
						(0x00 == validdata[15])&&
						(0x00 == validdata[16])&&
						(0xFA == validdata[17])&&
						(0xFE == validdata[22])&&
						(0xFE == validdata[23])){
					printmsg("Signal strength test successfully");
					printdecmsg("Background noise", validdata[18]);
					printdecmsg("Packet signal strength", validdata[19]);
					return 1;
				}
				else{
					printhexmsg("Formation error", validdata, 24);
					break;
				}
			}
			else{
				printmsg("Length error");
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
	
	return -1;
}


int send_0_domain_cmd(int modemfd, char *ccid)		//0域名配置命令
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x1B;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x0D;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x01;
	
	for(i=2; i<18; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[18] = check >> 8;		//CHK
	sendbuff[19] = check;		//CHK
	sendbuff[20] = 0xFE;		//TAIL
	sendbuff[21] = 0xFE;		//TAIL
	
	write(modemfd, sendbuff, 22);
	sleep(10);
	
	return 0;
}

/*int send_allowed_report_cmd(int modemfd, char *ccid)		//允许逆变器上报各自的ID;函数用于自动上报逆变器ID的功能
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x1A;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x0D;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x0A;
	
	for(i=2; i<18; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[18] = check >> 8;		//CHK
	sendbuff[19] = check;		//CHK
	sendbuff[20] = 0xFE;		//TAIL
	sendbuff[21] = 0xFE;		//TAIL
	
	write(modemfd, sendbuff, 22);
	sleep(10);
	
	return 0;
}*/

/***************************************向所有逆变器发送配置的数据****************************************/
int sendconfigdomaincmd(int modemfd, char *ccid, char *tnid)		//ccid,ECU上3501的ID；tnid,逆变器上3501的ID；函数用于配置每个逆变器，每次换了新的逆变器后都需要配置，否则无法通信
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	unsigned short check=0x00;
	int i, j, res, validlength=0, recvbytes=0;
	fd_set rd;
	struct timeval timeout;
	struct timeval tpstart,tpend;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x11;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x0D;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = tnid[0];		//TNID
	sendbuff[12] = tnid[1];		//TNID
	sendbuff[13] = tnid[2];		//TNID
	sendbuff[14] = tnid[3];		//TNID
	sendbuff[15] = tnid[4];		//TNID
	sendbuff[16] = tnid[5];		//TNID
	sendbuff[17] = 0x07;
	
	for(i=2; i<18; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[18] = check >> 8;		//CHK
	sendbuff[19] = check;		//CHK
	sendbuff[20] = 0xFE;		//TAIL
	sendbuff[21] = 0xFE;		//TAIL
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
		
	write(modemfd, sendbuff, 22);
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printdecmsg("Failed to config domain! res", res);
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<25)
				continue;
			else if(25 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x11 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x10 == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(tnid[0] == validdata[11])&&
						(tnid[1] == validdata[12])&&
						(tnid[2] == validdata[13])&&
						(tnid[3] == validdata[14])&&
						(tnid[4] == validdata[15])&&
						(tnid[5] == validdata[16])&&
						(0x07 == validdata[17])&&
						(0x02 == validdata[18])&&
						(0xFE == validdata[23])&&
						(0xFE == validdata[24])){
					//for(i=0, j=21; i<22; i++, j++){
					//	data[i] = validdata[j];
					//}
					return 1;
				}
				else{
					printhexmsg("Formation error", validdata, 25);
					break;
				}
			}
			else{
				printmsg("Length error");
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
	}
	
	return -1;
}

int askpresetdatacmd(struct inverter_info_t *inverter, char *presetdata, char cmd)		//读取逆变器的配置参数，（最终显示在“参数配置”页面上）；presetdata：保存读到的参数值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	unsigned short check=0x00;
	int i, j, res, error=0, validlength=0, recvbytes=0, count=0;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = inverter->tnuid[0];		//TNID
	sendbuff[12] = inverter->tnuid[1];		//TNID
	sendbuff[13] = inverter->tnuid[2];		//TNID
	sendbuff[14] = inverter->tnuid[3];		//TNID
	sendbuff[15] = inverter->tnuid[4];		//TNID
	sendbuff[16] = inverter->tnuid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xDD;
	sendbuff[21] = cmd;
	
	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("sendcmd", sendbuff, 26);
	
	/*FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	write(modemfd, sendbuff, 26);
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
#ifdef DEBUGINFO
			printf("preset data res: %d\n", res);
			printf("Preset data did not return\n", count);
#endif
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<37)
				continue;
			else if(37 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x01 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x20 == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(tnid[0] == validdata[11])&&
						(tnid[1] == validdata[12])&&
						(tnid[2] == validdata[13])&&
						(tnid[3] == validdata[14])&&
						(tnid[4] == validdata[15])&&
						(tnid[5] == validdata[16])&&
						(0x4F == validdata[17])&&
						(0x00 == validdata[18])&&
						(0x00 == validdata[19])&&
						(0xDD == validdata[20])){
					for(i=0, j=21; i<12; i++, j++){
						presetdata[i] = validdata[j];
						printf("%x,", presetdata[i]);
					}
					printf("\n");
					return 1;
				}
				else
					continue;
			}
			else{
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}*/
	
	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	//gettimeofday(&tpstart,NULL);
	write(plcmodem, sendbuff, 26);
	
	while(1){
		res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printdecmsg("Get data failure! res", res);
			break;
		}
		else{
			res=read(plcmodem, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<37)
				continue;
			else if(37 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x01 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x1C == validdata[4])&&
						(ccuid[0] == validdata[5])&&
						(ccuid[1] == validdata[6])&&
						(ccuid[2] == validdata[7])&&
						(ccuid[3] == validdata[8])&&
						(ccuid[4] == validdata[9])&&
						(ccuid[5] == validdata[10])&&
						(inverter->tnuid[0] == validdata[11])&&
						(inverter->tnuid[1] == validdata[12])&&
						(inverter->tnuid[2] == validdata[13])&&
						(inverter->tnuid[3] == validdata[14])&&
						(inverter->tnuid[4] == validdata[15])&&
						(inverter->tnuid[5] == validdata[16])&&
						(0x4F == validdata[17])&&
						(0x00 == validdata[18])&&
						(0x00 == validdata[19])&&
						(0xDD == validdata[20])&&
						(0xFE == validdata[35])&&
						(0xFE == validdata[36])){
					for(i=0, j=21; i<12; i++, j++){
						presetdata[i] = validdata[j];
					}
					printhexmsg(inverter->inverterid, validdata, 37);
					inverter->inverter_with_13_parameters = 1;

					return 1;
					//gettimeofday(&tpend,NULL);
					//if(((tpend.tv_sec - tpstart.tv_sec)+(tpend.tv_usec - tpstart.tv_usec)/1000000)<2.0){
						//usleep(1000000*(tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_usec - tpstart.tv_usec));
						//sleep(1-(tpend.tv_sec - tpstart.tv_sec));
					//}
					//return 1;
				}
				else{
					printhexmsg("Data formation error", validdata, 37);
					break;
				}
			}
			else if(42 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x01 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x21 == validdata[4])&&
						(ccuid[0] == validdata[5])&&
						(ccuid[1] == validdata[6])&&
						(ccuid[2] == validdata[7])&&
						(ccuid[3] == validdata[8])&&
						(ccuid[4] == validdata[9])&&
						(ccuid[5] == validdata[10])&&
						(inverter->tnuid[0] == validdata[11])&&
						(inverter->tnuid[1] == validdata[12])&&
						(inverter->tnuid[2] == validdata[13])&&
						(inverter->tnuid[3] == validdata[14])&&
						(inverter->tnuid[4] == validdata[15])&&
						(inverter->tnuid[5] == validdata[16])&&
						(0x4F == validdata[17])&&
						(0x00 == validdata[18])&&
						(0x00 == validdata[19])&&
						(0xDA == validdata[20])&&
						(0xFE == validdata[40])&&
						(0xFE == validdata[41])){
					for(i=0, j=21; i<17; i++, j++){
						presetdata[i] = validdata[j];
					}
					inverter->inverter_with_13_parameters = 2;
					printhexmsg(inverter->inverterid, validdata, 42);
					return 2;
				}
				else{
					printhexmsg("Data formation error", validdata, 42);
					break;
				}
			}
			else{
				printmsg("Data length error");
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(plcmodem, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
	
	return -1;
}

/*****************向终端发送单点参数设置命令******************/
int sendpresetdata(int modemfd, char *ccid, char *tnid, unsigned char *data)		//ccid,ECU上3501的ID；tnid,逆变器上3501的ID；发送用户在“参数配置”页面上设置的值给逆变器
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned char validdata[256]={'\0'};
	unsigned short check=0x00;
	int i, j, res, error=0, validlength=0, recvbytes=0;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = tnid[0];		//TNID
	sendbuff[12] = tnid[1];		//TNID
	sendbuff[13] = tnid[2];		//TNID
	sendbuff[14] = tnid[3];		//TNID
	sendbuff[15] = tnid[4];		//TNID
	sendbuff[16] = tnid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xCC;			//CMD
	sendbuff[21] = data[0];
	sendbuff[22] = data[1];
	sendbuff[23] = data[2];
	sendbuff[24] = data[3];
	sendbuff[25] = data[4];
	sendbuff[26] = data[5];
	sendbuff[27] = data[6];
	sendbuff[28] = data[7];
	sendbuff[29] = data[8];
	sendbuff[30] = data[9];
	sendbuff[31] = data[10];
	sendbuff[32] = data[11];
	
	for(i=2; i<33; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL
	
	FD_ZERO(&rd);
	FD_SET(modemfd, &rd);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	write(modemfd, sendbuff, 37);

	printhexmsg("SEND PRESET DATA TO ONE", sendbuff, 37);
	
	while(1){
		res = select(modemfd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printmsg("error");
			break;
		}
		else{
			res=read(modemfd, readbuff, 255);
		
			for(i=0; i<res; i++, validlength++)
				validdata[validlength]=readbuff[i];
			memset(readbuff, '\0', 50);
			recvbytes += res;

			if(recvbytes<25)
				continue;
			else if(25 == recvbytes){
				if((0xFB == validdata[0])&&
						(0xFB == validdata[1])&&
						(0x01 == validdata[2])&&
						(0x00 == validdata[3])&&
						(0x10 == validdata[4])&&
						(ccid[0] == validdata[5])&&
						(ccid[1] == validdata[6])&&
						(ccid[2] == validdata[7])&&
						(ccid[3] == validdata[8])&&
						(ccid[4] == validdata[9])&&
						(ccid[5] == validdata[10])&&
						(tnid[0] == validdata[11])&&
						(tnid[1] == validdata[12])&&
						(tnid[2] == validdata[13])&&
						(tnid[3] == validdata[14])&&
						(tnid[4] == validdata[15])&&
						(tnid[5] == validdata[16])&&
						(0x4F == validdata[17])&&
						(0x00 == validdata[18])&&
						(0x00 == validdata[19])&&
						//(0x55 == validdata[20])&&
						(0xFE == validdata[23])&&
						(0xFE == validdata[24])){
					printmsg("Send preset data command successfully");
					if(0x55 == validdata[20])
						return 1;
					else
						return 0;
				}
				else
					break;
			}
			else{
				break;
			}
		}
		FD_ZERO(&rd);
		FD_SET(modemfd, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
	}
	
	return -1;
}

/*****************向所有逆变器发送参数设置命令******************/
int sendpresetdatatoallcmd(int modemfd, char *ccid, unsigned char *data)		//ccid,ECU上3501的ID；tnid,逆变器上3501的ID；data：保存用户在页面上设置的值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccid[0];		//CCID
	sendbuff[6] = ccid[1];		//CCID
	sendbuff[7] = ccid[2];		//CCID
	sendbuff[8] = ccid[3];		//CCID
	sendbuff[9] = ccid[4];		//CCID
	sendbuff[10] = ccid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xCC;			//CMD
	sendbuff[21] = data[0];
	sendbuff[22] = data[1];
	sendbuff[23] = data[2];
	sendbuff[24] = data[3];
	sendbuff[25] = data[4];
	sendbuff[26] = data[5];
	sendbuff[27] = data[6];
	sendbuff[28] = data[7];
	sendbuff[29] = data[8];
	sendbuff[30] = data[9];
	sendbuff[31] = data[10];
	sendbuff[32] = data[11];
	
	for(i=2; i<33; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printhexmsg("SEND PRESET DATA TO ALL", sendbuff, 37);
	
	write(modemfd, sendbuff, 37);
	sleep(10);
	
	return 0;
}

/*****************AFD对齐******************/
int afd_broadcast(void)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xAF;			//CMD
	sendbuff[21] = 0x00;
	sendbuff[22] = 0x00;
	
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("AFD COMMAND", sendbuff, 27);
	
	write(plcmodem, sendbuff, 27);
	sleep(10);
	
	return 0;
}

/*********************单个开机************************/
int wakeup(struct inverter_info_t *inverter)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res=0;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = inverter->tnuid[0];		//TNID
	sendbuff[12] = inverter->tnuid[1];		//TNID
	sendbuff[13] = inverter->tnuid[2];		//TNID
	sendbuff[14] = inverter->tnuid[3];		//TNID
	sendbuff[15] = inverter->tnuid[4];		//TNID
	sendbuff[16] = inverter->tnuid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xA3;			//CMD
	sendbuff[21] = 0x11;
	
	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 26);

	printhexmsg("TURN ON ONE COMMAND", sendbuff, 26);

	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		return -1;
	}
	else{
		res = 0;
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		if((res == 29) && (readbuff[11] == inverter->tnuid[0]) && (readbuff[12] == inverter->tnuid[1]) && (readbuff[13] == inverter->tnuid[2]) && (readbuff[14] == inverter->tnuid[3]) && (readbuff[15] == inverter->tnuid[4]) && (readbuff[16] == inverter->tnuid[5]) && (readbuff[20] == 0xA3) && (0xFE == readbuff[27]) && (0xFE == readbuff[28]))
			return 1;
	}
	
	return -1;
}

/*********************广播开机************************/
int wakeup_broadcast(void)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xA1;			//CMD
	sendbuff[21] = 0x00;
	sendbuff[22] = 0x00;
	
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL
	
	write(plcmodem, sendbuff, 27);

	printhexmsg("TURN ON ALL COMMAND", sendbuff, 27);

	sleep(10);
	
	return 0;
}

/*********************单个关机************************/
int shutdown(struct inverter_info_t *inverter)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res=0;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = inverter->tnuid[0];		//TNID
	sendbuff[12] = inverter->tnuid[1];		//TNID
	sendbuff[13] = inverter->tnuid[2];		//TNID
	sendbuff[14] = inverter->tnuid[3];		//TNID
	sendbuff[15] = inverter->tnuid[4];		//TNID
	sendbuff[16] = inverter->tnuid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xA4;			//CMD
	sendbuff[21] = 0x11;
	
	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 26);

	printhexmsg("TURN OFF ONE COMMAND", sendbuff, 26);

	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		return -1;
	}
	else{
		res = 0;
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		if((res == 29) && (readbuff[11] == inverter->tnuid[0]) && (readbuff[12] == inverter->tnuid[1]) && (readbuff[13] == inverter->tnuid[2]) && (readbuff[14] == inverter->tnuid[3]) && (readbuff[15] == inverter->tnuid[4]) && (readbuff[16] == inverter->tnuid[5]) && (readbuff[20] == 0xA4) && (0xFE == readbuff[27]) && (0xFE == readbuff[28]))
			return 1;
	}
	
	return -1;
}

/*********************广播关机************************/
int shutdown_broadcast(void)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xA2;			//CMD
	sendbuff[21] = 0x00;
	sendbuff[22] = 0x00;
	
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}
	
	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL
	
	write(plcmodem, sendbuff, 27);

	printhexmsg("TURN OFF ALL COMMAND", sendbuff, 27);

	sleep(10);
	
	return 0;
}

