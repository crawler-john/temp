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
{printf("update********************\n\n*******************************\n");
	int ret;
	int i=0;
	int crc;
	unsigned char data[65535];
	unsigned char sendbuff[76]={0x00};
	char flag;
	FILE *fp=fopen("/home/OPT_TEST_1.BIN","r");
	if(fp==NULL)
		{
			printmsg("No BIN");
			return -1;
		}
	fseek(fp,0,SEEK_END);
	int pos=ftell(fp);
	fclose(fp);
	pos=(pos-0.5)/64+1;
	printf("pos=%d",pos);

	sendbuff[8]=pos/256;
	sendbuff[9]=pos%256;
	sendbuff[10]=0x01;
	sendbuff[11]=0x01;
	fp=fopen("/etc/yuneng/remote_upload1.conf","r");
	if(fp==NULL)
	{}
	else
	{
		flag=fgetc(fp);
		fclose(fp);
		sendbuff[10]=flag-0x30;
	}



	fp=fopen("/etc/yuneng/remote_upload2.conf","r");
	if(fp==NULL)
	{}
	else
	{
		flag=fgetc(fp);
		fclose(fp);
		sendbuff[11]=flag-0x30;
	}


	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x03;
	sendbuff[4]=0x01;
	sendbuff[5]=0x80;
	sendbuff[74]=0xfe;
	sendbuff[75]=0xfe;

	crc=crc_array(&sendbuff[2],70);
	sendbuff[72]=crc/256;
	sendbuff[73]=crc%256;
	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,sendbuff,76);printhexmsg("Ysend",sendbuff,76);
		printmsg("Sendupdatepackage_start");
		ret = zb_get_reply(data,inverter);
		if((0x03 == data[2]) && (0x02 == data[3]) && (0x18 == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[8]) && (0xFE == data[9]))
		{
			crc=crc_array(&data[2],4);
			if((data[6]==crc/256)&&(data[7]==crc%256))
			break;
		}
	}

	if(i>=3)								//如果发送3遍指令仍然没有返回正确指令，则返回-1
		return -1;
	else
		return 1;

}

int Sendupdatepackage_single(inverter_info *inverter)	//发送单点数据包
{printf("**********\n2\n*************");
	int fd;
	int crc;
	int i=0,package_num=0;
	unsigned char package_buff[100];
	unsigned char sendbuff[76]={0xff};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x03;
	sendbuff[4]=0x02;
	sendbuff[5]=0x40;
	sendbuff[72]=0x00;
	sendbuff[73]=0x00;
	sendbuff[74]=0xfe;
	sendbuff[75]=0xfe;

//	if((1 == inverter->model)||(2 == inverter->model))	//YC250机型
//		fd=open("/home/UPDATE_YC250.BIN", O_RDONLY);
//	else if((3 == inverter->model)||(4 == inverter->model))	//YC500机型
//		fd=open("/home/UPDATE_YC500.BIN", O_RDONLY);
//	else if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN机型
//		fd=open("/home/UPDATE_YC1000.BIN", O_RDONLY);
//	else
//		return -1;		//没有对应更新包

	fd=open("/home/OPT_TEST_1.BIN", O_RDONLY);
	if(fd>0)
	{
		while(read(fd,package_buff,64)>0){
			sendbuff[6]=package_num/256;
			sendbuff[7]=package_num%256;
			for(i=0;i<64;i++){
				sendbuff[i+8]=package_buff[i];
			}
			crc=crc_array(&sendbuff[2],70);
			sendbuff[72]=crc/256;
			sendbuff[73]=crc%256;

			zb_send_cmd(inverter,sendbuff,76);
			package_num++;
			printdecmsg("package_num",package_num);printhexmsg("send",sendbuff,76);
			usleep(150000);		//在单点帧里已经加入延时操作，这里就省去
			memset(package_buff, 0, sizeof(package_buff));
		}
		close(fd);
		return 1;
	}
	else
		return -2;		//发包不完全的情况
}

int Complementupdatepackage_single(inverter_info *inverter)	//检查漏掉的数据包并补发
{
	int crc;
	int fd;
	int ret;
	int i=0,j=0,k=0;
	unsigned char data[65535];
	unsigned char checkbuff[76]={0x00};
	unsigned char sendbuff[76]={0xff};
	unsigned char package_buff[100];

	checkbuff[0]=0xfc;
	checkbuff[1]=0xfc;
	checkbuff[2]=0x02;
	checkbuff[3]=0x03;
	checkbuff[4]=0x04;
	checkbuff[5]=0x20;
	checkbuff[74]=0xfe;
	checkbuff[75]=0xfe;

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x03;
	sendbuff[4]=0x02;
	sendbuff[5]=0x4f;
	sendbuff[72]=0x00;
	sendbuff[73]=0x00;
	sendbuff[74]=0xfe;
	sendbuff[75]=0xfe;

	clear_zbmodem();
	do{
		zb_send_cmd(inverter,checkbuff,76);
		printmsg("Complementupdatepackage_single_checkbuff");
		ret = zb_get_reply(data,inverter);
		printdecmsg("ret",ret);
		i++;
	}while((-1==ret)&&(14!=ret)&&(i<3));

	if((0x03 == data[2]) && (0x02 == data[3]) && (14 == ret) && (0x42 == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[12]) && (0xFE == data[13]))
	{

//		if((1 == inverter->model)||(2 == inverter->model))	//YC250机型
//			fd=open("/home/UPDATE_YC250.BIN", O_RDONLY);
//		else if((3 == inverter->model)||(4 == inverter->model))	//YC500机型
//			fd=open("/home/UPDATE_YC500.BIN", O_RDONLY);
//		else if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN机型
//			fd=open("/home/UPDATE_YC1000.BIN", O_RDONLY);
		fd=open("/home/OPT_TEST_1.BIN", O_RDONLY);

		if(fd>0){
			while((data[8]*256+data[9])>0)
			{
				lseek(fd,(data[6]*256+data[7])*64,SEEK_SET);
				memset(package_buff, 0, sizeof(package_buff));
				read(fd, package_buff, 64);
				sendbuff[6]=data[6];
				sendbuff[7]=data[7];
				for(k=0;k<64;k++){
					sendbuff[k+8]=package_buff[k];
				}
				crc=crc_array(&sendbuff[2],70);
				sendbuff[72]=crc/256;
				sendbuff[73]=crc%256;

				for(i=0;i<7;i++)
				{
					zb_send_cmd(inverter,sendbuff,76);
					ret = zb_get_reply(data,inverter);
					if((0x03 == data[2]) && (0x02 == data[3]) && (0x24 == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[12]) && (0xFE == data[13]))
					{
						crc=crc_array(&data[2],8);
						if((data[10]==crc/256)&&(data[11]==crc%256))
							break;
					}
				}
				if(i>=7)
				{
					printmsg("Complementupdatepackage single 3 times failed");
					return -1;		//补单包3次没响应的情况
				}
				printdecmsg("Complement_package",(data[8]*256+data[9]));
				usleep(30000);
			}
		return 1;	//成功补包
		}
		close(fd);
	}
	else if((0x03 == data[2]) && (0x02 == data[3]) &&(14 == ret) && (0x45 == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[12]) && (0xFE == data[13]))//补包如果超过512包就直接还原
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
void crc_add_crc(unsigned char *pchMsg, int wDataLen,int cc)
{
	unsigned short wCRCTalbeAbs[16] = {0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400};
	unsigned short wCRC = cc;
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

int crc_bin_file(inverter_info *inverter)
{
	char checkbuff[76]={'\0'};
	char data[100]={'\0'};
	int crc,crc1,crc2;
	unsigned short wCRCTalbeAbs[16] = {0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400};
	unsigned short wCRC = 0xFFFF;
	int fd,i;
	int ret;
	unsigned char chChar;
	unsigned char *pchMsg;
	fd=open("/home/OPT_TEST_1.BIN", O_RDONLY);
	if(fd>0)
	{printf("1\n");
		crc=crc_file("/home/OPT_TEST_1.BIN");
		close(fd);
		checkbuff[0]=0xFC;
		checkbuff[1]=0xFC;
		checkbuff[2]=0x02;
		checkbuff[3]=0x03;
		checkbuff[4]=0x07;
		checkbuff[5]=0xE0;
		checkbuff[8]=crc/256;
		checkbuff[9]=crc%256;
		checkbuff[74]=0xFE;
		checkbuff[75]=0xFE;
		crc=crc_array(&checkbuff[2],70);
		checkbuff[72]=crc/256;
		checkbuff[73]=crc%256;

		zb_send_cmd(inverter,checkbuff,76);
		printhexmsg("Ysend",checkbuff,76);
		ret = zb_get_reply(data,inverter);
		if((data[2]==0x03)&&(data[3]==0x02)&&(ret%10==0)&&(data[0]==0xFB)&&(data[1]==0xFB)&&(data[4]==0x01)&&(data[8]==0xFE)&&(data[9]==0xFE))
		{

			crc=crc_array(&data[2],4);
			if((data[6]==crc/256)&&(data[7]==crc%256))
			{
				if(data[5]==0x7A)
					return 1;
				else if(data[5]==0x7E)
					return -1;
				else
					return 0;
			}
		}

	}
	return -1;
}

int remote_update_single(inverter_info *inverter)
{
	int ret_sendsingle,ret_complement,ret_update_start=0;
	char flag;

//	Update_success_end(inverter);
	if(1==Sendupdatepackage_start(inverter))						//1远程更新启动指令
	{
		printmsg("Sendupdatepackage_start_OK");
		ret_sendsingle = Sendupdatepackage_single(inverter);		//2数据包发送

		if(-1==ret_sendsingle)		//没有对应升级文件情况
		{
			printmsg("No corresponding BIN file");
			return -1;
		}
		else if(-2==ret_sendsingle)		//文件发包不全情况
		{
//			Restore(inverter);
//			Update_success_end(inverter);
		}
		else if(1==ret_sendsingle)		//文件发送完全的情况
		{
			printmsg("Complementupdatepackage_single");
			ret_complement = Complementupdatepackage_single(inverter);	//3补包指令

			if(-1==ret_complement)		//补单包超过3次没响应和补包个数直接超过512个的情况
			{
//				Restore(inverter);
//				Update_success_end(inverter);
			}
			else if(1==ret_complement)		//成功补包完全的情况
			{
				ret_update_start=crc_bin_file(inverter);
//				ret_update_start=Update_start(inverter);
				if(1==ret_update_start)
				{
					printmsg("Update start successful");
//					Update_success_end(inverter);
				}
				else if(-1==ret_update_start)
				{
					printmsg("Update_start_failed");
//					Restore(inverter);
//					Update_success_end(inverter);
				}
				else if(0==ret_update_start)
				{
					printmsg("Update start no response");
					return -1;
				}
			}
			else if(0==ret_complement)
			{
				return -1;
			}
		}
	}
	else
		return -1;
}

int remote_update(inverter_info *firstinverter)
{
	sqlite3 *db=NULL;
	char *zErrMsg = 0;
	int i=0, nrow = 0, ncolumn = 0, j=0;
	char **azResult;
	char sql[1024]={'\0'};

	inverter_info *curinverter = firstinverter;

	sqlite3_open("/home/database.db", &db);

printmsg("if111");
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++)
	{
//		if(curinverter->dataflag==1)
//		{
			printmsg("222");
			sprintf(sql,"SELECT id FROM update_inverter WHERE id='%s' AND update_flag=1 ",curinverter->inverterid);
			sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
			//print2msg("zErrMsg=",zErrMsg);

			if(1==nrow)
			{printmsg("333");
				printmsg(curinverter->inverterid);
				remote_update_single(curinverter);
				sprintf(sql,"UPDATE update_inverter SET update_flag=0 WHERE id='%s' ",curinverter->inverterid);
				for(j=0;j<3;j++)
				{
					if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
						break;
					sleep(1);
				}
				for(j=0;j<3;j++)
				{
					if(1 == zb_query_inverter_info(curinverter))
					{
						update_inverter_model_version(curinverter);
						break;
					}
				}

			}
			sqlite3_free_table( azResult );
//		}
		curinverter++;
	}

	sqlite3_close( db );

}

