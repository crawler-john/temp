#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "variation.h"
#include "debug.h"
#include "zigbee.h"
#include "sqlite3.h"

extern int zbmodem;

void showtime(int flag)
{
struct timeval tp;

gettimeofday(&tp,NULL);
if(flag==0)
printf("Sendtime-1:%d,%d\n",tp.tv_sec,tp.tv_usec);
if(flag==1)
printf("Receivetime-1:%d,%d\n",tp.tv_sec,tp.tv_usec);
if(flag==2)
printf("Sendtime-2:%d,%d\n",tp.tv_sec,tp.tv_usec);
if(flag==3)
printf("Receivetime-2:%d,%d\n",tp.tv_sec,tp.tv_usec);
if(flag==4)
printf("Sendtime-3:%d,%d\n",tp.tv_sec,tp.tv_usec);
if(flag==5)
printf("Receivetime-3:%d,%d\n",tp.tv_sec,tp.tv_usec);
if(flag==6)
printf("Sendtime-broadcast:%d,%d\n",tp.tv_sec,tp.tv_usec);
if(flag==7)
printf("Receivetime-broadcast:%d,%d\n",tp.tv_sec,tp.tv_usec);
}

int Sendupdatepackage_start(inverter_info *inverter)	//发送单点开始数据包
{
	int ret;
	int i=0,crc=0;
	unsigned char data[65535];
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x01;
	sendbuff[3]=0x80;
	crc=crc_array(&sendbuff[2],68);
	sendbuff[70]=crc/256;
	sendbuff[71]=crc%256;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<10;i++)
	{
		zb_send_cmd(inverter,sendbuff,74);
		printmsg("Sendupdatepackage_start");
		ret = zb_get_reply(data,inverter);
		if((0 == ret%8) && (0x01 == data[2]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))
			return data[3];
	}

	if(i>=10)								//如果发送3遍指令仍然没有返回正确指令，则返回-1
		return -1;
	else
		return 1;

}

int Sendupdatepackage_single(inverter_info *inverter)	//发送单点数据包
{
	int fd;
	int i=0,package_num=0;
	unsigned char package_buff[100];
	unsigned char sendbuff[74]={0xff};
	unsigned short check = 0x00;

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x40;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	if((1 == inverter->model)||(2 == inverter->model))	//YC250机型
		fd=open("/home/UPDATE_YC250.BIN", O_RDONLY);
	else if((3 == inverter->model)||(4 == inverter->model))	//YC500机型
		fd=open("/home/UPDATE_YC500.BIN", O_RDONLY);
	else if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN机型
		fd=open("/home/UPDATE_YC1000.BIN", O_RDONLY);
	else if((7 == inverter->model))		//YC600CN机型
		fd=open("/home/UPDATE_YC600.BIN", O_RDONLY);
	else
		return -1;		//没有该类机型

	if(fd>0)
	{
		while(read(fd,package_buff,64)>0){
			sendbuff[4]=package_num/256;
			sendbuff[5]=package_num%256;
			for(i=0;i<64;i++){
				sendbuff[i+6]=package_buff[i];
			}

			for(i=2; i<70; i++)
				check = check + sendbuff[i];

			printdecmsg("check",check);

			sendbuff[70]=check >> 8;			//校验
			sendbuff[71]=check;

			zb_send_cmd(inverter,sendbuff,74);
			package_num++;
			printdecmsg("package_num",package_num);
			printhexmsg("package_msg", sendbuff, 74);
			memset(package_buff, 0, sizeof(package_buff));
			check = 0x00;
		}
		close(fd);
		return 1;
	}
	else
		return -2;		//打开文件失败
}

int Complementupdatepackage_single(inverter_info *inverter)	//检查漏掉的数据包并补发
{
	int fd;
	int ret;
	int i=0,j=0,k=0;
	unsigned char data[65535];
	unsigned char checkbuff[74]={0x00};
	unsigned char sendbuff[74]={0xff};
	unsigned char package_buff[100];
	unsigned short check = 0x00;

	checkbuff[0]=0xfc;
	checkbuff[1]=0xfc;
	checkbuff[2]=0x04;
	checkbuff[3]=0x20;
	checkbuff[72]=0xfe;
	checkbuff[73]=0xfe;

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x4f;
//	sendbuff[70]=0x00;
//	sendbuff[71]=0x00;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	clear_zbmodem();
	do{
		zb_send_cmd(inverter,checkbuff,74);
		printmsg("Complementupdatepackage_single_checkbuff");
		ret = zb_get_reply(data,inverter);
		printdecmsg("ret",ret);
		i++;
	}while((12!=ret)&&(i<5));		//(-1==ret)

	if((12 == ret) && (0x42 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[10]) && (0xFE == data[11]))
	{

		if((1 == inverter->model)||(2 == inverter->model))	//YC250机型
			fd=open("/home/UPDATE_YC250.BIN", O_RDONLY);
		else if((3 == inverter->model)||(4 == inverter->model))	//YC500机型
			fd=open("/home/UPDATE_YC500.BIN", O_RDONLY);
		else if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN机型
			fd=open("/home/UPDATE_YC1000.BIN", O_RDONLY);
		else if((7 == inverter->model))		//YC600CN机型
			fd=open("/home/UPDATE_YC600.BIN", O_RDONLY);

		if(fd>0){
			while((data[6]*256+data[7])>0)
			{
				lseek(fd,(data[4]*256+data[5])*64,SEEK_SET);
				memset(package_buff, 0, sizeof(package_buff));
				read(fd, package_buff, 64);
				sendbuff[4]=data[4];
				sendbuff[5]=data[5];
				for(k=0;k<64;k++){
					sendbuff[k+6]=package_buff[k];
				}

				for(i=2; i<70; i++)
					check = check + sendbuff[i];

				sendbuff[70]=check >> 8;			//校验
				sendbuff[71]=check;

				for(i=0;i<10;i++)
				{
					zb_send_cmd(inverter,sendbuff,74);
					ret = zb_get_reply(data,inverter);
					if((12 == ret) && (0x24 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[10]) && (0xFE == data[11]))
						break;
				}
				if(i>=10)
				{
					printmsg("Complementupdatepackage single 10 times failed");
					return -1;		//补单包3次没响应的情况
				}
				printdecmsg("Complement_package",(data[6]*256+data[7]));
				usleep(30000);
				check = 0x00;
			}
		close(fd);
		return 1;	//成功补包
		}
	}
	else if((12 == ret) && (0x45 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[10]) && (0xFE == data[11]))//补包如果超过512包就直接还原
	{
		printmsg("Complementupdatepackage over 512");
		return -1;
	}
	else
	{
		printmsg("Complement checkbuff no response");
		return 0;	//发补包check指令没响应的情况
	}

}

int Update_start(inverter_info *inverter)		//发送更新指令
{
	int ret;
	int i=0;
	unsigned char data[65535]={};
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x05;
	sendbuff[3]=0xa0;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,sendbuff,74);
		printmsg("Update_start");
		ret = zb_get_reply_update_start(data,inverter);
		if((8 == ret) && (0x05 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//更新成功
			return 1;
		if((8 == ret) && (0xe5 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//更新失败，还原
			return -1;
	}

	if(i>=3)								//如果发送3遍指令仍然没有返回正确指令，则返回0
		return 0;

}

int Update_success_end(inverter_info *inverter)		//更新成功结束指令
{
	int ret;
	int i=0;
	unsigned char data[65535]={};
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x03;
	sendbuff[3]=0xc0;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,sendbuff,74);
		printmsg("Update_success_end");
		ret = zb_get_reply(data,inverter);
		if((0 == ret%8) && (0x3C == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))
			break;
	}

	if(i>=3)								//如果发送3遍指令仍然没有返回正确指令，则返回-1
		return -1;
	else
	{
		printmsg("Update_success_end successful");
		return 1;
	}

}

int Restore(inverter_info *inverter)		//发送还原指令
{
	int ret;
	int i=0;
	unsigned char data[65535]={};
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x06;
	sendbuff[3]=0x60;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,sendbuff,74);
		printmsg("Restore");
		ret = zb_get_reply_restore(data,inverter);
		if((8 == ret) && (0x06 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//还原成功
			return 1;
		if((8 == ret) && (0xe6 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//还原失败
			return -1;
	}

	if(i>=3)								//如果发送3遍指令仍然没有返回正确指令，则返回0
		return 0;
}

int save_sector(char *id,int sector)
{
	FILE *fp;
	fp=fopen("/home/sector.conf","a");
	fprintf(fp,"%s--%d\n",id,sector);
	fclose(fp);
	return 0;
}


int set_update_new(inverter_info *inverter,int *sector_all)
{
	int K1=1,K6=0,crc=0;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	int i,ret;
	unsigned short check = 0x00;

	FILE *fp=NULL;
	if(inverter->model==7)
	{
		fp=fopen("/home/UPDATE_YC600.BIN","r");
		crc=crc_file("/home/UPDATE_YC600.BIN");
	}
	else if((inverter->model==5)||(inverter->model==6))
	{
		fp=fopen("/home/UPDATE_YC1000.BIN","r");
		crc=crc_file("/home/UPDATE_YC1000.BIN");
	}
	else
	{
		printmsg("Inverter's model Error");
		return -1;
	}
	if(fp==NULL)
	{
		printmsg("No BIN");
		return -1;
	}
	fseek(fp,0,SEEK_END);
	int sector_num=ftell(fp)/4096;
	fclose(fp);
	*sector_all=sector_num;

	fp=fopen("/etc/yuneng/update_working.conf","r");	//1:No working;0:working
	if(fp!=NULL)
	{
		K1=fgetc(fp)-0x30;
		fclose(fp);
	}
	fp=fopen("/etc/yuneng/update_outime.conf","r");		//time_out,小于5min，默认5min
	if(fp!=NULL)
	{
		K6=fgetc(fp)-0x30;
		fclose(fp);
	}


	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x08;
	sendbuff[3] = 0x10;
	sendbuff[4] = K1;
	sendbuff[5] = 0x01;
	sendbuff[6] = 0x00;
	sendbuff[7] = sector_num/256;
	sendbuff[8] = sector_num%256;
	sendbuff[9] = K6;
	sendbuff[10] = crc/256;
	sendbuff[11] = crc%256;

	crc=crc_array(&sendbuff[2],68);
	sendbuff[70]=crc/256;
	sendbuff[71]=crc%256;
	sendbuff[72]=0xFE;
	sendbuff[73]=0xFE;

	for(i=0;i<5;i++)
	{
		memset(readbuff,'\0',256);
		zb_send_cmd(inverter,sendbuff,74);
		printhexmsg("set_update",sendbuff,74);
		ret = zb_get_reply(readbuff,inverter);
		if(ret%13==0)
			break;
	}
	if((0 == ret%13) && (0x06 == readbuff[2]) && (0xFB == readbuff[0]) && (0xFB == readbuff[1]) && (0xFE == readbuff[11]) && (0xFE == readbuff[12]))
	{
		crc=crc_array(&readbuff[2],7);
		if((readbuff[9]==crc/256)&&(readbuff[10]==crc%256))
		{
			return readbuff[5]*256+readbuff[6];
		}

	}
	printmsg("Failed to set_update");
	return -1;
}

int send_package_to_single_new(inverter_info *inverter, int speed,int cur_sector,char *file_name)
{

	int i, fd, package_num=0;
	char sendbuff[256]={'\0'};
	unsigned char package_buff[128];
	unsigned short check = 0x00;
	int package_count;
	int crc;

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x40;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;

	fd = open(file_name, O_RDONLY);

	if(fd>0)
	{
		lseek(fd,cur_sector*4096,SEEK_SET);
		for(package_count=4096/speed;package_count>0;package_count--)
		{
			read(fd,package_buff,speed);
			sendbuff[4]=(package_num+(4096*cur_sector)/speed)/256;
			sendbuff[5]=(package_num+(4096*cur_sector)/speed)%256;

			for(i=0;i<speed;i++){
				sendbuff[i+6]=package_buff[i];
			}
			crc=crc_array(&sendbuff[2],68);
			sendbuff[6+speed]=crc/256;
			sendbuff[7+speed]=crc%256;

			check=0x00;
//			for(i=2; i<(26+speed); i++)
//				check = check + sendbuff[i];
//
//			sendbuff[26+speed] = check >> 8;		//CHK
//			sendbuff[27+speed] = check;		//CHK

			zb_send_cmd(inverter,sendbuff,74);

			package_num++;
			printdecmsg("package_num",package_num);
			printhexmsg("sendbuff", sendbuff, 10+speed);
		}
		close(fd);
		return 0;
	}

	return -1;
}

int zb_get_reply_new(char *data,inverter_info *inverter,int second)			//读取逆变器的返回帧
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int ret, temp_size,size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = second;
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



int send_crc_cmd(inverter_info *inverter,unsigned short crc_result,int file_or_4k,int sector)
{
	int i, fd, ret, mark, resend_packet_flag,crc;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x07;
	sendbuff[3] = 0xE0;
	sendbuff[4] = file_or_4k;
	sendbuff[5] = sector/256;
	sendbuff[6] = sector%256;
	sendbuff[7] = crc_result/256;
	sendbuff[8] = crc_result%256;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;

	crc=crc_array(&sendbuff[2],68);
	sendbuff[70] = crc/256;
	sendbuff[71] = crc%256;
	printhexmsg(inverter->id, sendbuff, 74);
	if(file_or_4k==1)
	{
		for(i=0;i<5;i++)
		{
			sleep(1);
			zb_send_cmd(inverter,sendbuff,74);printhexmsg("***CRC",sendbuff,74);
			if(13 == zb_get_reply_new(readbuff,inverter,90))
				break;
		}
	}
	else
	{
		for(i=0;i<5;i++)
		{
			sleep(1);
			zb_send_cmd(inverter,sendbuff,74);printhexmsg("***CRC",sendbuff,74);
			if(13 == zb_get_reply(readbuff,inverter))
				break;
		}
	}

	if(i>=5)
	{
		printmsg("Send crc_4k over 5 times");	//查询补包数3次没响应的情况
		return -1;
	}
	crc=crc_array(&readbuff[2],7);
	if(
			(readbuff[0]==0xFB)&&
			(readbuff[1]==0xFB)&&
			(readbuff[2]==0x06)&&
			(readbuff[3]==0x7E)&&
			(readbuff[9]==(crc/256))&&
			(readbuff[10]==(crc%256))&&
			(readbuff[11]==0xFE)&&
			(readbuff[12]==0xFE)
	)
		{
			if(readbuff[4]==0xA0)
				return 10;
			else if(readbuff[4]==0xA1)
				return 9;
			else if(readbuff[4]==0xB0)
				return 20;
			else if(readbuff[4]==0xB1)
				return 19;
			else return -1;
		}
	return -1;

}

int crc_4k(char *file,int sector,inverter_info *inverter)
{
	unsigned short result = 0xFFFF;
	char package_buff[128];
	int fd,i;
	fd = open(file, O_RDONLY);
	if(fd>0)
	{
		lseek(fd,4096*sector,SEEK_SET);
		for(i=0;i<4096;i++)
		{
			read(fd, package_buff, 1);
			result = UpdateCRC(result, package_buff[0]);
		}
		close(fd);
	}
	return send_crc_cmd(inverter,result,0,sector);			//0:4k; 1:file_all  return 1代表SUCCESS
}


int resend_lost_packets_new(inverter_info *inverter, int speed,int cur_sector,char *file_name)
{

	int i, fd, ret, mark, resend_packet_flag;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	unsigned short check = 0x00;
	int crc;

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x04;
	sendbuff[3] = 0x20;
	sendbuff[4] = cur_sector/256;
	sendbuff[5] = cur_sector%256;
	crc=crc_array(&sendbuff[2],68);
	sendbuff[70] = crc/256;
	sendbuff[71] = crc%256;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;


	printhexmsg(inverter->id, sendbuff, 74);
	for(i=0;i<5;i++)
	{
		sleep(1);
		zb_send_cmd(inverter,sendbuff,74);printhexmsg("***bubaowenxun",sendbuff,74);
		if(13==zb_get_reply(readbuff,inverter))
			break;
	}

	if(i>=5)
	{
		printmsg("Query the lost packet over 5 times");	//查询补包数5次没响应的情况
		return -1;
	}

	memset(sendbuff,'\0',256);
	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x4F;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;

	crc=crc_array(&readbuff[2],7);printdecmsg("crc",crc);
	if((readbuff[0] == 0xFB) &&
			(readbuff[1] == 0xFB) &&
			(readbuff[2] == 0x06) &&
			(readbuff[3] == 0x42) &&
			(readbuff[9] == (crc/256)) &&
			(readbuff[10] == (crc%256)) &&
			(readbuff[11] == 0xFE) &&
			(readbuff[12] == 0xFE))
	{
		if(readbuff[4] == 0x01)		//扇区码匹配
		{
			if((readbuff[7]*256 + readbuff[8]) == 0)
			{
				return crc_4k(file_name,cur_sector,inverter);		//SUCCESS
//				if(-1==send_update_command(id))
//				{
//					resend_packet_flag = 1;
//					//restore_inverter(inverter);
//					return -1;
//				}
			}
			else if((readbuff[7]*256 + readbuff[8]) > 0)
			{
				printf("%d\n",__LINE__);
				fd = open(file_name, O_RDONLY);

				if(fd>0)
				{
					while((readbuff[7]*256 + readbuff[8]) > 0)
					{printf("%d\n",__LINE__);
						sleep(2);
						lseek(fd,(readbuff[5]*256 + readbuff[6])*speed, SEEK_SET);
						memset(package_buff, 0, sizeof(package_buff));
						read(fd, package_buff, speed);
						sendbuff[4] = readbuff[5];
						sendbuff[5] = readbuff[6];
						for(i=0; i<speed; i++)
						{
							sendbuff[i+6] = package_buff[i];
						}

						crc=crc_array(&sendbuff[2],68);
						sendbuff[6+speed]=crc/256;
						sendbuff[7+speed]=crc%256;

						sendbuff[8+speed] = 0xFE;
						sendbuff[9+speed] = 0xFE;

						for(i=0;i<5;i++)
						{printf("%d\n",__LINE__);
							zb_send_cmd(inverter,sendbuff,74);
							printhexmsg("sendbuff", sendbuff, 10+speed);

							memset(readbuff, 0, sizeof(readbuff));
							mark++;
							printdecmsg("mark=",mark);
							ret = zb_get_reply(readbuff,inverter);
							if((13 == ret) &&
									(readbuff[0] == 0xFB) &&
									(readbuff[1] == 0xFB) &&
									(readbuff[2] == 0x06) &&
									(readbuff[3] == 0x24) &&
									(readbuff[4] == 0x01) &&				//扇区码匹配成功
									(readbuff[11] == 0xFE) &&
									(readbuff[12] == 0xFE))
								break;
						}
						if(i>=5)
						{
							printmsg("debug33333333333333333333333");
							printmsg("Resend the lost packet over 5 times");
							resend_packet_flag=0;
							printdecmsg("resend_packet_flag", resend_packet_flag);
							close(fd);
							return -1;		//补单包5次没响应的情况
						}
						printdecmsg("Resend the lost packet", (readbuff[7]*256 + readbuff[8]));
					}
					resend_packet_flag = 1;
					print2msg(inverter->id, "All of the lost packets have been resent");
					close(fd);
					return crc_4k(file_name,cur_sector,inverter);
//					if(-1==send_update_command(id))
//					{
//						//restore_inverter(inverter);
//						return -1;
//					}

				}
			}

			return 0;
		}
		else if(readbuff[5] == 0x00)
		{
			resend_packet_flag = 1;
//			restore_inverter(id);		//包数丢失过多，还原逆变器程序
			return -1;
		}
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

int crc_bin_file_new(char *file,inverter_info *inverter)
{
	unsigned short result = 0xFFFF;
	char package_buff[128];
	int fd;

	fd = open(file, O_RDONLY);
	if(fd>0)
	{
		while(read(fd, package_buff, 1)>0)
			result = UpdateCRC(result, package_buff[0]);
		close(fd);
	}

	return send_crc_cmd(inverter,result,1,0);
}


int cover_atob(inverter_info *inverter,int source_addr,int to_addr)
{
	int i, fd, ret, mark, resend_packet_flag,crc;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x36;
	sendbuff[3] = 0x60;
	sendbuff[4] = source_addr;
	sendbuff[5] = to_addr;
	for(i=6;i<70;i++)
		sendbuff[i]=i;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;
	crc=crc_array(&sendbuff[2],68);
	sendbuff[70]=crc/256;
	sendbuff[71]=crc%256;
	printhexmsg(inverter->id, sendbuff, 74);
	for(i=0;i<5;i++)
	{
		sleep(10);
		zb_send_cmd(inverter,sendbuff,74);printhexmsg("***Cover",sendbuff,74);
		if(13 <= zb_get_reply_new(readbuff,inverter,90))
			break;
	}
	if(i>=5)
	{
		printmsg("Send Cover_addr over 5 times");	//查询补包数3次没响应的情况
		return -1;
	}
	crc=crc_array(&readbuff[2],7);
	if(
			(readbuff[0]==0xFB)&&
			(readbuff[1]==0xFB)&&
			(readbuff[2]==0x36)&&
			(readbuff[3]==0x66)&&
			(readbuff[9]==(crc/256))&&
			(readbuff[10]==(crc%256))&&
			(readbuff[11]==0xFE)&&
			(readbuff[12]==0xFE)
	)
		{
			if(readbuff[4]==0x00)
				return 0;
			else if(readbuff[4]==0x11)
				return 1;
			else if(readbuff[4]==0x02)
				return 2;
			else if(readbuff[4]==0x10)
				return 3;
			else return -1;
		}
	return -1;

}

int stop_update_new(inverter_info *inverter)
{
	int i, ret=0,crc;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFC;
	sendbuff[1] = 0xFC;
	sendbuff[2] = 0x03;
	sendbuff[3] = 0xC0;
	sendbuff[72] = 0xFE;
	sendbuff[73] = 0xFE;
	crc=crc_array(&sendbuff[2],68);
	sendbuff[70]=crc/256;
	sendbuff[71]=crc%256;


	for(i=0; i<3; i++){
		zb_send_cmd(inverter,sendbuff,74);
		usleep(500000);

		ret = zb_get_reply(readbuff,inverter);

		if((13 == ret) &&
				(readbuff[0] == 0xFB) &&
				(readbuff[1] == 0xFB) &&
				(readbuff[2] == 0x06) &&
				(readbuff[3] == 0x3C) &&
				(readbuff[11] == 0xFE) &&
				(readbuff[12] == 0xFE)
			)
		{
			if(sendbuff[4]==0x00){
				print2msg(inverter->id, "Stop updating successfully");
				return 0;
			}
			else{
				print2msg(inverter->id, "Failed to stop updating");
				return 0;
			}
		}
	}

	print2msg(inverter->id, "Failed to stop updating");
	return -1;
}


/* 升级单台逆变器
 *
 * 返回值：	0 升级成功
 * 			1 发送开始升级数据包失败
 * 		 	2 没有对应型号的逆变器
 * 		 	3 打开升级文件失败
 * 		 	4 补包失败
 * 		 	5 补包无响应
 * 		 	6 更新失败
 * 		 	7 更新无响应
 */
int remote_update_single(inverter_info *inverter)
{
	int ret_sendsingle,ret_complement,ret_update_start=0;
	char flag;
	int i,ret,nxt_sector,j,sector_all;
	int cur_crc=0;
	char update_file_name[100]={'\0'};
	if(inverter->model==7)
		sprintf(update_file_name,"/home/UPDATE_YC600.BIN");
	else if((inverter->model==5)||(inverter->model==6))
		sprintf(update_file_name,"/home/UPDATE_YC1000.BIN");
	Update_success_end(inverter);
	ret=Sendupdatepackage_start(inverter);
	if(32==ret)
	{
		nxt_sector=set_update_new(inverter,&sector_all);
		if(-1!=nxt_sector)
		{
			for(j=nxt_sector;j<sector_all;j++)
			{
				printdecmsg("Sector:",j);
				if(!send_package_to_single_new(inverter,64,j,update_file_name))
				{
					ret=resend_lost_packets_new(inverter, 64,j,update_file_name);
					if(ret==10)
					{
						cur_crc=0;
						continue;
					}
					if((ret==9)&&(cur_crc<3))
					{
						j--;
						cur_crc++;
						continue;
					}
				}
				save_sector(inverter->id,j);
				return -1;
			}
			cur_crc=0;
			ret=crc_bin_file_new(update_file_name,inverter);
			if(ret==19)
			{printf("mmm\n");
				for(j=0;j<sector_all;j++)
				{
					if(10!=crc_4k(update_file_name,j,inverter))
					{

						if(!send_package_to_single_new(inverter,64,j,update_file_name))
						{
							ret=resend_lost_packets_new(inverter, 64,j,update_file_name);printf("ret==%d\n",ret);
							if(ret==10)													//补包成功
							{
								cur_crc=0;
								ret=crc_bin_file_new(update_file_name,inverter);
								if(ret==20)
									break;
								else continue;
							}
							if((ret==9)&&(cur_crc<10))
							{
								j--;
								cur_crc++;
								continue;
							}
							break;
						}
					}

				}
			}
			if(ret==20)
			{
				ret=cover_atob(inverter,2,0);
				if(ret!=0)
					ret=cover_atob(inverter,2,0);
				if(ret!=0)
					ret=cover_atob(inverter,2,0);
				if(ret!=0)
					ret=cover_atob(inverter,2,0);
				if(ret!=0)
					ret=cover_atob(inverter,2,0);
				if(ret!=0)
					ret=cover_atob(inverter,2,0);
				if(ret!=0)
					ret=cover_atob(inverter,2,0);
				if(ret!=0)
					ret=cover_atob(inverter,2,0);
				if(ret!=0)
					ret=cover_atob(inverter,2,0);

				if(ret==0)
				{
					print2msg(inverter->id,"UPDATE SUCCESS!");
				}
				else if(ret==1)
					print2msg(inverter->id,"RELOADING FAILED!");
				else if(ret==2)
					print2msg(inverter->id,"READ_WRITE FAILED!");
				else if(ret==3)
					print2msg(inverter->id,"ADDRESS FAILED!");
				else print2msg(inverter->id,"UPDATE FAILED!");
				if(ret!=0)
					cover_atob(inverter,1,0);
			}
			stop_update_new(inverter);
		}

	}
	else if(24==ret)
	{
		printmsg("Sendupdatepackage_start_OK");
		ret_sendsingle = Sendupdatepackage_single(inverter);

		if(-1==ret_sendsingle)		//没有对应型号的逆变器
		{
			printmsg("No corresponding BIN file");
			return 2;
		}
		else if(-2==ret_sendsingle)		//打开升级文件失败
		{
			Restore(inverter);
			Update_success_end(inverter);
			return 3;
		}
		else if(1==ret_sendsingle)		//文件发送完全的情况
		{
			printmsg("Complementupdatepackage_single");
			ret_complement = Complementupdatepackage_single(inverter); //检查漏掉的数据包并补发

			if(-1==ret_complement)		//补单包超过3次没响应和补包个数直接超过512个的情况
			{
				Restore(inverter);
				Update_success_end(inverter);
				return 4; //补包失败
			}
			else if(1==ret_complement)		//成功补包完全的情况
			{
				for(i=0;i<10;i++)
				{
					ret_update_start=Update_start(inverter); //发送开始更新指令
					if(1==ret_update_start)
						break;
				}
				if(1==ret_update_start)
				{
					printmsg("Update start successful");
					Update_success_end(inverter);
					return 0; //升级成功
				}
				else if(-1==ret_update_start)
				{
					printmsg("Update_start_failed");
					Restore(inverter);
					Update_success_end(inverter);
					return 6; //更新失败
				}
				else if(0==ret_update_start)
				{
					printmsg("Update start no response");
					return 7; //更新无响应
				}
			}
			else if(0==ret_complement)
			{
				return 5; //补包无响应
			}
		}
	}
	else{
		return 1; //发送开始升级数据包失败
	}
}

int remote_update_result(char *id, int version, int update_result)
{
	char inverter_result[128];

	sprintf(inverter_result, "%s%05d%02dEND", id, version, update_result);
	save_inverter_parameters_result2(id, 135, inverter_result);
	return 0;
}

/* 升级逆变器
 *
 * update_result：
 * 0 升级成功
 * 1 发送开始升级数据包失败
 * 2 没有对应型号的逆变器
 * 3 打开升级文件失败
 * 4 补包失败
 * 5 补包无响应
 * 6 更新失败
 * 7 更新无响应
 * 1+ 读取版本号失败
 */
int remote_update(inverter_info *firstinverter)
{
	sqlite3 *db=NULL;
	char *zErrMsg = 0;
	int i=0, nrow = 0, ncolumn = 0, j=0;
	char **azResult;
	char sql[1024]={'\0'};
	int update_result = 0;

	inverter_info *curinverter = firstinverter;

	sqlite3_open("/home/database.db", &db);


	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++)
	{
		sprintf(sql, "SELECT id FROM update_inverter WHERE id='%s' AND update_flag=1", curinverter->id);
		sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		//print2msg("zErrMsg=",zErrMsg);

		if(1==nrow)
		{
			printmsg(curinverter->id);
			if(curinverter->model==7)		//YC600需要关闭半小时以上才能进行升级,与下面的开机对应，凹
			{
				if(curinverter->updating==0)
				{
					if(1 == zb_shutdown_single(curinverter))
					{
						curinverter->updating=1;
						curinverter->updating_time=time(NULL);printdecmsg("shutdown time",curinverter->updating_time);
					}
					continue;
				}
				else
				{printdecmsg("now time",time(NULL));printdecmsg("shutdown time",curinverter->updating_time);
					if((time(NULL)-curinverter->updating_time)<1800)
						continue;
					else curinverter->updating=0;
				}
			}printmsg("sheng ji");
			update_result = remote_update_single(curinverter);
			sprintf(sql,"UPDATE update_inverter SET update_flag=0 WHERE id='%s' ",curinverter->id);
			for(j=0;j<3;j++)
			{
				if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
					break;
				sleep(1);
			}
			for(j=0;j<3;j++)
			{
				if(1 == zb_query_inverter_info(curinverter)) //读取逆变器版本号
				{
					update_inverter_model_version(curinverter);
					break;
				}
			}
			if(j>=3)update_result += 10;
			remote_update_result(curinverter->id, curinverter->version, update_result);

			zb_boot_single(curinverter);		//关机升级完成后，进行开机，与上面对应，凸
		}
		sqlite3_free_table( azResult );

	}

	sqlite3_close( db );

}

