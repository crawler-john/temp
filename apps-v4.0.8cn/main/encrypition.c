#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"
#include "encrypition.h"
#include "debug.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID
extern char ecuid[13];			//ECU的ID

char key_global[16]={'\0'};



int initEncryption(struct inverter_info_t *firstinverter)
{
	struct inverter_info_t *curinverter = firstinverter;
	int i =0;
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db=NULL;


	if(SQLITE_OK != sqlite3_open("/home/encryption.db", &db))
		return -1;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){		//每个逆变器要一次数据

		//查询时间，并更新最后一次连接逆变器的时间
		memset(sql, '\0', sizeof(sql));
		sprintf(sql, "SELECT time FROM alarm where id = '%s'",curinverter->inverterid);
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
		{
			curinverter->connect_time = atoi(azResult[1]);
		}

		curinverter++;
	}
	sqlite3_close( db );
	return 0;
}



int save_encrypition_result(char *id, char *key, int status, int result)					//更新逆变器的密钥信息
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;

	memset(sql, '\0', sizeof(sql));
	if(SQLITE_OK != sqlite3_open("/home/encryption.db", &db))
		return -1;

	sprintf(sql, "REPLACE into info(id,key, status, result) values('%s' ,'%s',%d,%d) ", id, key, status, result);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int save_encrypition_key(char *id, char *key)					//读取密钥
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;

	memset(sql, '\0', sizeof(sql));
	if(SQLITE_OK != sqlite3_open("/home/encryption.db", &db))
		return -1;

	sprintf(sql, "REPLACE into info(id,key, status, result) values('%s' ,'%s',(select status from info where id ='%s'),(select result from info where id ='%s')) ", id, key,id,id);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int clear_set_flag()		//清除数据库中的设置标志
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;

	memset(sql, '\0', sizeof(sql));
	if(SQLITE_OK != sqlite3_open("/home/encryption.db", &db))
		return -1;

	sprintf(sql, "UPDATE key SET set_flag=0 WHERE item=1");
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int clear_read_flag()		//清除数据库中的读取标志
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;

	memset(sql, '\0', sizeof(sql));
	if(SQLITE_OK != sqlite3_open("/home/encryption.db", &db))
		return -1;

	sprintf(sql, "UPDATE key SET read_flag=0 WHERE item=1");
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int set_encrypition_key(char *id, char *key, char *buff_inv,char A139_flag)	//给逆变器添加密钥
{
	unsigned char sendbuff[512]={'\0'};
	char inverter_result[256]={'\0'};
	char readbuff[256];
	unsigned short check=0x00;
	int i, ret=0;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1B;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xD0;
	sendbuff[21] = 0xA0;
	sendbuff[22] = 0x00;
	sendbuff[23] = 0x00;
	sendbuff[24] = key[0];
	sendbuff[25] = key[1];
	sendbuff[26] = key[2];
	sendbuff[27] = key[3];
	sendbuff[28] = key[4];
	sendbuff[29] = key[5];
	sendbuff[30] = key[6];
	sendbuff[31] = key[7];

	for(i=2; i<32; i++){
		check = check + sendbuff[i];
	}

	sendbuff[32] = check >> 8;		//CHK
	sendbuff[33] = check;		//CHK
	sendbuff[34] = 0xFE;		//TAIL
	sendbuff[35] = 0xFE;		//TAIL

	printhexmsg("Set Encrypition Key", sendbuff, 36);

	for(i=0; i<3; i++){
		write(plcmodem, sendbuff, 36);
		usleep(500000);

		ret = get_reply_from_serial(plcmodem, 5, 0, readbuff);

		if((36 == ret) &&
				(readbuff[0] == 0xFB) &&
				(readbuff[1] == 0xFB) &&
				(readbuff[2] == 0x01) &&
				(readbuff[3] == 0x00) &&
				(readbuff[4] == 0x1B) &&
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
				(readbuff[20] == 0xD0) &&
				(readbuff[21] == 0xA0) &&
				(readbuff[23] == 0x00) &&
				(readbuff[34] == 0xFE) &&
				(readbuff[35] == 0xFE)
			){
			print2msg(id, "Set Encrypition Key successfully");
			if(0x00 == readbuff[22])
			{
				save_encrypition_result(id, key, 1, 1);
				sprintf(buff_inv, "%012s11END", id);
				return 0;
			}
			else
			{
//				save_encrypition_result(id, key, 0, 2);
				sprintf(buff_inv, "%012s22END", id);

				sprintf(inverter_result, "%012s0END", id);
				if(1 == A139_flag)
					save_inverter_parameters_result_id(id, 139, inverter_result);
				return 1;
			}
		}
		else
		{
			print2msg(id, "Failed to Set Encrypition Key");
		}
	}
//	save_encrypition_result(id, key, 0, 3);
	sprintf(buff_inv, "%012s32END", id);

	sprintf(inverter_result, "%012s0END", id);
	if(1 == A139_flag)
		save_inverter_parameters_result_id(id, 139, inverter_result);

	return 1;
}

int set_encrypition_all(struct inverter_info_t *firstinverter, char *key, int operator)		//设置所有逆变器的加密信息
{
	struct inverter_info_t *inverter = firstinverter;
	int i, count=0;
	char buff_ema[65535]={'\0'};
	char buff_all[65535]={'\0'};
	char buff_inv[256]={'\0'};
	char date_time[16]={'\0'};

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		memset(buff_inv, '\0', sizeof(buff_inv));
		set_encrypition_key(inverter->inverterid, key, buff_inv,1);
		count++;
		strcat(buff_all, buff_inv);
	}

	if(count>0)
	{
		get_date_time(date_time);

		sprintf(buff_ema, "%s%05d%s%s%012s%01d%04d%s%sEND%s", "APS13",(66+17*count),"A140","AAA0",ecuid, operator, count, "00000000000000",date_time, buff_all);

//		sprintf(buff_ema, "%012s%01d%04d%sEND%s", ecuid, operator, count, date_time, buff_all);
//		printf("!!!!!!!!!!!!!\nsss%s\n",buff_ema);
		save_process_result(140, buff_ema);
	}

	clear_set_flag();

	return 0;
}

int read_encrypition_key(char *id, char *key_ecu, char *buff_inv,char A139_flag)		//读取逆变器的密钥信息
{
	unsigned char sendbuff[512]={'\0'};
	char readbuff[256];
	char inverter_result[256]={'\0'};
	unsigned short check=0x00;
	int i, ret=0;
	char key[16]={'\0'};

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1B;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xD0;
	sendbuff[21] = 0xA4;
	sendbuff[22] = 0x00;
	sendbuff[23] = 0x00;
	sendbuff[24] = 0x00;
	sendbuff[25] = 0x00;
	sendbuff[26] = 0x00;
	sendbuff[27] = 0x00;
	sendbuff[28] = 0x00;
	sendbuff[29] = 0x00;
	sendbuff[30] = 0x00;
	sendbuff[31] = 0x00;

	for(i=2; i<32; i++){
		check = check + sendbuff[i];
	}

	sendbuff[32] = check >> 8;		//CHK
	sendbuff[33] = check;		//CHK
	sendbuff[34] = 0xFE;		//TAIL
	sendbuff[35] = 0xFE;		//TAIL

	printhexmsg("Read Encrypition Key", sendbuff, 36);

	for(i=0; i<3; i++){
		write(plcmodem, sendbuff, 36);
		usleep(500000);

		ret = get_reply_from_serial(plcmodem, 5, 0, readbuff);

		if((36 == ret) &&
				(readbuff[0] == 0xFB) &&
				(readbuff[1] == 0xFB) &&
				(readbuff[2] == 0x01) &&
				(readbuff[3] == 0x00) &&
				(readbuff[4] == 0x1B) &&
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
				(readbuff[20] == 0xD0) &&
				(readbuff[21] == 0xA4) &&
				(readbuff[34] == 0xFE) &&
				(readbuff[35] == 0xFE)
			){
			print2msg(id, "Read Encrypition Key successfully");
			for(i=0; i<8; i++)
				key[i] = readbuff[24+i];
			save_encrypition_key(id, key);
			sprintf(buff_inv, "%012s0%01dEND", id, readbuff[22]);

			if(!strlen(key_ecu))
			{
				for(i=0; i<8; i++)
				{
					if(0xFF != key[i])
					{
						sprintf(inverter_result, "%012s1END", id);
						if(1 == A139_flag)
							save_inverter_parameters_result_id(id, 139, inverter_result);
						return 1;
					}
				}
			}
			if(8 == strlen(key_ecu))
			{
				if(strcmp(key_ecu, key))
				{
					sprintf(inverter_result, "%012s2END", id);
					if(1 == A139_flag)
						save_inverter_parameters_result_id(id, 139, inverter_result);
					return 1;
				}
			}

			return 1;
		}
		else
		{
			print2msg(id, "Failed to Read Encrypition Key");
		}
	}
	sprintf(buff_inv, "%012s02END", id, readbuff[22]);
//	save_encrypition_result(id, key, 0, 3);

	return 0;
}

int read_encrypition_all(struct inverter_info_t *firstinverter, char *key_ecu)		//读取所有逆变器的加密信息
{
	struct inverter_info_t *inverter = firstinverter;
	int i, count=0;
	char buff_ema[65535]={'\0'};
	char buff_all[65535]={'\0'};
	char buff_inv[256]={'\0'};
	char date_time[16]={'\0'};

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		memset(buff_inv, '\0', sizeof(buff_inv));
		read_encrypition_key(inverter->inverterid, key_ecu, buff_inv,1);
		count++;
		strcat(buff_all, buff_inv);
	}

	if(count>0)
	{
		get_date_time(date_time);
		sprintf(buff_ema, "%s%05d%s%s%012s2%04d%s%sEND%s", "APS13",(66+17*count),"A140","AAA0",ecuid, count, "00000000000000",date_time, buff_all);

//		sprintf(buff_ema, "%012s2%04d%sEND%s", ecuid, count, date_time, buff_all);
//		printf("%s\n",buff_ema);
		save_process_result(140, buff_ema);
	}

	clear_read_flag();

	return 0;
}

int clear_encrypition_key(char *id, char *buff_inv,char A139_flag)		//给逆变器清空密钥
{
	unsigned char sendbuff[512]={'\0'};
	char inverter_result[256]={'\0'};
	char readbuff[256];
	unsigned short check=0x00;
	int i, ret=0;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1B;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xD0;
	sendbuff[21] = 0xA1;
	sendbuff[22] = 0x00;
	sendbuff[23] = 0x00;
	sendbuff[24] = 0xFF;
	sendbuff[25] = 0xFF;
	sendbuff[26] = 0xFF;
	sendbuff[27] = 0xFF;
	sendbuff[28] = 0xFF;
	sendbuff[29] = 0xFF;
	sendbuff[30] = 0xFF;
	sendbuff[31] = 0xFF;

	for(i=2; i<32; i++){
		check = check + sendbuff[i];
	}

	sendbuff[32] = check >> 8;		//CHK
	sendbuff[33] = check;		//CHK
	sendbuff[34] = 0xFE;		//TAIL
	sendbuff[35] = 0xFE;		//TAIL

	printhexmsg("Clear Encrypition Key", sendbuff, 36);

	for(i=0; i<3; i++){
		write(plcmodem, sendbuff, 36);
		usleep(500000);

		ret = get_reply_from_serial(plcmodem, 5, 0, readbuff);
		if((36 == ret) &&
				(readbuff[0] == 0xFB) &&
				(readbuff[1] == 0xFB) &&
				(readbuff[2] == 0x01) &&
				(readbuff[3] == 0x00) &&
				(readbuff[4] == 0x1B) &&
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
				(readbuff[20] == 0xD0) &&
				(readbuff[21] == 0xA1) &&
				(readbuff[23] == 0x55) &&
				(readbuff[34] == 0xFE) &&
				(readbuff[35] == 0xFE)
			){
			print2msg(id, "Clear Encrypition Key successfully");
			if(0x00 == readbuff[22])
			{
				save_encrypition_result(id, "", 2, 1);
				sprintf(buff_inv, "%012s10END", id);
				return 0;
			}
			else
			{
//				save_encrypition_result(id, key, 0, 2);
				sprintf(buff_inv, "%012s22END", id);

				sprintf(inverter_result, "%012s0END", id);
				if(1 == A139_flag)
					save_inverter_parameters_result_id(id, 139, inverter_result);
				return -1;
			}

		}
		else
		{
			print2msg(id, "Failed to Clear Encrypition Key");
		}
	}
//	save_encrypition_result(id, key, 0, 3);
	sprintf(buff_inv, "%012s32END", id);

	sprintf(inverter_result, "%012s0END", id);
	if(1 == A139_flag)
		save_inverter_parameters_result_id(id, 139, inverter_result);

	return -1;
}

int clear_encrypition_all(struct inverter_info_t *firstinverter)		//清除所有逆变器的加密信息
{
	struct inverter_info_t *inverter = firstinverter;
	int i, count=0,clear_count=0;
	char buff_ema[65535]={'\0'};
	char buff_all[65535]={'\0'};
	char buff_inv[256]={'\0'};
	char date_time[16]={'\0'};

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		memset(buff_inv, '\0', sizeof(buff_inv));
		if(0 == clear_encrypition_key(inverter->inverterid, buff_inv,1))
			clear_count++;
		count++;
		strcat(buff_all, buff_inv);
	}

	if(count>0)
	{
		get_date_time(date_time);
		if(clear_count == count)
		{
			sprintf(buff_ema, "%s%05d%s%s%012s3%04d%s%sEND%s", "APS13",(66+17*count),"A140","AAA0",ecuid, count, "00000000000000",date_time, buff_all);
		}else
		{
			sprintf(buff_ema, "%s%05d%s%s%012s1%04d%s%sEND%s", "APS13",(66+17*count),"A140","AAA0",ecuid, count, "00000000000000",date_time, buff_all);
		}

//		printf("%s\n",buff_ema);
		save_process_result(140, buff_ema);
	}

	clear_set_flag();

	return 0;
}

int encrypition_heartbeat()		//防盗系统的心跳包，在每次大轮询前广播发送给所有逆变器，在plc.c中调用
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	if(!strlen(key_global))
		return 0;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1B;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];	//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xD0;
	sendbuff[21] = 0xA3;
	sendbuff[22] = 0x00;
	sendbuff[23] = 0x00;
	sendbuff[24] = key_global[0];
	sendbuff[25] = key_global[1];
	sendbuff[26] = key_global[2];
	sendbuff[27] = key_global[3];
	sendbuff[28] = key_global[4];
	sendbuff[29] = key_global[5];
	sendbuff[30] = key_global[6];
	sendbuff[31] = key_global[7];

	for(i=2; i<32; i++){
		check = check + sendbuff[i];
	}

	sendbuff[32] = check >> 8;		//CHK
	sendbuff[33] = check;		//CHK
	sendbuff[34] = 0xFE;		//TAIL
	sendbuff[35] = 0xFE;		//TAIL

	printhexmsg("Encrypition Heartbeat", sendbuff, 36);

	write(plcmodem, sendbuff, 36);
	sleep(10);

	return 0;
}

//加密功能入口，从数据库中读取key表中的信息，如果有设置、清除、读取加密信息，先所有逆变器统一处理。然后查漏补缺，没有信息的逆变器，重新读取加密信息，如果逆变器的加密
//信息和最后一次操作不符合，单独再设置或清除加密
int process_encrypition(struct inverter_info_t *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	char buff_ema[65535]={'\0'};
	char buff_all[65535]={'\0'};
	char date_time[16]={'\0'};
	sqlite3 *db;
	char buff_inv[256]={'\0'};
	char key[16]={'\0'};
	char flag_supple = 0,clear_flag = 0;
	int operator, cmd=0;
	struct inverter_info_t *inverter = firstinverter;
	int i, j, exist, set_flag=0, read_flag=0;
	FILE *fp;
	char flag[2]={'\0'};
	int count =0;

	fp = fopen("/etc/yuneng/encryption.conf", "r");	//读取配置文件
	if(fp)
	{
		fgets(flag, sizeof(flag), fp);
		fclose(fp);

		if('1' != flag[0])	//没有开启，直接返回。
			return 0;
	}
	else	//文件不存在，说明没有加密功能，直接返回。
		return 0;

	if(SQLITE_OK != sqlite3_open("/home/encryption.db", &db))		//create a database
		return -1;

	strcpy(sql, "SELECT key,operator,cmd,set_flag,read_flag FROM key WHERE item=1");	//读取最后一次操作的信息。
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		if(1 == nrow)
		{
			if(azResult[5])
			{
				strcpy(key, azResult[5]);
				strcpy(key_global, azResult[5]);
			}
			if(azResult[6])
				operator = atoi(azResult[6]);
			if(azResult[7])
				cmd = atoi(azResult[7]);
			if(azResult[8])
				set_flag = atoi(azResult[8]);
			if(azResult[9])
				read_flag = atoi(azResult[9]);
		}
	}
	sqlite3_free_table( azResult );

	if(1 == set_flag)		//需要设置加密或清除加密
	{
		if(1 == cmd)		//设置加密
		{
			set_encrypition_all(firstinverter, key, operator);
		}
		if(2 == cmd)		//清除加密
			clear_encrypition_all(firstinverter);
	}

	if(1 == read_flag)		//读取逆变器信息
		read_encrypition_all(firstinverter, key);

	strcpy(sql, "SELECT id,status FROM info");
	flag_supple = 0;
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
		{
			memset(buff_inv,0x00,sizeof(buff_inv));
			exist = 0;
			if('1' == inverter->flag)
			{
				for(j=1; j<=nrow; j++)
				{
					if(!strcmp(inverter->inverterid, azResult[j*ncolumn]))
					{
						exist = 1;
						//存在逆变器，并且info表中的CMD和KEY表中的不同
						if((!azResult[j*ncolumn+1]) || (cmd != atoi(azResult[j*ncolumn+1])))		//逆变器的信息已存在，如果逆变器的加密信息和最后一次操作不一致，需要重新操作。
						{
							if(1 == cmd)
							{
								if(0 == set_encrypition_key(inverter->inverterid, key, buff_inv,0))
								{
									flag_supple = 1;
								}
							}
							if(2 == cmd)
							{
								if(0 == clear_encrypition_key(inverter->inverterid, buff_inv,0))
								{
									flag_supple = 2;
								}else
								{
									clear_flag = 1;
								}
							}

						}else //存在逆变器，并且info表中的CMD和KEY表中的不同
						{
							if(atoi(azResult[j*ncolumn+1]) == 1)
							{
								sprintf(buff_inv, "%012s11END", inverter->inverterid);
							}else if (atoi(azResult[j*ncolumn+1]) == 2)
							{
								sprintf(buff_inv, "%012s10END", inverter->inverterid);
							}else
							{
								sprintf(buff_inv, "%012s22END", inverter->inverterid);
							}

						}
						break;
					}
				}
				if(0 == exist)		//逆变器的加密信息不存在，需要重新读取。
				{
					read_encrypition_key(inverter->inverterid, key, buff_inv,0);
				}

			}
			count++;
			strcat(buff_all, buff_inv);
		}
	}

	if(flag_supple == 1)
	{
		get_date_time(date_time);

		sprintf(buff_ema, "%s%05d%s%s%012s0%00d%s%sEND%s", "APS13",(66+17*count),"A140","AAA0",ecuid, count, "00000000000000",date_time, buff_all);
//		printf("%s\n",buff_ema);
		save_process_result(140, buff_ema);
	}else if (flag_supple == 2)
	{
		get_date_time(date_time);
		if(clear_flag == 0)
		{
			sprintf(buff_ema, "%s%05d%s%s%012s3%04d%s%sEND%s", "APS13",(66+17*count),"A140","AAA0",ecuid, count, "00000000000000",date_time, buff_all);
		}else
		{
			sprintf(buff_ema, "%s%05d%s%s%012s0%04d%s%sEND%s", "APS13",(66+17*count),"A140","AAA0",ecuid, count, "00000000000000",date_time, buff_all);
		}
//		printf("%s\n",buff_ema);
		save_process_result(140, buff_ema);
	}

	sqlite3_free_table( azResult );

	sqlite3_close(db);

	return 0;
}

int save_Alarm(char *id,int time)
{

	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;

	memset(sql, '\0', sizeof(sql));
	if(SQLITE_OK != sqlite3_open("/home/encryption.db", &db))
		return -1;

	sprintf(sql, "REPLACE into alarm(id,time) values('%s' ,%d) ", id, time);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int select_info(char *id,int *encryption_status)
{
	int i =0;
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db=NULL;


	if(SQLITE_OK != sqlite3_open("/home/encryption.db", &db))
	{
		*encryption_status = 3;
		return -1;
	}


	//查询时间，并更新最后一次连接逆变器的时间
	memset(sql, '\0', sizeof(sql));
	sprintf(sql, "SELECT status FROM info where id = '%s'",id);
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		*encryption_status = atoi(azResult[1]);
	}else
	{
		*encryption_status = 3;
	}

	sqlite3_close( db );
	return 0;
}

int process_encryption_alarm(struct inverter_info_t *firstinverter)
{
	int curTime = 0,i=0;
	struct inverter_info_t *curinverter = firstinverter;
	char inverter_result[256]={'\0'};
	char buff_ema[65535]={'\0'};
	char date_time[16]={'\0'};
	int encryption_status;
	curTime = time(NULL);
	//判断通信状态，如果通信


	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){		//每个逆变器要一次数据
		if('1' == curinverter->flag)	// 能通讯上，更新到最新的时间
		{
			//如果和之前那次的时间相差86400秒，上报一条A140
			if(curTime - curinverter->connect_time > 86400)
			{
				get_date_time(date_time);
				select_info(curinverter->inverterid,&encryption_status);
				sprintf(buff_ema, "%s%05d%s%s%012s0%00d%s%sEND%012s1%1dEND", "APS13",(66+17),"A140","AAA0",ecuid, 1, "00000000000000",date_time, curinverter->inverterid,encryption_status);
				save_process_result(140, buff_ema);
			}

			//只要通信上了就更新数据库时间
			save_Alarm(curinverter->inverterid,curTime);
			curinverter->connect_time = curTime;
		}else	//不能通讯上
		{
			//如果和之前的时间相差86400秒，上报一条A139
			if(curTime - curinverter->connect_time > 86400)
			{
				sprintf(inverter_result, "%012s3END", curinverter->inverterid);
				save_inverter_parameters_result_id(curinverter->inverterid, 139, inverter_result);
			}
		}

		curinverter++;
	}
}
