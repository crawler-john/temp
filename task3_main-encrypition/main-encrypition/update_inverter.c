#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "sqlite3.h"
#include "get_reply_from_serial.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID

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
int send_package_to_single_0x18(char *id, int speed)
{
	int i, fd, package_num=0;
	char sendbuff[256]={'\0'};
	unsigned char package_buff[128];
	unsigned short check = 0x00;

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
			sleep(1);

			package_num++;
			printdecmsg("package_num",package_num);
			printhexmsg("sendbuff", sendbuff, 28+speed);
		}
		close(fd);
		return 0;
	}

	return -1;
}

/*发送包给所有逆变器*/
int send_package_to_single_0x19(char *id, int speed)
{
	int i, fd, package_num=0;
	char sendbuff[256]={'\0'};
	unsigned char package_buff[128];
	unsigned short check = 0x00;

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

int send_package_to_single(char *id, int speed, unsigned char update_flag)
{
	if(0x18 == update_flag)
		send_package_to_single_0x18(id, speed);
	if(0x19 == update_flag)
		send_package_to_single_0x19(id, speed);
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
		return -1;
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
					return -1;
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
							return -1;		//补单包5次没响应的情况
						}
						printdecmsg("Resend the lost packet", (readbuff[23]*256 + readbuff[24]));
					}
					resend_packet_flag = 1;
					print2msg(id, "All of the lost packets have been resent");
					close(fd);
					if(-1==send_update_command(id))
					{
						//restore_inverter(inverter);
						return -1;
					}

				}
			}

			return 0;
		}
		else if(readbuff[20] == 0x45)
		{
			resend_packet_flag = 1;
			restore_inverter(id);		//包数丢失过多，还原逆变器程序
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
		return -1;
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
					return -1;
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
							return -1;		//补单包5次没响应的情况
						}
						printdecmsg("Resend the lost packet", (readbuff[23]*256 + readbuff[24]));
					}
					resend_packet_flag = 1;
					print2msg(id, "All of the lost packets have been resent");
					close(fd);
					if(-1==send_update_command(id))
					{
						//restore_inverter(inverter);
						return -1;
					}

				}
			}

			return 0;
		}
		else if(readbuff[20] == 0x45)
		{
			resend_packet_flag = 1;
			restore_inverter(id);		//包数丢失过多，还原逆变器程序
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

int resend_lost_packets(char *id, int speed, unsigned char update_flag)
{
	if(0x18 == update_flag)
		resend_lost_packets_0x18(id, speed);
	if(0x19 == update_flag)
		resend_lost_packets_0x19(id, speed);
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

int update_inverter()
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0, i, speed;
	char **azResult;
	sqlite3 *db;
	int version;
	unsigned char update_flag = 0x00;

	speed = get_update_speed();

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))		//create a database
		return 0;

	strcpy(sql, "SELECT id FROM update_inverter WHERE update_flag=1");
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		for(i=1; i<=nrow; i++)
		{
			clear_update_flag(azResult[i]);
			update_flag = start_update(azResult[i], speed);
			if(0 != update_flag)
			{
				send_package_to_single(azResult[i], speed, update_flag);
				resend_lost_packets(azResult[i], speed, update_flag);
				version = query_inverter_version(azResult[i]);
				update_inverter_version(azResult[i]);
				save_update_inverter_result(azResult[i], version);
			}

			stop_update(azResult[i]);
		}
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);
	
	return 0;
}
