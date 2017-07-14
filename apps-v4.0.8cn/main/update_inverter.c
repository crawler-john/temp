#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "sqlite3.h"
#include "get_reply_from_serial.h"
#include "variation.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID
int last_package=0;

int save_update_inverter_result(char *id, int result)					//更新逆变器的机型码和软件版本号
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;

	memset(sql, '\0', sizeof(sql));
	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sprintf(sql,"UPDATE update_inverter SET update_result=%d WHERE id='%s' ", result, id);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

}

/*发送包给所有逆变器*/
int send_package(char *id)
{
	int i, fd, package_num=0;
	char sendbuff[256]={'\0'};
	unsigned char package_buff[128];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x03;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x93;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x02;
	sendbuff[21] = 0x40;
	sendbuff[154] = 0xFE;
	sendbuff[155] = 0xFE;

//	if((1 == inverter->model)||(2 == inverter->model))	//YC250机型
//		fd=open("/home/UPDATE_YC250.BIN", O_RDONLY);
//	else if((3 == inverter->model)||(4 == inverter->model))	//YC500机型
//		fd=open("/home/UPDATE_YC500.BIN", O_RDONLY);
//	else if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN机型
//		fd=open("/home/UPDATE_YC1000.BIN", O_RDONLY);
//	else
//		return -1;		//没有对应更新包
	fd = open("/home/UPDATE_YC500.BIN", O_RDONLY);

	if(fd>0)
	{
		while(read(fd,package_buff,128)>0)
		{
			sendbuff[22]=package_num/256;
			sendbuff[23]=package_num%256;

			for(i=0;i<128;i++){
				sendbuff[i+24]=package_buff[i];
			}
			check=0x00;
			for(i=2; i<152; i++)
				check = check + sendbuff[i];

			sendbuff[152] = check >> 8;		//CHK
			sendbuff[153] = check;		//CHK

			write(plcmodem, sendbuff, 156);
			sleep(2);

			package_num++;
			printdecmsg("package_num",package_num);
		}
		close(fd);
		return 0;
	}

	return -1;
}

/*发送包给所有逆变器*/
int send_package_to_single_0x18(char *id, int speed,int last_get_time,int last_package)
{
	int i, fd, package_num=0;
	char sendbuff[256]={'\0'};
	unsigned char package_buff[128];
	unsigned short check = 0x00;

	package_num=last_package;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	if(32 == speed)
		sendbuff[4] = 0x33;
	if(64 == speed)
		sendbuff[4] = 0x53;
	if(128 == speed)
		sendbuff[4] = 0x93;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x02;
	sendbuff[21] = 0x40;
	sendbuff[26 + speed] = 0xFE;
	sendbuff[27 + speed] = 0xFE;

	fd = open("/home/UPDATE_YC500.BIN", O_RDONLY);

	if(fd>0)
	{
		lseek(fd, last_package*speed, SEEK_SET);
		while(read(fd,package_buff,speed)>0)
		{
			sendbuff[22]=package_num/256;
			sendbuff[23]=package_num%256;

			for(i=0;i<speed;i++){
				sendbuff[i+24]=package_buff[i];
			}

			check=0x00;
			for(i=2; i<(26+speed); i++)
				check = check + sendbuff[i];

			sendbuff[24+speed] = check >> 8;		//CHK
			sendbuff[25+speed] = check;		//CHK

			write(plcmodem, sendbuff, 28+speed);

			package_num++;
			printdecmsg("package_num",package_num);
			printhexmsg("sendbuff", sendbuff, 28+speed);

			for(i=0;i<2;i++)
			{
				usleep(500);
				if((time(NULL)-last_get_time)>1780){
					printf("\n\nGO GET DATA 30\n");
					return package_num+100000;}
			}

		}
		close(fd);
		return 0;
	}

	return -1;
}

/*发送包给所有逆变器*/
int send_package_to_single_0x19(char *id, int speed,int last_get_time,int last_package)
{
	int i, fd, package_num=0;
	char sendbuff[256]={'\0'};
	unsigned char package_buff[128];
	unsigned short check = 0x00;

	package_num=last_package;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	if(32 == speed)
		sendbuff[4] = 0x35;
	if(64 == speed)
		sendbuff[4] = 0x55;
	if(128 == speed)
		sendbuff[4] = 0x95;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x02;
	sendbuff[21] = 0x40;
	sendbuff[28 + speed] = 0xFE;
	sendbuff[29 + speed] = 0xFE;

	fd = open("/home/UPDATE_YC500.BIN", O_RDONLY);

	if(fd>0)
	{
		lseek(fd, last_package*speed, SEEK_SET);
		while(read(fd,package_buff,speed)>0)
		{
			sendbuff[22]=package_num/256;
			sendbuff[23]=package_num%256;

			for(i=0;i<speed;i++){
				sendbuff[i+24]=package_buff[i];
			}

			check=0x00;
			for(i=0; i<speed; i++)
				check += package_buff[i];
			sendbuff[24+speed] = check >> 8;
			sendbuff[25+speed] = check;

			check=0x00;
			for(i=2; i<(26+speed); i++)
				check = check + sendbuff[i];

			sendbuff[26+speed] = check >> 8;		//CHK
			sendbuff[27+speed] = check;		//CHK

			write(plcmodem, sendbuff, 30+speed);

			package_num++;
			printdecmsg("package_num",package_num);
			printhexmsg("sendbuff", sendbuff, 30+speed);

			for(i=0;i<2;i++)
			{
				usleep(500000);
				if((time(NULL)-last_get_time)>1780){
					printf("\n\nGO GET DATA 30\n");
					return package_num+200000;}
			}

		}
		close(fd);
		return 0;
	}

	return -1;
}

int send_package_to_single_new(char *id, int speed,int cur_sector)
{

	int i, fd, package_num=0;
	char sendbuff[256]={'\0'};
	unsigned char package_buff[128];
	unsigned short check = 0x00;
	int package_count;
	int crc;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	if(32 == speed)
		sendbuff[4] = 0x35;
	if(64 == speed)
		sendbuff[4] = 0x55;
	if(128 == speed)
		sendbuff[4] = 0x95;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x02;
	sendbuff[21] = 0x40;
	sendbuff[28 + speed] = 0xFE;
	sendbuff[29 + speed] = 0xFE;

	fd = open("/home/UPDATE_YC500.BIN", O_RDONLY);

	if(fd>0)
	{
		lseek(fd,cur_sector*4096,SEEK_SET);
		for(package_count=4096/speed;package_count>0;package_count--)
		{
			read(fd,package_buff,speed);
			sendbuff[22]=(package_num+(4096*cur_sector)/speed)/256;
			sendbuff[23]=(package_num+(4096*cur_sector)/speed)%256;

			for(i=0;i<speed;i++){
				sendbuff[i+24]=package_buff[i];
			}
			crc=crc_array(&sendbuff[20],4+speed);
			sendbuff[24+speed]=crc/256;
			sendbuff[25+speed]=crc%256;

			check=0x00;
			for(i=2; i<(26+speed); i++)
				check = check + sendbuff[i];

			sendbuff[26+speed] = check >> 8;		//CHK
			sendbuff[27+speed] = check;		//CHK

			write(plcmodem, sendbuff, 30+speed);
			sleep(1);

			package_num++;
			printdecmsg("package_num",package_num);
			printhexmsg("sendbuff", sendbuff, 30+speed);
		}
		close(fd);
		return 0;
	}

	return -1;
}

/*所有包发送玩后，发送命令通知逆变器更新*/
int send_update_command(char *id)
{
	int i, ret=0;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x33;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x05;
	sendbuff[21] = 0xA0;
	sendbuff[58] = 0xFE;
	sendbuff[59] = 0xFE;

	for(i=2; i<56; i++)
		check = check + sendbuff[i];

	sendbuff[56] = check >> 8;		//CHK
	sendbuff[57] = check;		//CHK

	for(i=0; i<3; i++){
		write(plcmodem, sendbuff, 60);
		usleep(500000);

		ret = get_reply_from_serial(plcmodem, 10, 0, readbuff);

		if((25 == ret) &&
				(readbuff[0] == 0xFB) &&
				(readbuff[1] == 0xFB) &&
				(readbuff[2] == 0x01) &&
				(readbuff[3] == 0x00) &&
				(readbuff[4] == 0x10) &&
				(readbuff[5] == ccuid[0]) &&
				(readbuff[6] == ccuid[1]) &&
				(readbuff[7] == ccuid[2]) &&
				(readbuff[8] == ccuid[3]) &&
				(readbuff[9] == ccuid[4]) &&
				(readbuff[10] == ccuid[5]) &&
				(readbuff[11] == sendbuff[11]) &&
				(readbuff[12] == sendbuff[12]) &&
				(readbuff[13] == sendbuff[13]) &&
				(readbuff[14] == sendbuff[14]) &&
				(readbuff[15] == sendbuff[15]) &&
				(readbuff[16] == sendbuff[16]) &&
				(readbuff[17] == 0x4F) &&
				(readbuff[18] == 0x00) &&
				(readbuff[19] == 0x00) &&
				(readbuff[20] == 0x05) &&
				(readbuff[23] == 0xFE) &&
				(readbuff[24] == 0xFE)
			){
			print2msg(id, "Update successfully");
			return 0;
		}
	}

	print2msg(id, "Failed to update");
	return -1;
}

/*补发丢失的包*/
int resend_lost_packets_0x18(char *id, int speed)
{
	int i, fd, ret, mark, resend_packet_flag;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x33;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x04;
	sendbuff[21] = 0x20;
	sendbuff[58] = 0xFE;
	sendbuff[59] = 0xFE;

	for(i=2; i<56; i++)
		check = check + sendbuff[i];

	sendbuff[56] = check >> 8;		//CHK
	sendbuff[57] = check;		//CHK

	printhexmsg(id, sendbuff, 60);
	for(i=0;i<5;i++)
	{
		sleep(5);
		write(plcmodem, sendbuff, 60);
		if(29 == get_reply_from_serial(plcmodem, 10, 0, readbuff))
			break;
	}

	if(i>=5)
	{
		printmsg("Query the lost packet over 5 times");	//查询补包数3次没响应的情况
		return 2;
	}

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	if(32 == speed)
		sendbuff[4] = 0x33;
	if(64 == speed)
		sendbuff[4] = 0x53;
	if(128 == speed)
		sendbuff[4] = 0x93;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x02;
	sendbuff[21] = 0x4F;
	sendbuff[26+speed] = 0xFE;
	sendbuff[27+speed] = 0xFE;

	if((readbuff[0] == 0xFB) &&
			(readbuff[1] == 0xFB) &&
			(readbuff[2] == 0x01) &&
			(readbuff[3] == 0x00) &&
			(readbuff[4] == 0x14) &&
			(readbuff[5] == ccuid[0]) &&
			(readbuff[6] == ccuid[1]) &&
			(readbuff[7] == ccuid[2]) &&
			(readbuff[8] == ccuid[3]) &&
			(readbuff[9] == ccuid[4]) &&
			(readbuff[10] == ccuid[5]) &&
			(readbuff[11] == sendbuff[11]) &&
			(readbuff[12] == sendbuff[12]) &&
			(readbuff[13] == sendbuff[13]) &&
			(readbuff[14] == sendbuff[14]) &&
			(readbuff[15] == sendbuff[15]) &&
			(readbuff[16] == sendbuff[16]) &&
			(readbuff[17] == 0x4F) &&
			(readbuff[18] == 0x00) &&
			(readbuff[19] == 0x00) &&
			//(readbuff[20] == 0x42) &&
			(readbuff[27] == 0xFE) &&
			(readbuff[28] == 0xFE))
	{
		if(readbuff[20] == 0x42)
		{
			if((readbuff[23]*256 + readbuff[24]) == 0)
			{
				if(-1==send_update_command(id))
				{
					resend_packet_flag = 1;
					//restore_inverter(inverter);
					return 4;
				}
			}
			else if((readbuff[23]*256 + readbuff[24]) > 0)
			{
				fd = open("/home/UPDATE_YC500.BIN", O_RDONLY);

				if(fd>0)
				{
					while((readbuff[23]*256 + readbuff[24]) > 0)
					{
						sleep(2);
						lseek(fd, (readbuff[21]*256 + readbuff[22])*speed, SEEK_SET);
						memset(package_buff, 0, sizeof(package_buff));
						read(fd, package_buff, speed);
						sendbuff[20] = 0x02;
						sendbuff[21] = 0x4F;
						sendbuff[22] = readbuff[21];
						sendbuff[23] = readbuff[22];
						for(i=0; i<speed; i++)
						{
							sendbuff[i+24] = package_buff[i];
						}

						check=0x00;
						for(i=2; i<(24+speed); i++)
							check = check + sendbuff[i];

						sendbuff[24+speed] = check >> 8;		//CHK
						sendbuff[25+speed] = check;		//CHK
						sendbuff[26+speed] = 0xFE;
						sendbuff[27+speed] = 0xFE;

						for(i=0;i<5;i++)
						{
							write(plcmodem, sendbuff, 28+speed);
							printhexmsg("sendbuff", sendbuff, 28+speed);

							memset(readbuff, 0, sizeof(readbuff));
							mark++;
							printdecmsg("mark=",mark);
							ret = get_reply_from_serial(plcmodem, 5, 0, readbuff);
							if((29 == ret) &&
									(readbuff[0] == 0xFB) &&
									(readbuff[1] == 0xFB) &&
									(readbuff[2] == 0x01) &&
									(readbuff[3] == 0x00) &&
									(readbuff[4] == 0x14) &&
									(readbuff[5] == ccuid[0]) &&
									(readbuff[6] == ccuid[1]) &&
									(readbuff[7] == ccuid[2]) &&
									(readbuff[8] == ccuid[3]) &&
									(readbuff[9] == ccuid[4]) &&
									(readbuff[10] == ccuid[5]) &&
									(readbuff[11] == sendbuff[11]) &&
									(readbuff[12] == sendbuff[12]) &&
									(readbuff[13] == sendbuff[13]) &&
									(readbuff[14] == sendbuff[14]) &&
									(readbuff[15] == sendbuff[15]) &&
									(readbuff[16] == sendbuff[16]) &&
									(readbuff[17] == 0x4F) &&
									(readbuff[18] == 0x00) &&
									(readbuff[19] == 0x00) &&
									(readbuff[20] == 0x24) &&
									(readbuff[27] == 0xFE) &&
									(readbuff[28] == 0xFE))
								break;
						}
						if(i>=5)
						{
							printmsg("debug33333333333333333333333");
							printmsg("Resend the lost packet over 5 times");
							resend_packet_flag=0;
							printdecmsg("resend_packet_flag", resend_packet_flag);
							close(fd);
							return 3;		//补单包5次没响应的情况
						}
						printdecmsg("Resend the lost packet", (readbuff[23]*256 + readbuff[24]));
					}
					resend_packet_flag = 1;
					print2msg(id, "All of the lost packets have been resent");
					close(fd);
					if(-1==send_update_command(id))
					{
						//restore_inverter(inverter);
						return 4;
					}

				}
			}

			return 0;
		}
		else if(readbuff[20] == 0x45)
		{
			resend_packet_flag = 1;
			restore_inverter(id);		//包数丢失过多，还原逆变器程序
			return 2;
		}
		else
			return 2;
	}
	else
	{
		return 2;
	}
}

/*补发丢失的包*/
int resend_lost_packets_0x19(char *id, int speed)
{
	int i, fd, ret, mark, resend_packet_flag;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x33;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x04;
	sendbuff[21] = 0x20;
	sendbuff[58] = 0xFE;
	sendbuff[59] = 0xFE;

	for(i=2; i<56; i++)
		check = check + sendbuff[i];

	sendbuff[56] = check >> 8;		//CHK
	sendbuff[57] = check;		//CHK

	printhexmsg(id, sendbuff, 60);
	for(i=0;i<5;i++)
	{
		sleep(5);
		write(plcmodem, sendbuff, 60);
		if(29 == get_reply_from_serial(plcmodem, 10, 0, readbuff))
			break;
	}

	if(i>=5)
	{
		printmsg("Query the lost packet over 5 times");	//查询补包数3次没响应的情况
		return 2;
	}

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	if(32 == speed)
		sendbuff[4] = 0x35;
	if(64 == speed)
		sendbuff[4] = 0x55;
	if(128 == speed)
		sendbuff[4] = 0x95;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x02;
	sendbuff[21] = 0x4F;
	sendbuff[28+speed] = 0xFE;
	sendbuff[29+speed] = 0xFE;

	if((readbuff[0] == 0xFB) &&
			(readbuff[1] == 0xFB) &&
			(readbuff[2] == 0x01) &&
			(readbuff[3] == 0x00) &&
			(readbuff[4] == 0x14) &&
			(readbuff[5] == ccuid[0]) &&
			(readbuff[6] == ccuid[1]) &&
			(readbuff[7] == ccuid[2]) &&
			(readbuff[8] == ccuid[3]) &&
			(readbuff[9] == ccuid[4]) &&
			(readbuff[10] == ccuid[5]) &&
			(readbuff[11] == sendbuff[11]) &&
			(readbuff[12] == sendbuff[12]) &&
			(readbuff[13] == sendbuff[13]) &&
			(readbuff[14] == sendbuff[14]) &&
			(readbuff[15] == sendbuff[15]) &&
			(readbuff[16] == sendbuff[16]) &&
			(readbuff[17] == 0x4F) &&
			(readbuff[18] == 0x00) &&
			(readbuff[19] == 0x00) &&
			//(readbuff[20] == 0x42) &&
			(readbuff[27] == 0xFE) &&
			(readbuff[28] == 0xFE))
	{
		if(readbuff[20] == 0x42)
		{
			if((readbuff[23]*256 + readbuff[24]) == 0)
			{
				if(-1==send_update_command(id))
				{
					resend_packet_flag = 1;
					//restore_inverter(inverter);
					return 4;
				}
			}
			else if((readbuff[23]*256 + readbuff[24]) > 0)
			{
				fd = open("/home/UPDATE_YC500.BIN", O_RDONLY);

				if(fd>0)
				{
					while((readbuff[23]*256 + readbuff[24]) > 0)
					{
						sleep(2);
						lseek(fd, (readbuff[21]*256 + readbuff[22])*speed, SEEK_SET);
						memset(package_buff, 0, sizeof(package_buff));
						read(fd, package_buff, speed);
						sendbuff[20] = 0x02;
						sendbuff[21] = 0x4F;
						sendbuff[22] = readbuff[21];
						sendbuff[23] = readbuff[22];
						for(i=0; i<speed; i++)
						{
							sendbuff[i+24] = package_buff[i];
						}

						check=0x00;
						for(i=0; i<speed; i++)
							check += package_buff[i];
						sendbuff[24+speed] = check >> 8;		//CHK
						sendbuff[25+speed] = check;		//CHK

						check=0x00;
						for(i=2; i<(26+speed); i++)
							check = check + sendbuff[i];

						sendbuff[26+speed] = check >> 8;		//CHK
						sendbuff[27+speed] = check;		//CHK
						sendbuff[28+speed] = 0xFE;
						sendbuff[29+speed] = 0xFE;

						for(i=0;i<5;i++)
						{
							write(plcmodem, sendbuff, 30+speed);
							printhexmsg("sendbuff", sendbuff, 30+speed);

							memset(readbuff, 0, sizeof(readbuff));
							mark++;
							printdecmsg("mark=",mark);
							ret = get_reply_from_serial(plcmodem, 5, 0, readbuff);
							if((29 == ret) &&
									(readbuff[0] == 0xFB) &&
									(readbuff[1] == 0xFB) &&
									(readbuff[2] == 0x01) &&
									(readbuff[3] == 0x00) &&
									(readbuff[4] == 0x14) &&
									(readbuff[5] == ccuid[0]) &&
									(readbuff[6] == ccuid[1]) &&
									(readbuff[7] == ccuid[2]) &&
									(readbuff[8] == ccuid[3]) &&
									(readbuff[9] == ccuid[4]) &&
									(readbuff[10] == ccuid[5]) &&
									(readbuff[11] == sendbuff[11]) &&
									(readbuff[12] == sendbuff[12]) &&
									(readbuff[13] == sendbuff[13]) &&
									(readbuff[14] == sendbuff[14]) &&
									(readbuff[15] == sendbuff[15]) &&
									(readbuff[16] == sendbuff[16]) &&
									(readbuff[17] == 0x4F) &&
									(readbuff[18] == 0x00) &&
									(readbuff[19] == 0x00) &&
									(readbuff[20] == 0x24) &&
									(readbuff[27] == 0xFE) &&
									(readbuff[28] == 0xFE))
								break;
						}
						if(i>=5)
						{
							printmsg("debug33333333333333333333333");
							printmsg("Resend the lost packet over 5 times");
							resend_packet_flag=0;
							printdecmsg("resend_packet_flag", resend_packet_flag);
							close(fd);
							return 3;		//补单包5次没响应的情况
						}
						printdecmsg("Resend the lost packet", (readbuff[23]*256 + readbuff[24]));
					}
					resend_packet_flag = 1;
					print2msg(id, "All of the lost packets have been resent");
					close(fd);
					if(-1==send_update_command(id))
					{
						//restore_inverter(inverter);
						return 4;
					}

				}
			}

			return 0;
		}
		else if(readbuff[20] == 0x45)
		{
			resend_packet_flag = 1;
			restore_inverter(id);		//包数丢失过多，还原逆变器程序
			return 2;
		}
		else
			return 2;
	}
	else
	{
		return 2;
	}
}

//返回值10，20：成功（4k，file） 9，19：失败；-1没回复，失败
int send_crc_cmd(char *id,unsigned short crc_result,int file_or_4k,int sector)
{
	int i, fd, ret, mark, resend_packet_flag;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x35;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x07;
	sendbuff[21] = 0xE0;
	sendbuff[22] = file_or_4k;
	sendbuff[23] = sector/256;
	sendbuff[24] = sector%256;
	sendbuff[25] = crc_result/256;
	sendbuff[26] = crc_result%256;
	sendbuff[60] = 0xFE;
	sendbuff[61] = 0xFE;

	for(i=2; i<58; i++)
		check = check + sendbuff[i];

	sendbuff[58] = check >> 8;		//CHK
	sendbuff[59] = check;		//CHK
	printhexmsg(id, sendbuff, 62);
	for(i=0;i<5;i++)
	{
		sleep(1);
		write(plcmodem, sendbuff, 62);printhexmsg("***CRC",sendbuff,62);
		if(32 == get_reply_from_serial(plcmodem, 10, 0, readbuff))
			break;
	}
	if(i>=5)
	{
		printmsg("Send crc_4k over 5 times");	//查询补包数3次没响应的情况
		return -1;
	}
	if(
			(readbuff[0]==0xFB)&&
			(readbuff[1]==0xFB)&&
			(readbuff[2]==0x01)&&
			(readbuff[3]==0x00)&&
			(readbuff[4]==0x17)&&
			(readbuff[5]==sendbuff[5])&&
			(readbuff[6]==sendbuff[6])&&
			(readbuff[7]==sendbuff[7])&&
			(readbuff[8]==sendbuff[8])&&
			(readbuff[9]==sendbuff[9])&&
			(readbuff[10]==sendbuff[10])&&
			(readbuff[11]==sendbuff[11])&&
			(readbuff[12]==sendbuff[12])&&
			(readbuff[13]==sendbuff[13])&&
			(readbuff[14]==sendbuff[14])&&
			(readbuff[15]==sendbuff[15])&&
			(readbuff[16]==sendbuff[16])&&
			(readbuff[17]==0x4F)&&
			(readbuff[18]==0x00)&&
			(readbuff[19]==0x00)&&
			(readbuff[20]==0x7E)&&
			(readbuff[30]==0xFE)&&
			(readbuff[31]==0xFE)
	)
		{
			if(readbuff[21]==0xA0)
				return 10;
			else if(readbuff[21]==0xA1)
				return 9;
			else if(readbuff[21]==0xB0)
				return 20;
			else if(readbuff[21]==0xB1)
				return 19;
			else return -1;
		}
	return -1;

}
int crc_4k(char *file,int sector,char *id)
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
	return send_crc_cmd(id,result,0,sector);			//0:4k; 1:file_all  return 1代表SUCCESS
}
int crc_bin_file_new(char *file,char *id)
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

	return send_crc_cmd(id,result,1,0);
}
int cover_atob(char *id,int source_addr,int to_addr)
{
	int i, fd, ret, mark, resend_packet_flag,crc;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x35;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x06;
	sendbuff[21] = 0x60;
	sendbuff[22] = source_addr;
	sendbuff[23] = to_addr;
	sendbuff[60] = 0xFE;
	sendbuff[61] = 0xFE;
	crc=crc_array(&sendbuff[20],36);
	sendbuff[56]=crc/256;
	sendbuff[57]=crc%256;
	for(i=2; i<58; i++)
		check = check + sendbuff[i];

	sendbuff[58] = check >> 8;		//CHK
	sendbuff[59] = check;		//CHK
	printhexmsg(id, sendbuff, 62);
	for(i=0;i<5;i++)
	{
		sleep(1);
		write(plcmodem, sendbuff, 62);printhexmsg("***Cover",sendbuff,62);
		if(32 == get_reply_from_serial(plcmodem, 60, 0, readbuff))
			break;
	}
	if(i>=5)
	{
		printmsg("Send Cover_addr over 5 times");	//查询补包数3次没响应的情况
		return -1;
	}
	if(
			(readbuff[0]==0xFB)&&
			(readbuff[1]==0xFB)&&
			(readbuff[2]==0x01)&&
			(readbuff[3]==0x00)&&
			(readbuff[4]==0x17)&&
			(readbuff[5]==sendbuff[5])&&
			(readbuff[6]==sendbuff[6])&&
			(readbuff[7]==sendbuff[7])&&
			(readbuff[8]==sendbuff[8])&&
			(readbuff[9]==sendbuff[9])&&
			(readbuff[10]==sendbuff[10])&&
			(readbuff[11]==sendbuff[11])&&
			(readbuff[12]==sendbuff[12])&&
			(readbuff[13]==sendbuff[13])&&
			(readbuff[14]==sendbuff[14])&&
			(readbuff[15]==sendbuff[15])&&
			(readbuff[16]==sendbuff[16])&&
			(readbuff[17]==0x4F)&&
			(readbuff[18]==0x00)&&
			(readbuff[19]==0x00)&&
			(readbuff[20]==0x66)&&
			(readbuff[30]==0xFE)&&
			(readbuff[31]==0xFE)
	)
		{
			if(readbuff[21]==0x00)
				return 0;
			else if(readbuff[21]==0x11)
				return 1;
			else if(readbuff[21]==0x02)
				return 2;
			else if(readbuff[21]==0x10)
				return 3;
			else return -1;
		}
	return -1;

}

int resend_lost_packets_new(char *id, int speed, int cur_sector)
{

	int i, fd, ret, mark, resend_packet_flag;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned char package_buff[129];
	unsigned short check = 0x00;
	int crc;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x35;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x04;
	sendbuff[21] = 0x20;
	sendbuff[22] = cur_sector/256;
	sendbuff[23] = cur_sector%256;
	sendbuff[60] = 0xFE;
	sendbuff[61] = 0xFE;

	for(i=2; i<58; i++)
		check = check + sendbuff[i];

	sendbuff[58] = check >> 8;		//CHK
	sendbuff[59] = check;		//CHK

	printhexmsg(id, sendbuff, 62);
	for(i=0;i<5;i++)
	{
		sleep(5);
		write(plcmodem, sendbuff, 62);printhexmsg("***bubaowenxun",sendbuff,62);
		if(32 == get_reply_from_serial(plcmodem, 10, 0, readbuff))
			break;
	}

	if(i>=5)
	{
		printmsg("Query the lost packet over 5 times");	//查询补包数3次没响应的情况
		return -1;
	}

	memset(sendbuff,'\0',256);
	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	if(32 == speed)
		sendbuff[4] = 0x35;
	if(64 == speed)
		sendbuff[4] = 0x55;
	if(128 == speed)
		sendbuff[4] = 0x95;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x02;
	sendbuff[21] = 0x4F;
	sendbuff[28+speed] = 0xFE;
	sendbuff[29+speed] = 0xFE;

	if((readbuff[0] == 0xFB) &&
			(readbuff[1] == 0xFB) &&
			(readbuff[2] == 0x01) &&
			(readbuff[3] == 0x00) &&
			(readbuff[4] == 0x17) &&
			(readbuff[5] == ccuid[0]) &&
			(readbuff[6] == ccuid[1]) &&
			(readbuff[7] == ccuid[2]) &&
			(readbuff[8] == ccuid[3]) &&
			(readbuff[9] == ccuid[4]) &&
			(readbuff[10] == ccuid[5]) &&
			(readbuff[11] == sendbuff[11]) &&
			(readbuff[12] == sendbuff[12]) &&
			(readbuff[13] == sendbuff[13]) &&
			(readbuff[14] == sendbuff[14]) &&
			(readbuff[15] == sendbuff[15]) &&
			(readbuff[16] == sendbuff[16]) &&
			(readbuff[17] == 0x4F) &&
			(readbuff[18] == 0x00) &&
			(readbuff[19] == 0x00) &&
			(readbuff[20] == 0x42) &&
			(readbuff[30] == 0xFE) &&
			(readbuff[31] == 0xFE))
	{
		if(readbuff[21] == 0x01)		//扇区码匹配
		{
			if((readbuff[24]*256 + readbuff[25]) == 0)
			{
				return crc_4k("/home/UPDATE_YC500.BIN",cur_sector,id);		//SUCCESS
//				if(-1==send_update_command(id))
//				{
//					resend_packet_flag = 1;
//					//restore_inverter(inverter);
//					return -1;
//				}
			}
			else if((readbuff[24]*256 + readbuff[25]) > 0)
			{
				fd = open("/home/UPDATE_YC500.BIN", O_RDONLY);

				if(fd>0)
				{
					while((readbuff[24]*256 + readbuff[25]) > 0)
					{
						sleep(2);
						lseek(fd,(readbuff[22]*256 + readbuff[23])*speed, SEEK_SET);
						memset(package_buff, 0, sizeof(package_buff));
						read(fd, package_buff, speed);
						sendbuff[20] = 0x02;
						sendbuff[21] = 0x4F;
						sendbuff[22] = readbuff[22];
						sendbuff[23] = readbuff[23];
						for(i=0; i<speed; i++)
						{
							sendbuff[i+24] = package_buff[i];
						}

						crc=crc_array(&sendbuff[20],4+speed);
						sendbuff[24+speed]=crc/256;
						sendbuff[25+speed]=crc%256;

						check = 0;
						for(i=2; i<(26+speed); i++)
							check = check + sendbuff[i];

						sendbuff[26+speed] = check >> 8;		//CHK
						sendbuff[27+speed] = check;		//CHK
						sendbuff[28+speed] = 0xFE;
						sendbuff[29+speed] = 0xFE;

						for(i=0;i<5;i++)
						{
							write(plcmodem, sendbuff, 30+speed);
							printhexmsg("sendbuff", sendbuff, 30+speed);

							memset(readbuff, 0, sizeof(readbuff));
							mark++;
							printdecmsg("mark=",mark);
							ret = get_reply_from_serial(plcmodem, 5, 0, readbuff);
							if((32 == ret) &&
									(readbuff[0] == 0xFB) &&
									(readbuff[1] == 0xFB) &&
									(readbuff[2] == 0x01) &&
									(readbuff[3] == 0x00) &&
									(readbuff[4] == 0x17) &&
									(readbuff[5] == ccuid[0]) &&
									(readbuff[6] == ccuid[1]) &&
									(readbuff[7] == ccuid[2]) &&
									(readbuff[8] == ccuid[3]) &&
									(readbuff[9] == ccuid[4]) &&
									(readbuff[10] == ccuid[5]) &&
									(readbuff[11] == sendbuff[11]) &&
									(readbuff[12] == sendbuff[12]) &&
									(readbuff[13] == sendbuff[13]) &&
									(readbuff[14] == sendbuff[14]) &&
									(readbuff[15] == sendbuff[15]) &&
									(readbuff[16] == sendbuff[16]) &&
									(readbuff[17] == 0x4F) &&
									(readbuff[18] == 0x00) &&
									(readbuff[19] == 0x00) &&
									(readbuff[20] == 0x24) &&
									(readbuff[21] == 0x01) &&				//扇区码匹配成功
									(readbuff[30] == 0xFE) &&
									(readbuff[31] == 0xFE))
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
						printdecmsg("Resend the lost packet", (readbuff[24]*256 + readbuff[25]));
					}
					resend_packet_flag = 1;
					print2msg(id, "All of the lost packets have been resent");
					close(fd);
					return crc_4k("/home/UPDATE_YC500.BIN",cur_sector,id);
//					if(-1==send_update_command(id))
//					{
//						//restore_inverter(inverter);
//						return -1;
//					}

				}
			}

			return 0;
		}
		else if(readbuff[21] == 0x00)
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

int set_update_flag_two(char *id)
{
	sqlite3 *db=NULL;
	char sql[1024];

	sprintf(sql, "UPDATE update_inverter SET update_flag=2 WHERE id='%s'", id);

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int update_flag_minus_one(char *id)
{
	sqlite3 *db=NULL;
	char sql[1024];

	sprintf(sql, "UPDATE update_inverter SET update_flag=update_flag-1 WHERE id='%s'", id);

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}


int clear_update_flag(char *id)
{
	sqlite3 *db=NULL;
	char sql[1024];

	sprintf(sql, "UPDATE update_inverter SET update_flag=0 WHERE id='%s'", id);

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

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

int resend_lost_packets(char *id, int speed, unsigned char update_flag)
{
	int ret=4;
	if(0x18 == update_flag)
		ret=resend_lost_packets_0x18(id, speed);
	if(0x19 == update_flag)
		ret=resend_lost_packets_0x19(id, speed);
	return ret;
}

int send_package_to_single_again(char *id, int speed, int last_get_time,int last_pack_num)
{
	int ret = 0;
	if(last_pack_num>200000)
		ret=send_package_to_single_0x19(id, speed,last_get_time,(last_pack_num-200000));
	else if(last_pack_num>100000)
		ret=send_package_to_single_0x18(id, speed,last_get_time,(last_pack_num-100000));
	return ret;
}


int send_package_to_single(char *id, int speed, unsigned char update_flag,int last_get_time)
{
	int ret = 0;
	if(0x18 == update_flag)
		ret=send_package_to_single_0x18(id, speed,last_get_time,0);
	if(0x19 == update_flag)
		ret=send_package_to_single_0x19(id, speed,last_get_time,0);
	return ret;
}

int set_updating_flag_clear(struct inverter_info_t *firstinverter)
{
	int i;
	struct inverter_info_t *curinverter = firstinverter;
	for(i=0; (i<999)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
	{
		curinverter->updating_flag = 0;
	}
	return 0;
}


int set_updating_flag_one(char *id,struct inverter_info_t *firstinverter)
{
	int i;
	struct inverter_info_t *curinverter = firstinverter;
	for(i=0; (i<999)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
	{
		if(!strncmp(id,curinverter->inverterid,12))
			curinverter->updating_flag = 1;
		else
			curinverter->updating_flag = 0;
	}
	return 0;
}

int update_inverter(int thistime,struct inverter_info_t *firstinverter)
{ //crc_4k("/home/UPDATE_YC500.BIN",0,"406000008004");
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0, i, speed,j,nxt_sector,ret,shanquok=0,cur_crc=0;
	char **azResult;
	sqlite3 *db;
	int version;
	unsigned char update_flag = 0x00;
	struct inverter_info_t *curinverter = firstinverter;
	char buff[256] = {'\0'};
	char sendcommanddatetime_temp[20] = {'\0'};
	char sendcommandtime_temp[3] = {'\0'};
	int version_bin=0;
	int time_not_enough;
	int update_flag_num;
	version_bin = get_bin_version();
	FILE *fp=NULL;
	fp=fopen("/home/UPDATE_YC500.BIN","r");
	if(fp==NULL)
		{
			printmsg("No BIN");
			return -1;
		}
	fseek(fp,0,SEEK_END);
	int sector_all=ftell(fp)/4096;
	fclose(fp);

	speed = get_update_speed();

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))		//create a database
		return 0;

	for(i=0; (i<999)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
	{//printf("%d\n",__LINE__);
		if((curinverter->updating_flag==1))
		{
			if(last_package>200000)
				update_flag=0x19;
			else if(last_package>100000)
				update_flag=0x18;
			else
			{
				//update_flag_minus_one(curinverter->inverterid);
				return 0;
			}
			last_package=send_package_to_single_again(curinverter->inverterid, speed,thistime,last_package);
			if(last_package>0)
				return 0;
			ret=resend_lost_packets(curinverter->inverterid, speed, update_flag);
			set_updating_flag_clear(firstinverter);
			if(ret==0)
			{
				clear_update_flag(curinverter->inverterid);
			}
			version = query_inverter_version(curinverter->inverterid);
			update_inverter_version(curinverter->inverterid,version);
			save_update_inverter_result(curinverter->inverterid, version);
			stop_update(curinverter->inverterid);

			memset(buff,'\0',256);
			get_time(sendcommanddatetime_temp, sendcommandtime_temp);
			sprintf(buff,"%s%02d%06d%.14sEND",curinverter->inverterid,ret,version,sendcommanddatetime_temp);
			save_inverter_parameters_result_id(curinverter->inverterid, 147,buff,1);


		stop_update(curinverter->inverterid);

		}
	}
	//printf("%d\n",__LINE__);


	strcpy(sql, "SELECT id,update_flag FROM update_inverter WHERE update_flag>0 ORDER BY update_flag DESC");
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		for(i=1; i<=nrow; i++)
		{
			update_flag_num=atoi(azResult[2*i+1]);
			printf("\tupdate_flag_num=%d\n",update_flag_num);
			time_not_enough=0;
			curinverter=firstinverter;
			ret = 0;
			for(j=0; (j<999)&&(12==strlen(curinverter->inverterid)); j++, curinverter++)
			{
				if(!strncmp(azResult[i*2],curinverter->inverterid,12))
				{
					ret = 1;
					if((curinverter->curacctime<1800)||(curinverter->curflag=='0'))			//*****暂时600，应该1800
					{
						if(update_flag_num>3)
							time_not_enough=1;
						break;
					}
					else
						break;
				}
			}
			if(time_not_enough==1)
			{
				continue;
			}
			if(ret == 0)
			{
				clear_update_flag(azResult[i*2]);
				continue;
			}

			version = query_inverter_version(azResult[i*2]);
			if(version == 0)
			{
				if(update_flag_num>3)
				{
					update_flag_minus_one(azResult[i*2]);
					continue;
				}
				else
				{
					printdecmsg("inv_version",version);
					printdecmsg("bin_version",version_bin);
				}
			}
			else if(version == version_bin)
			{
				clear_update_flag(azResult[i*2]);
				continue;
			}
			else
			{
				printdecmsg("inv_version",version);
				printdecmsg("bin_version",version_bin);
				//sleep(10);
			}

			shanquok=0;
			print2msg("Update",azResult[i*2]);
			nxt_sector = start_update(azResult[i*2], speed);
			if(nxt_sector == 32)
			{
				clear_update_flag(azResult[i*2]);
				nxt_sector = set_update(azResult[i*2], speed);
				if(-1 != nxt_sector)
				{
					for(j=nxt_sector;j<sector_all;j++)
					{
						//fwritofile("/tmp/updating_status",j+1,sector_all);
						printdecmsg("Sector:",j);
						if(!send_package_to_single_new(azResult[i*2], speed,j))				//return 0:SUCCESS.
						{
							ret=resend_lost_packets_new(azResult[i*2], speed,j);
							if(ret==10)													//补包成功
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
						save_sector(azResult[i*2],j);
						shanquok=1;
						break;
					}
					if(nxt_sector<sector_all)
						if(shanquok==1)
							continue;
					shanquok=0;
					cur_crc=0;
					ret=crc_bin_file_new("/home/UPDATE_YC500.BIN",azResult[i*2]);
					if(ret==19)
					{
						for(j=0;j<sector_all;j++)
						{
							if(10!=crc_4k("/home/UPDATE_YC500.BIN",j,azResult[i*2]))
							{

								if(!send_package_to_single_new(azResult[i*2], speed,j))
								{
									ret=resend_lost_packets_new(azResult[i*2], speed,j);
									if(ret==10)													//补包成功
									{
										cur_crc=0;
										ret=crc_bin_file_new("/home/UPDATE_YC500.BIN",azResult[i*2]);
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
						ret=cover_atob(azResult[i*2],2,0);
						if(ret==0)
						{
							//fwritofile("/tmp/updating_status",0,sector_all);
							print2msg(azResult[i*2],"UPDATE SUCCESS!");
						}
						else if(ret==1)
							print2msg(azResult[i*2],"RELOADING FAILED!");
						else if(ret==2)
							print2msg(azResult[i*2],"READ_WRITE FAILED!");
						else if(ret==3)
							print2msg(azResult[i*2],"ADDRESS FAILED!");
						else print2msg(azResult[i*2],"UPDATE FAILED!");
						if(ret!=0)
							cover_atob(azResult[i*2],1,0);
					}
	//				send_package_to_single(azResult[i*2], speed, update_flag);
	//				resend_lost_packets(azResult[i*2], speed, update_flag);
					version = query_inverter_version(azResult[i*2]);
					update_inverter_version(azResult[i*2],version);
					save_update_inverter_result(azResult[i*2], version);
				}

				stop_update_new(azResult[i*2]);
			}
			else if((nxt_sector == 24)||(nxt_sector == 25))
			{
				update_flag_minus_one(azResult[i*2]);	//更新标志位减1
				set_updating_flag_one(azResult[i*2],firstinverter); //正在更新标志置1.
				last_package=send_package_to_single(azResult[i*2], speed, nxt_sector,thistime);
				if(last_package>0)
				{
					sqlite3_free_table( azResult );
					sqlite3_close(db);
					return 0;
				}
				ret=resend_lost_packets(azResult[i*2], speed, nxt_sector);
				if(ret==0)
				{
					set_updating_flag_clear(firstinverter);
					clear_update_flag(azResult[i*2]);
				}
				version = query_inverter_version(azResult[i*2]);
				update_inverter_version(azResult[i*2],version);
				save_update_inverter_result(azResult[i*2], version);
				stop_update(azResult[i*2]);

				memset(buff,'\0',256);
				get_time(sendcommanddatetime_temp, sendcommandtime_temp);
				sprintf(buff,"%s%02d%06d%.14sEND",azResult[i*2],ret,version,sendcommanddatetime_temp);
				save_inverter_parameters_result_id(azResult[i*2], 147,buff,1);

				//clear_update_flag(azResult[i]);
			}
			else
			{
				update_flag_minus_one(azResult[i*2]);	//更新标志位减1
				memset(buff,'\0',256);
				get_time(sendcommanddatetime_temp, sendcommandtime_temp);
				sprintf(buff,"%s01000000%.14sEND",azResult[i*2],sendcommanddatetime_temp);
				save_inverter_parameters_result_id(azResult[i*2], 147,buff,1);
			}
		}
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);
	
	return 0;
}
