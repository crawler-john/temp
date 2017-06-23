#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"
#include "get_reply_from_serial.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID
extern int displaymaxcount;			//液晶屏显示最大逆变器数

int get_update_speed()
{
	FILE *fp;
	int speed=32;
	char tmp[256];

	fp = fopen("/etc/yuneng/speed.conf", "r");
	if(fp)
	{
		fgets(tmp, 256, fp);
		speed = atoi(tmp);
		fclose(fp);
	}
	return speed;
}

unsigned char start_update(char *id, int speed)
{
	int i, ret;
	char sendbuff[256]={'\0'};
	char readbuff[256];
	unsigned short check = 0x00;
	unsigned short speed_mode;

	if(128==speed)
		speed_mode=0x01;
	else if(64==speed)
		speed_mode=0x02;
	else
		speed_mode=0x03;

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
	sendbuff[20] = 0x01;
	sendbuff[21] = 0x80;
	sendbuff[22] = displaymaxcount;
	sendbuff[23] = speed_mode;
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

		if((ret == 25) &&
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
				((readbuff[20] == 0x18) || (readbuff[20] == 0x19)) &&
				(readbuff[23] == 0xFE) &&
				(readbuff[24] == 0xFE)
			){
			print2msg(id, "Start to update inverters");
			return readbuff[20];
		}
	}

	printhexmsg(id, "Failed to start updating");
	return 0;
}

int stop_update(char *id)
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
	sendbuff[20] = 0x03;
	sendbuff[21] = 0xC0;
	sendbuff[58] = 0xFE;
	sendbuff[59] = 0xFE;

	for(i=2; i<56; i++)
		check = check + sendbuff[i];

	sendbuff[56] = check >> 8;		//CHK
	sendbuff[57] = check;		//CHK

	for(i=0; i<3; i++){
		write(plcmodem, sendbuff, 60);
		usleep(500000);

		ret = get_reply_from_serial(plcmodem, 5, 0, readbuff);

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
				(readbuff[20] == 0x3C) &&
				(readbuff[23] == 0xFE) &&
				(readbuff[24] == 0xFE)
			)
		{
			print2msg(id, "Stop updating successfully");
			return 0;
		}
	}

	print2msg(id, "Failed to stop updating");
	return -1;
}

int restore_inverter(char *id)
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
	sendbuff[20] = 0x06;
	sendbuff[21] = 0x60;
	sendbuff[58] = 0xFE;
	sendbuff[59] = 0xFE;

	for(i=2; i<56; i++)
		check = check + sendbuff[i];

	sendbuff[56] = check >> 8;		//CHK
	sendbuff[57] = check;		//CHK

	for(i=0; i<3; i++){
		write(plcmodem, sendbuff, 60);
		usleep(500000);

		ret = get_reply_from_serial(plcmodem, 200, 0, readbuff);

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
				(readbuff[20] == 0x06) &&
				(readbuff[23] == 0xFE) &&
				(readbuff[24] == 0xFE)
			){
				print2msg(id, "Restore successfully");
				update_restore_result(id);
				return 0;
		}
	}

	print2msg(id, "Failed to restore");
	return -1;
}

int update_restore_result(char *id)
{
	char inverter_result[65535]={'\0'};
	char sql[1024] = {'\0'};
	sqlite3 *db;

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))		//create a database
		return 0;

	sprintf(sql, "UPDATE restore_inverters SET restore_result=1 WHERE id='%s'", id);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);

	sprintf(inverter_result, "%sEND", id);
	save_inverter_parameters_result2(id, 133, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA

	return 0;
}

int clear_restore_flag(char *id)
{
	char sql[1024] = {'\0'};
	sqlite3 *db;

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))		//create a database
		return 0;

	sprintf(sql, "UPDATE restore_inverters SET restore_flag=0 WHERE id='%s'", id);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);
}

int process_restore_inverter(struct inverter_info_t *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0, i, speed;
	char **azResult;
	sqlite3 *db;

	speed = get_update_speed();

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))		//create a database
		return 0;

	strcpy(sql, "SELECT id FROM restore_inverters WHERE restore_flag=1");
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		for(i=1; i<=nrow; i++)
		{
			if(1 == check_plc_connected(firstinverter, azResult[i]))
			{
				clear_restore_flag(azResult[i]);
				if(0 != start_update(azResult[i], speed))
					restore_inverter(azResult[i]);

				stop_update(azResult[i]);
			}
		}
	}
	sqlite3_free_table( azResult );

	sqlite3_close(db);

	return 0;
}
