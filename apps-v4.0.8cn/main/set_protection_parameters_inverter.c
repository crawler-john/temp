/*
 * 本文件的主要功能是设置单个逆变器的保护参数。同时兼容新的13项保护参数和老的5项参数。
 * Created by Zhyf
 * Created on 2014/08/22
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"
#include "set_protection_parameters.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID
extern int caltype;		//计算方式，NA版和非NA版的区别
char set_result[65535];

/*读取逆变器的返回结果*/
/*int get_reply_from_inverter(struct inverter_info_t *inverter, char *data)			//读取逆变器的返回帧
{
	int i;
	int size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	if(select(plcmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		print2msg(inverter->inverterid, "Get reply time out");
		return -1;
	}
	else
	{
		size = read(plcmodem, data, 255);

		printhexmsg(inverter->inverterid, data, size);
		return size;
	}
}*/

/*读取设置的参数值和标志位*/
int get_value_flag_one(char *id, char *para_name, char *value)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	float temp;
	sqlite3 *db;
	int res;
	
	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK != res){		//create a database
		return -1;
	}

	strcpy(sql, "SELECT id,parameter_name,parameter_value FROM set_protection_parameters_inverter WHERE set_flag=1 LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		strcpy(id, azResult[3]);
		strcpy(para_name, azResult[4]);
		strcpy(value, azResult[5]);
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	if(1 == nrow)
		return 1;
	else
		return 0;
}

int get_under_voltage_slow_one(char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int under_voltage_slow=0xFFFF, res, i;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database
		sprintf(sql, "SELECT parameter_value FROM set_protection_parameters_inverter WHERE parameter_name='under_voltage_slow' AND id='%s'", id);
		for(i=0; i<3; i++){
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
				if(1 == nrow){
					if(2 == caltype)
						temp = atof(azResult[1]) * 2.93;
					else if(1 == caltype)
						temp = atof(azResult[1]) * 2.90345;
					else
						temp = atof(azResult[1]) * 1.48975;
					if((temp-(int)temp)>=0.5)
						under_voltage_slow = temp + 1;
					else
						under_voltage_slow = temp;
				}
				sqlite3_free_table( azResult );
				break;
			}
			else{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}

//		if(1 != nrow){
//			strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='under_voltage_slow'");
//			for(i=0; i<3; i++){
//				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
//					if(1 == nrow){
//						if(2 == caltype)
//							temp = atof(azResult[1]) * 2.93;
//						else if(1 == caltype)
//							temp = atof(azResult[1]) * 2.90345;
//						else
//							temp = atof(azResult[1]) * 1.48975;
//						if((temp-(int)temp)>=0.5)
//							under_voltage_slow = temp + 1;
//						else
//							under_voltage_slow = temp;
//					}
//					sqlite3_free_table( azResult );
//					break;
//				}
//				else{
//					print2msg("Get under_voltage_slow", zErrMsg);
//					sqlite3_free_table( azResult );
//				}
//			}
//		}
	}
	sqlite3_close(db);

	return under_voltage_slow;
}

int get_over_voltage_slow_one(char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int over_voltage_slow=0xFFFF, res, i;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database
		sprintf(sql, "SELECT parameter_value FROM set_protection_parameters_inverter WHERE parameter_name='over_voltage_slow' AND id='%s'", id);
		for(i=0; i<3; i++){
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
				if(1 == nrow){
					if(2 == caltype)
						temp = atof(azResult[1]) * 2.93;
					else if(1 == caltype)
						temp = atof(azResult[1]) * 2.90345;
					else
						temp = atof(azResult[1]) * 1.48975;
					if((temp-(int)temp)>=0.5)
						over_voltage_slow = temp + 1;
					else
						over_voltage_slow = temp;
				}
				sqlite3_free_table( azResult );
				break;
			}
			else{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}

//		if(1 != nrow){
//			strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='over_voltage_slow'");
//			for(i=0; i<3; i++){
//				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
//					if(1 == nrow){
//						if(2 == caltype)
//							temp = atof(azResult[1]) * 2.93;
//						else if(1 == caltype)
//							temp = atof(azResult[1]) * 2.90345;
//						else
//							temp = atof(azResult[1]) * 1.48975;
//						if((temp-(int)temp)>=0.5)
//							over_voltage_slow = temp + 1;
//						else
//							over_voltage_slow = temp;
//					}
//					sqlite3_free_table( azResult );
//					break;
//				}
//				else{
//					print2msg("Get under_voltage_slow", zErrMsg);
//					sqlite3_free_table( azResult );
//				}
//			}
//		}
	}
	sqlite3_close(db);

	return over_voltage_slow;
}

char get_under_frequency_slow_one(char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char under_frequency_slow=0xFF, res, i;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database
		sprintf(sql, "SELECT parameter_value FROM set_protection_parameters_inverter WHERE parameter_name='under_frequency_slow' AND id='%s'", id);
		for(i=0; i<3; i++){
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
				if(1 == nrow){
					if((2 == caltype)||(1 == caltype)){
						if((600 - (int)(atof(azResult[1])*10))<0)
							under_frequency_slow = 0xFF;
						else
							under_frequency_slow = 600 - (int)(atof(azResult[1])*10);
					}
					else{
						if((500 - (int)(atof(azResult[1])*10))<0)
							under_frequency_slow = 0xFF;
						else
							under_frequency_slow = 500 - (int)(atof(azResult[1])*10);
					}
				}
				sqlite3_free_table( azResult );
				break;
			}
			else{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}

//		if(1 != nrow){
//			strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='under_frequency_slow'");
//			for(i=0; i<3; i++){
//				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
//					if(1 == nrow){
//						if((2 == caltype)||(1 == caltype))
//							under_frequency_slow = 600 - atof(azResult[1])*10;
//						else
//							under_frequency_slow = 500 - atof(azResult[1])*10;
//					}
//					sqlite3_free_table( azResult );
//					break;
//				}
//				else{
//					print2msg("Get under_voltage_slow", zErrMsg);
//					sqlite3_free_table( azResult );
//				}
//			}
//		}
	}
	sqlite3_close(db);

	return under_frequency_slow;
}

char get_over_frequency_slow_one(char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char over_frequency_slow=0xFF, res, i;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database

		sprintf(sql, "SELECT parameter_value FROM set_protection_parameters_inverter WHERE parameter_name='over_frequency_slow' AND id='%s'", id);
		for(i=0; i<3; i++){
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
				if(1 == nrow){
					if((2 == caltype)||(1 == caltype)){
						if(((int)(atof(azResult[1])*10)-600)<0)
							over_frequency_slow = 0xFF;
						else
							over_frequency_slow = (int)(atof(azResult[1])*10)-600;
					}
					else{
						if(((int)(atof(azResult[1])*10)-500)<0)
							over_frequency_slow = 0xFF;
						else
							over_frequency_slow = (int)(atof(azResult[1])*10)-500;
					}
				}
				sqlite3_free_table( azResult );
				break;
			}
			else{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}

//		if(1 != nrow){
//			strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='over_frequency_slow'");
//			for(i=0; i<3; i++){
//				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
//					if(1 == nrow){
//						if((2 == caltype)||(1 == caltype))
//							over_frequency_slow = atof(azResult[1])*10-600;
//						else
//							over_frequency_slow = atof(azResult[1])*10-500;
//					}
//					sqlite3_free_table( azResult );
//					break;
//				}
//				else{
//					print2msg("Get under_voltage_slow", zErrMsg);
//					sqlite3_free_table( azResult );
//				}
//			}
//		}
	}
	sqlite3_close(db);

	return over_frequency_slow;
}

/*int get_recovery_time_from_inverter(struct inverter_info_t *inverter)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;
	fd_set rd;
	struct timeval timeout;
	int grid_recovery_time=-1;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
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
	sendbuff[21] = 0x11;

	for(i=2; i<22; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	write(plcmodem, sendbuff, 26);

	res = get_reply_from_inverter(inverter, readbuff);

	if((42 == res) &&
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0x01 == readbuff[2]) &&
			(0x00 == readbuff[3]) &&
			(0x21 == readbuff[4]) &&
			(ccuid[0] == readbuff[5]) &&
			(ccuid[1] == readbuff[6]) &&
			(ccuid[2] == readbuff[7]) &&
			(ccuid[3] == readbuff[8]) &&
			(ccuid[4] == readbuff[9]) &&
			(ccuid[5] == readbuff[10]) &&
			(inverter->tnuid[0] == readbuff[11]) &&
			(inverter->tnuid[1] == readbuff[12]) &&
			(inverter->tnuid[2] == readbuff[13]) &&
			(inverter->tnuid[3] == readbuff[14]) &&
			(inverter->tnuid[4] == readbuff[15]) &&
			(inverter->tnuid[5] == readbuff[16]) &&
			(0x4F == readbuff[17]) &&
			(0x00 == readbuff[18]) &&
			(0x00 == readbuff[19]) &&
			(0xDA == readbuff[20]) &&
			(0xFE == readbuff[40]) &&
			(0xFE == readbuff[41])){
		if(9 == readbuff[36])
			grid_recovery_time = 300;
		else
			grid_recovery_time = readbuff[36]*32;
	}
	else if((37 == res) &&
			(0xFB == readbuff[0])&&
			(0xFB == readbuff[1])&&
			(0x01 == readbuff[2])&&
			(0x00 == readbuff[3])&&
			(0x1C == readbuff[4])&&
			(ccuid[0] == readbuff[5])&&
			(ccuid[1] == readbuff[6])&&
			(ccuid[2] == readbuff[7])&&
			(ccuid[3] == readbuff[8])&&
			(ccuid[4] == readbuff[9])&&
			(ccuid[5] == readbuff[10])&&
			(inverter->tnuid[0] == readbuff[11])&&
			(inverter->tnuid[1] == readbuff[12])&&
			(inverter->tnuid[2] == readbuff[13])&&
			(inverter->tnuid[3] == readbuff[14])&&
			(inverter->tnuid[4] == readbuff[15])&&
			(inverter->tnuid[5] == readbuff[16])&&
			(0x4F == readbuff[17])&&
			(0x00 == readbuff[18])&&
			(0x00 == readbuff[19])&&
			(0xDD == readbuff[20])&&
			(0xFE == readbuff[35])&&
			(0xFE == readbuff[36])){
		grid_recovery_time = readbuff[31]*256 + readbuff[32];
	}

	return grid_recovery_time;
}*/

int get_grid_recovery_time_one(struct inverter_info_t *firstinverter, char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int recovery_time=300, recovery_time_result=-1, res, i, flag=0;
	struct inverter_info_t *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database
		sprintf(sql, "SELECT parameter_value FROM set_protection_parameters_inverter WHERE parameter_name='grid_recovery_time' AND id='%s'", id);
		for(i=0; i<3; i++){
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
				if(1 == nrow){
					flag = 1;
					recovery_time = atof(azResult[1]);
				}
				sqlite3_free_table( azResult );
				break;
			}
			else{
				print2msg("Get grid recovery time", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}

		if(!flag){
			strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='grid_recovery_time'");
			for(i=0; i<3; i++){
				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
					if(1 == nrow){
						flag = 1;
						recovery_time = atof(azResult[1]);
					}
					sqlite3_free_table( azResult );
					break;
				}
				else{
					print2msg("Get grid recovery time", zErrMsg);
					sqlite3_free_table( azResult );
				}
			}
		}

		if(!flag){
			strcpy(sql, "SELECT grid_recovery_time FROM protection_parameters WHERE grid_recovery_time IS NOT NULL LIMIT 0,1");
			for(i=0; i<3; i++){
				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
					if(1 == nrow){
						flag = 1;
						recovery_time = atof(azResult[1]);
					}
					sqlite3_free_table( azResult );
					break;
				}
				else{
					print2msg("Get grid recovery time", zErrMsg);
					sqlite3_free_table( azResult );
				}
			}
		}
		sqlite3_close(db);
	}

	if(!flag){
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
			recovery_time_result = get_recovery_time_from_inverter(inverter);
			if(-1 != recovery_time_result)
				return recovery_time_result;
		}
	}

	return recovery_time;
}

/*设置逆变器的快速欠压保护值*/
int set_undervoltage_fast_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xE1;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage fast", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

	return 0;
}

/*设置逆变器的快速过压保护值*/
int set_overvoltage_fast_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xE2;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage fast", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

	return 0;
}

/*设置逆变器的慢速欠压保护值，有新老两种逆变器*/
int set_undervoltage_slow_one(struct inverter_info_t *firstinverter, char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;
	float temp;
	int voltage;

///////////////////////////////以下设置17项参数//////////////////////////////////
	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xF7;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage slow 17", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

//以下13项
	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	if(2 == get_inverter_type(id))
	{
		sendbuff[0] = 0xFB;			//HEAD
		sendbuff[1] = 0xFB;			//HEAD
		sendbuff[2] = 0x01;			//CMD
		sendbuff[3] = 0x00;			//LENGTH
		sendbuff[4] = 0x11;			//LENGTH
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
		sendbuff[20] = 0xE3;			//CMD
		sendbuff[21] = data;

		check = 0x00;
		for(i=2; i<22; i++){
			check = check + sendbuff[i];
		}

		sendbuff[22] = check >> 8;		//CHK
		sendbuff[23] = check;		//CHK
		sendbuff[24] = 0xFE;		//TAIL
		sendbuff[25] = 0xFE;		//TAIL

		printhexmsg("Set undervoltage slow", sendbuff, 26);

		write(plcmodem, sendbuff, 26);
		sleep(2);
	}

///////////////////////////////以下设置老参数//////////////////////////////////
	grid_recovery_time = get_grid_recovery_time_one(firstinverter, id);
	under_voltage_slow = get_under_voltage_slow_one(id);
	over_voltage_slow = get_over_voltage_slow_one(id);
	under_frequency_slow = get_under_frequency_slow_one(id);
	over_frequency_slow = get_over_frequency_slow_one(id);

	if(2 == caltype)
		temp = atof(value) * 2.93;
	else if(1 == caltype)
		temp = atof(value) * 2.90345;
	else
		temp = atof(value) * 1.48975;
	if((temp-(int)temp)>=0.5)
		voltage = temp + 1;
	else
		voltage = temp;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
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
	sendbuff[20] = 0xCC;			//CMD
	sendbuff[21] = 0xFF;
	sendbuff[22] = 0xFF;
	sendbuff[23] = 0xFF;
	sendbuff[24] = 0xFF;
	sendbuff[25] = voltage / 256;
	sendbuff[26] = voltage % 256;
	sendbuff[27] = over_voltage_slow / 256;
	sendbuff[28] = over_voltage_slow % 256;
	sendbuff[29] = under_frequency_slow;
	sendbuff[30] = over_frequency_slow;
	sendbuff[31] = grid_recovery_time / 256;
	sendbuff[32] = grid_recovery_time % 256;

	check = 0x00;
	for(i=2; i<33; i++){
		check = check + sendbuff[i];
	}

	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage slow (old)", sendbuff, 37);

	write(plcmodem, sendbuff, 37);
	sleep(2);

	return 0;
}

/*设置逆变器的慢速过压保护值，有新老两种逆变器*/
int set_overvoltage_slow_one(struct inverter_info_t *firstinverter, char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;
	float temp;
	int voltage;

	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xE4;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage slow", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

//////////////////////////////////////////////////////////////
	grid_recovery_time = get_grid_recovery_time_one(firstinverter, id);
	under_voltage_slow = get_under_voltage_slow_one(id);
	over_voltage_slow = get_over_voltage_slow_one(id);
	under_frequency_slow = get_under_frequency_slow_one(id);
	over_frequency_slow = get_over_frequency_slow_one(id);

	if(2 == caltype)
		temp = atof(value) * 2.93;
	else if(1 == caltype)
		temp = atof(value) * 2.90345;
	else
		temp = atof(value) * 1.48975;
	if((temp-(int)temp)>=0.5)
		voltage = temp + 1;
	else
		voltage = temp;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
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
	sendbuff[20] = 0xCC;			//CMD
	sendbuff[21] = 0xFF;
	sendbuff[22] = 0xFF;
	sendbuff[23] = 0xFF;
	sendbuff[24] = 0xFF;
	sendbuff[25] = under_voltage_slow / 256;
	sendbuff[26] = under_voltage_slow % 256;
	sendbuff[27] = voltage / 256;
	sendbuff[28] = voltage % 256;
	sendbuff[29] = under_frequency_slow;
	sendbuff[30] = over_frequency_slow;
	sendbuff[31] = grid_recovery_time / 256;
	sendbuff[32] = grid_recovery_time % 256;

	check = 0x00;
	for(i=2; i<33; i++){
		check = check + sendbuff[i];
	}

	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage slow (old)", sendbuff, 37);

	write(plcmodem, sendbuff, 37);
	sleep(2);

	return 0;
}

/*设置逆变器的快速欠频保护值*/
int set_underfrequency_fast_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype)){
		if((600 - (int)(atof(value)*10))<0)
			data = 0xFF;
		else
			data = 600 - (int)(atof(value)*10);
	}
	else{
		if((500 - (int)(atof(value)*10))<0)
			data = 0xFF;
		else
			data = 500 - (int)(atof(value)*10);
	}

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xE7;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency fast", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

	return 0;
}

/*设置逆变器的快速过频保护值*/
int set_overfrequency_fast_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype)){
		if(((int)(atof(value)*10)-600)<0)
			data = 0xFF;
		else
			data = (int)(atof(value)*10)-600;
	}
	else{
		if(((int)(atof(value)*10)-500)<0)
			data = 0xFF;
		else
			data = (int)(atof(value)*10)-500;
	}

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xE8;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency fast", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

	return 0;
}

/*设置逆变器的慢速欠频保护值，有新老两种逆变器*/
int set_underfrequency_slow_one(struct inverter_info_t *firstinverter, char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;

	if((2 == caltype)||(1 == caltype)){
		if((600 - (int)(atof(value)*10))<0)
			data = 0xFF;
		else
			data = 600 - (int)(atof(value)*10);
	}
	else{
		if((500 - (int)(atof(value)*10))<0)
			data = 0xFF;
		else
			data = 500 - (int)(atof(value)*10);
	}

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xE9;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency slow", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

//////////////////////////////////////////////////////////////
	grid_recovery_time = get_grid_recovery_time_one(firstinverter, id);
	under_voltage_slow = get_under_voltage_slow_one(id);
	over_voltage_slow = get_over_voltage_slow_one(id);
	under_frequency_slow = get_under_frequency_slow_one(id);
	over_frequency_slow = get_over_frequency_slow_one(id);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
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
	sendbuff[20] = 0xCC;			//CMD
	sendbuff[21] = 0xFF;
	sendbuff[22] = 0xFF;
	sendbuff[23] = 0xFF;
	sendbuff[24] = 0xFF;
	sendbuff[25] = under_voltage_slow / 256;
	sendbuff[26] = under_voltage_slow % 256;
	sendbuff[27] = over_voltage_slow / 256;
	sendbuff[28] = over_voltage_slow % 256;
	sendbuff[29] = data;
	sendbuff[30] = over_frequency_slow;
	sendbuff[31] = grid_recovery_time / 256;
	sendbuff[32] = grid_recovery_time % 256;

	check = 0x00;
	for(i=2; i<33; i++){
		check = check + sendbuff[i];
	}

	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency slow (old)", sendbuff, 37);

	write(plcmodem, sendbuff, 37);
	sleep(2);

	return 0;
}

/*设置逆变器的慢速过频保护值，有新老两种逆变器*/
int set_overfrequency_slow_one(struct inverter_info_t *firstinverter, char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;

	if((2 == caltype)||(1 == caltype)){
		if(((int)(atof(value)*10)-600)<0)
			data = 0xFF;
		else
			data = (int)(atof(value)*10)-600;
	}
	else{
		if(((int)(atof(value)*10)-500)<0)
			data = 0xFF;
		else
			data = (int)(atof(value)*10)-500;
	}

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xEA;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency slow", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

////////////////////////////////////////////////////////////
	grid_recovery_time = get_grid_recovery_time_one(firstinverter, id);
	under_voltage_slow = get_under_voltage_slow_one(id);
	over_voltage_slow = get_over_voltage_slow_one(id);
	under_frequency_slow = get_under_frequency_slow_one(id);
	over_frequency_slow = get_over_frequency_slow_one(id);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
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
	sendbuff[20] = 0xCC;			//CMD
	sendbuff[21] = 0xFF;
	sendbuff[22] = 0xFF;
	sendbuff[23] = 0xFF;
	sendbuff[24] = 0xFF;
	sendbuff[25] = under_voltage_slow / 256;
	sendbuff[26] = under_voltage_slow % 256;
	sendbuff[27] = over_voltage_slow / 256;
	sendbuff[28] = over_voltage_slow % 256;
	sendbuff[29] = under_frequency_slow;
	sendbuff[30] = data;
	sendbuff[31] = grid_recovery_time / 256;
	sendbuff[32] = grid_recovery_time % 256;

	check = 0x00;
	for(i=2; i<33; i++){
		check = check + sendbuff[i];
	}

	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency slow (old)", sendbuff, 37);

	write(plcmodem, sendbuff, 37);
	sleep(2);

	return 0;
}

int set_voltage_triptime_fast_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
	{
		if(atof(value)/0.0083-(int)(atof(value)/0.0083)>0.5)
			data = (int)(atof(value)/0.0083)+1;
		else
			data = (int)(atof(value)/0.0083);
	}
	else
	{
		if(atof(value)/0.01-(int)(atof(value)/0.01)>0.5)
			data = (int)(atof(value)/0.01)+1;
		else
			data = (int)(atof(value)/0.01);
	}

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xE5;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime fast 13", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

//以下17项参数
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xF5;			//CMD
	sendbuff[21] = change_1_clearance_time_to_char(atof(value));
	sendbuff[22] = 0x00;

	check = 0x00;
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime fast 17", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

	return 0;
}

int set_voltage_triptime_slow_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
	{
		if(atof(value)/1.07-(int)(atof(value)/1.07)>0.5)
			data = (int)(atof(value)/1.07)+1;
		else
			data = (int)(atof(value)/1.07);
	}
	else
	{
		if(atof(value)/1.28-(int)(atof(value)/1.28)>0.5)
			data = (int)(atof(value)/1.28)+1;
		else
			data = (int)(atof(value)/1.28);
	}

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xE6;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime slow 13", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

//以下17项
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xF6;			//CMD
	sendbuff[21] = change_2_clearance_time_to_char(atof(value));
	sendbuff[22] = 0x00;

	check = 0x00;
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime slow 17", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

	return 0;
}

int set_frequency_triptime_fast_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
	{
		if(atof(value)/0.033-(int)(atof(value)/0.033)>0.5)
			data = (int)(atof(value)/0.033)+1;
		else
			data = (int)(atof(value)/0.033);
	}
	else
	{
		if(atof(value)/0.04-(int)(atof(value)/0.04)>0.5)
			data = (int)(atof(value)/0.04)+1;
		else
			data = (int)(atof(value)/0.04);
	}

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xEB;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime fast 13", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

//17项参数
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xFB;			//CMD
	sendbuff[21] = change_1_clearance_time_to_char(atof(value));
	sendbuff[22] = 0x00;

	check = 0x00;
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime fast 17", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

	return 0;
}

int set_frequency_triptime_slow_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
	{
		if(atof(value)/1.07-(int)(atof(value)/1.07)>0.5)
			data = (int)(atof(value)/1.07)+1;
		else
			data = (int)(atof(value)/1.07);
	}
	else
	{
		if(atof(value)/1.28-(int)(atof(value)/1.28)>0.5)
			data = (int)(atof(value)/1.28)+1;
		else
			data = (int)(atof(value)/1.28);
	}

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xEC;			//CMD
	sendbuff[21] = data;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime slow 13", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

//以下17项
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xFC;			//CMD
	sendbuff[21] = change_2_clearance_time_to_char(atof(value));
	sendbuff[22] = 0x00;

	check = 0x00;
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime slow 17", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

	return 0;
}

/*设置逆变器的并网恢复时间，有新老两种逆变器*/
int set_grid_recovery_time_one(struct inverter_info_t *firstinverter, char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;

//以下17项
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xFD;			//CMD
	sendbuff[21] = change_start_time_to_char(atof(value));
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set grid recovery time", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

//以下13项
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
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
	sendbuff[20] = 0xED;			//CMD
	sendbuff[21] = atoi(value)/32;

	check=0x00;
	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set grid recovery time", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

///////////////////////////////////////////////////////
	under_voltage_slow = get_under_voltage_slow_one(id);
	over_voltage_slow = get_over_voltage_slow_one(id);
	under_frequency_slow = get_under_frequency_slow_one(id);
	over_frequency_slow = get_over_frequency_slow_one(id);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
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
	sendbuff[20] = 0xCC;			//CMD
	sendbuff[21] = 0xFF;
	sendbuff[22] = 0xFF;
	sendbuff[23] = 0xFF;
	sendbuff[24] = 0xFF;
	sendbuff[25] = under_voltage_slow / 256;
	sendbuff[26] = under_voltage_slow % 256;
	sendbuff[27] = over_voltage_slow / 256;
	sendbuff[28] = over_voltage_slow % 256;
	sendbuff[29] = under_frequency_slow;
	sendbuff[30] = over_frequency_slow;
	sendbuff[31] = atoi(value) / 256;
	sendbuff[32] = atoi(value) % 256;

	check = 0x00;
	for(i=2; i<33; i++){
		check = check + sendbuff[i];
	}

	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printhexmsg("Set grid recovery slow (old)", sendbuff, 37);

	write(plcmodem, sendbuff, 37);
	sleep(2);

	return 0;
}

int set_under_voltage_stage_2_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

//17项参数，未修改指令的逆变器
	if(3 == get_inverter_type(id))
	{
		sendbuff[0] = 0xFB;			//HEAD
		sendbuff[1] = 0xFB;			//HEAD
		sendbuff[2] = 0x01;			//CMD
		sendbuff[3] = 0x00;			//LENGTH
		sendbuff[4] = 0x12;			//LENGTH
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
		sendbuff[20] = 0xE3;			//CMD
		sendbuff[21] = data;
		sendbuff[22] = 0x00;

		for(i=2; i<23; i++){
			check = check + sendbuff[i];
		}

		sendbuff[23] = check >> 8;		//CHK
		sendbuff[24] = check;		//CHK
		sendbuff[25] = 0xFE;		//TAIL
		sendbuff[26] = 0xFE;		//TAIL

		printhexmsg("Set under voltage stage 2", sendbuff, 27);

		write(plcmodem, sendbuff, 27);
		sleep(2);
	}

//17项参数，修改指令的逆变器
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0x53;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	check=0x00;
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set under voltage stage 2", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

	return 0;
}

int set_voltage_3_clearance_time_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xF8;			//CMD
	sendbuff[21] = change_2_clearance_time_to_char(atof(value));
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set voltage 3 clearance time", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

	return 0;
}

int set_regulated_dc_working_point_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data[2];

	data[0] = ((int)(atof(value)*4096/82.5))/256;
	data[1] = ((int)(atof(value)*4096/82.5))%256;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xF3;			//CMD
	sendbuff[21] = data[0];
	sendbuff[22] = data[1];

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set regulated dc working point", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

	return 0;
}

int set_start_time_one(char *id, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
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
	sendbuff[20] = 0xF9;			//CMD
	sendbuff[21] = change_start_time_to_char(atof(value));
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set start time", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(2);

	return 0;
}

/*设置完后，清除设置标志*/
int clear_flag_one(char *id, char *para_name)					//设置后清除数据库中参数的设置标志
{
	int i;
	sqlite3 *db=NULL;
	char sql[1024];
	char *zErrMsg = 0;

	sprintf(sql, "DELETE FROM set_protection_parameters_inverter WHERE id='%s' AND parameter_name='%s'", id, para_name);

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	for(i=0; i<3; i++){
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
			break;
		else
			print2msg("Failed to clear protection set flag", zErrMsg);
	}

	sqlite3_close( db );

	return 0;
}

/*EMA读取逆变器的参数*/
//int read_protect_parameters(int plcmodem, char *ccuid, struct inverter_info_t *firstinverter, int type)		//读取逆变器的预设值
//{
//	int i, j, res;		//发送次数
//	char readpresetdata[20]={'\0'};
//	struct inverter_info_t *curinverter = firstinverter;
//
//	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
//	{
//		for(j=0; j<3; j++)
//		{
//			res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);			//读取失败标志为‘B’
//			if(-1 == res){
//				set_no_presetdata(curinverter);
//			}
//			else						//读取成功，且与页面上输入的值相等
//			{
//				resolve_presetdata(curinverter, readpresetdata, type);
//				break;
//			}
//		}
//	}
//
//	display_presetdata(firstinverter);		//在页面上显示AB类
//	save_protect_result(firstinverter);
//
//	return 0;
//}

/*给逆变器设置保护参数，并且读取逆变器设置后的保护参数*/
int set_protection_paras_one(struct inverter_info_t *firstinverter, char *inverter_id)
{
	char para_name[64];
	char value[16];

	if(1 == get_value_flag_one(inverter_id, para_name, value))
	{
		if(!strcmp("under_voltage_fast", para_name))
		{
			set_undervoltage_fast_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("over_voltage_fast", para_name))
		{
			set_overvoltage_fast_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("under_voltage_slow", para_name))
		{
			set_undervoltage_slow_one(firstinverter, inverter_id,  value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("over_voltage_slow", para_name))
		{
			set_overvoltage_slow_one(firstinverter, inverter_id,  value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("under_frequency_fast", para_name))
		{
			set_underfrequency_fast_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("over_frequency_fast", para_name))
		{
			set_overfrequency_fast_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("under_frequency_slow", para_name))
		{
			set_underfrequency_slow_one(firstinverter, inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("over_frequency_slow", para_name))
		{
			set_overfrequency_slow_one(firstinverter, inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("voltage_triptime_fast", para_name))
		{
			set_voltage_triptime_fast_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("voltage_triptime_slow", para_name))
		{
			set_voltage_triptime_slow_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("frequency_triptime_fast", para_name))
		{
			set_frequency_triptime_fast_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("frequency_triptime_slow", para_name))
		{
			set_frequency_triptime_slow_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("grid_recovery_time", para_name))
		{
			set_grid_recovery_time_one(firstinverter, inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("under_voltage_stage_2", para_name))
		{
			set_under_voltage_stage_2_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("voltage_3_clearance_time", para_name))
		{
			set_voltage_3_clearance_time_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("regulated_dc_working_point", para_name))
		{
			set_regulated_dc_working_point_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("start_time", para_name))
		{
			set_start_time_one(inverter_id, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}

		return 1;
	}

	return 0;
}

/*解析参数，并保存到数据库中*/
/*int resolve_protection_paras(struct inverter_info_t *inverter, char *readbuff, int size)
{
	sqlite3 *db=NULL;
	char sql[1024];
	char inverter_result[65535]={'\0'};
	char *zErrMsg = 0;
	int i, max_power;

	int under_voltage_fast;
	int over_voltage_fast;
	int under_voltage_slow;
	int over_voltage_slow;
	float under_frequency_fast;
	float over_frequency_fast;
	float under_frequency_slow;
	float over_frequency_slow;
	float voltage_triptime_fast;
	float voltage_triptime_slow;
	float frequency_triptime_fast;
	float frequency_triptime_slow;
	int grid_recovery_time;
	float temp;

	if(42 == size){		//13项参数
		if((2 == caltype)||(1 == caltype))
			temp = readbuff[24]*4/1.47;
		else
			temp = readbuff[24]*4/1.51;

		if((temp-(int)temp)>=0.5)
			under_voltage_fast = (int)temp +1;
		else
			under_voltage_fast = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[25]*4/1.47;
		else
			temp = readbuff[25]*4/1.51;

		if((temp-(int)temp)>=0.5)
			over_voltage_fast = (int)temp +1;
		else
			over_voltage_fast = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[26]*4/1.47;
		else
			temp = readbuff[26]*4/1.51;

		if((temp-(int)temp)>0.5)
			under_voltage_slow = (int)temp +1;
		else
			under_voltage_slow = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[27]*4/1.47;
		else
			temp = readbuff[27]*4/1.51;

		if((temp-(int)temp)>0.5)
			over_voltage_slow = (int)temp +1;
		else
			over_voltage_slow = (int)temp;

		if((2 == caltype)||(1 == caltype))
			under_frequency_fast = (600 - readbuff[28])/10.0;
		else
			under_frequency_fast = (500 - readbuff[28])/10.0;

		if((2 == caltype)||(1 == caltype))
			over_frequency_fast = (600 + readbuff[29])/10.0;
		else
			over_frequency_fast = (500 + readbuff[29])/10.0;

		if((2 == caltype)||(1 == caltype))
			under_frequency_slow = (600 - readbuff[30])/10.0;
		else
			under_frequency_slow = (500 - readbuff[30])/10.0;

		if((2 == caltype)||(1 == caltype))
			over_frequency_slow = (600 + readbuff[31])/10.0;
		else
			over_frequency_slow = (500 + readbuff[31])/10.0;

		if((2 == caltype)||(1 == caltype))
			voltage_triptime_fast = readbuff[32]*0.033;
		else
			voltage_triptime_fast = readbuff[32]*0.04;

		if((2 == caltype)||(1 == caltype))
			voltage_triptime_slow = readbuff[33]*1.07;
		else
			voltage_triptime_slow = readbuff[33]*1.28;

		if((2 == caltype)||(1 == caltype))
			frequency_triptime_fast = readbuff[34]*0.033;
		else
			frequency_triptime_fast = readbuff[34]*0.04;

		if((2 == caltype)||(1 == caltype))
			frequency_triptime_slow = readbuff[35]*1.07;
		else
			frequency_triptime_slow = readbuff[35]*1.28;

		if(9 == readbuff[36])
			grid_recovery_time = 300;
		else
			grid_recovery_time = readbuff[36]*32;

		if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
				return -1;
		strcpy(sql,"CREATE TABLE IF NOT EXISTS protection_parameters(id VARCHAR(15), type INTEGER, under_voltage_fast INTEGER, over_voltage_fast INTEGER, under_voltage_slow INTEGER, over_voltage_slow INTEGER, under_frequency_fast REAL, over_frequency_fast REAL, under_frequency_slow REAL, over_frequency_slow REAL, voltage_triptime_fast REAL, voltage_triptime_slow REAL, frequency_triptime_fast REAL, frequency_triptime_slow REAL, grid_recovery_time INTEGER, set_flag INTEGER, primary key(id));");	//create data table
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		sprintf(sql, "REPLACE INTO protection_parameters (id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time) VALUES('%s', %d, %d, %d, %d, %f, %f, %f, %f, %f, %f, %f, %f, %d) ", inverter->inverterid, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time);
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		max_power = (readbuff[22] << 16) / 7395;
		sprintf(sql,"UPDATE power SET limitedresult=%d WHERE id=%s ", max_power, inverter->inverterid);
		sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

		sqlite3_close( db );

		sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAA%03d%03d%03d%03d%02d%04d%02d%04dEND", inverter->inverterid, under_voltage_slow, over_voltage_slow, (int)(under_frequency_slow*10), (int)(over_frequency_slow*10), grid_recovery_time, under_voltage_fast, over_voltage_fast, (int)(under_frequency_fast*10), (int)(over_frequency_fast*10), (int)(voltage_triptime_fast*100), (int)(voltage_triptime_slow*100), (int)(frequency_triptime_fast*100), (int)(frequency_triptime_slow*100));
		save_inverter_parameters_result(inverter, 121, inverter_result);

		return 0;
	}
	if(37 == size){		//5项参数
		if(2 == caltype)
			temp = (readbuff[25]*256 + readbuff[26])/2.93;
		else if(1 == caltype)
			temp = (readbuff[25]*256 + readbuff[26])/2.90345;
		else
			temp = (readbuff[25]*256 + readbuff[26])/1.48975;
		if((temp-(int)temp)>0.5)
			under_voltage_slow = (int)temp +1;
		else
			under_voltage_slow = (int)temp;

		if(2 == caltype)
			temp = (readbuff[27]*256 + readbuff[28])/2.93;
		else if(1 == caltype)
			temp = (readbuff[27]*256 + readbuff[28])/2.90345;
		else
			temp = (readbuff[27]*256 + readbuff[28])/1.48975;
		if((temp-(int)temp)>0.5)
			over_voltage_slow = (int)temp +1;
		else
			over_voltage_slow = (int)temp;

		if(caltype)
			under_frequency_slow = (600-readbuff[29])/10.0;
		else
			under_frequency_slow = (500-readbuff[29])/10.0;

		if(caltype)
			over_frequency_slow = (600+readbuff[30])/10.0;
		else
			over_frequency_slow = (500+readbuff[30])/10.0;

		grid_recovery_time = readbuff[31]*256 + readbuff[32];

		if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
				return -1;
		strcpy(sql,"CREATE TABLE IF NOT EXISTS protection_parameters(id VARCHAR(15), type INTEGER, under_voltage_fast INTEGER, over_voltage_fast INTEGER, under_voltage_slow INTEGER, over_voltage_slow INTEGER, under_frequency_fast REAL, over_frequency_fast REAL, under_frequency_slow REAL, over_frequency_slow REAL, voltage_triptime_fast REAL, voltage_triptime_slow REAL, frequency_triptime_fast REAL, frequency_triptime_slow REAL, grid_recovery_time INTEGER, set_flag INTEGER, primary key(id));");	//create data table
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		sprintf(sql, "REPLACE INTO protection_parameters (id, under_voltage_slow, over_voltage_slow, under_frequency_slow, over_frequency_slow, grid_recovery_time) VALUES('%s', %d, %d, %f, %f, %d) ", inverter->inverterid, under_voltage_slow, over_voltage_slow, under_frequency_slow, over_frequency_slow, grid_recovery_time);
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		sqlite3_close( db );

		sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEND", inverter->inverterid, under_voltage_slow, over_voltage_slow, (int)(under_frequency_slow*10), (int)(over_frequency_slow*10), grid_recovery_time);
		save_inverter_parameters_result(inverter, 121, inverter_result);

		return 0;
	}
}*/

/*向逆变器读取设置的值*/
/*int get_parameters_from_inverter(struct inverter_info_t *inverter)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;
	fd_set rd;
	struct timeval timeout;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
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
	sendbuff[21] = 0x11;

	for(i=2; i<22; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	write(plcmodem, sendbuff, 26);

	res = get_reply_from_inverter(inverter, readbuff);

	if((42 == res) &&
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0x01 == readbuff[2]) &&
			(0x00 == readbuff[3]) &&
			(0x21 == readbuff[4]) &&
			(ccuid[0] == readbuff[5]) &&
			(ccuid[1] == readbuff[6]) &&
			(ccuid[2] == readbuff[7]) &&
			(ccuid[3] == readbuff[8]) &&
			(ccuid[4] == readbuff[9]) &&
			(ccuid[5] == readbuff[10]) &&
			(inverter->tnuid[0] == readbuff[11]) &&
			(inverter->tnuid[1] == readbuff[12]) &&
			(inverter->tnuid[2] == readbuff[13]) &&
			(inverter->tnuid[3] == readbuff[14]) &&
			(inverter->tnuid[4] == readbuff[15]) &&
			(inverter->tnuid[5] == readbuff[16]) &&
			(0x4F == readbuff[17]) &&
			(0x00 == readbuff[18]) &&
			(0x00 == readbuff[19]) &&
			(0xDA == readbuff[20]) &&
			(0xFE == readbuff[40]) &&
			(0xFE == readbuff[41])){
		resolve_protection_paras(inverter, readbuff, res);
		return 0;
	}
	else if((37 == res) &&
			(0xFB == readbuff[0])&&
			(0xFB == readbuff[1])&&
			(0x01 == readbuff[2])&&
			(0x00 == readbuff[3])&&
			(0x1C == readbuff[4])&&
			(ccuid[0] == readbuff[5])&&
			(ccuid[1] == readbuff[6])&&
			(ccuid[2] == readbuff[7])&&
			(ccuid[3] == readbuff[8])&&
			(ccuid[4] == readbuff[9])&&
			(ccuid[5] == readbuff[10])&&
			(inverter->tnuid[0] == readbuff[11])&&
			(inverter->tnuid[1] == readbuff[12])&&
			(inverter->tnuid[2] == readbuff[13])&&
			(inverter->tnuid[3] == readbuff[14])&&
			(inverter->tnuid[4] == readbuff[15])&&
			(inverter->tnuid[5] == readbuff[16])&&
			(0x4F == readbuff[17])&&
			(0x00 == readbuff[18])&&
			(0x00 == readbuff[19])&&
			(0xDD == readbuff[20])&&
			(0xFE == readbuff[35])&&
			(0xFE == readbuff[36])){
		resolve_protection_paras(inverter, readbuff, res);
		return 0;
	}
	else
		return -1;
}*/

/*读取每一台逆变器的保护参数*/
int read_protection_parameters_one(struct inverter_info_t *firstinverter, char *id)
{
	int i, j, k;
	int result;
	struct inverter_info_t *inverter = firstinverter;

	memset(set_result, '\0', sizeof(set_result));

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		if(!strcmp(inverter->inverterid, id)){
			for(j=0; j<3; j++){
				result = get_parameters_from_inverter(inverter);
				if(result > 0){
					if(3 == result) {
						for(k=0; k<3; k++)
							if(!get_parameters_from_inverter_de(inverter->inverterid))
								break;
					}
					break;
				}
			}
		}
	}

	return 0;
}

/*设置逆变器的保护参数*/
int set_protection_parameters_inverter(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	char buff[16];
	int flag;
	char inverter_id[16];
	
	while(1){
		if(1 != set_protection_paras_one(firstinverter, inverter_id))
			break;
		else
			read_protection_parameters_one(firstinverter, inverter_id);
	}

//	if(set_protection_paras(firstinverter) > 0)
//		read_protection_parameters(firstinverter);

//	fp = fopen("/tmp/presetdata.conf", "r");
//	if(fp)
//	{
//		fgets(buff, 255, fp);
//		fclose(fp);
//
//		if(!strlen(buff))
//			flag = 0;
//		if('0' == buff[0])
//			flag = 0;
//		if('1' == buff[0])
//			flag = 1;
//		if('2' == buff[0])
//			flag = 2;

//		if(1 == flag){
//printmsg("debug1");
//			set_protection_paras();
//			read_protection_parameters(firstinverter);

//			fp = fopen("/tmp/presetdata.conf", "w");
//			fprintf(fp, "0");
//			fclose(fp);
//		}
//		if(2 == flag)
//		{
//			read_protection_parameters(firstinverter);
//
//			fp = fopen("/tmp/presetdata.conf", "w");
//			fprintf(fp, "0");
//			fclose(fp);
//		}
//	}
	
//printmsg("debug7");

	return 0;
}
