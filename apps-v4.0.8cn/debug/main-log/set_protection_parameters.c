/*
 * 本文件的主要功能是设置逆变器的保护参数。同时兼容新的13项保护参数和老的5项参数。
 * Created by Zhyf
 * Created on 2014/04/24
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include "sqlite3.h"
#include "variation.h"
#include "database.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID
extern int caltype;		//计算方式，NA版和非NA版的区别
char set_result[65535];

typedef struct protection_parameters_t{
	int under_voltage_fast;
	int over_voltage_fast;
	int under_voltage_stage_2;
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
}protection_parameters;

protection_parameters paras;

int reset_protection_parameters()
{
	paras.under_voltage_fast = 0;
	paras.over_voltage_fast = 0;
	paras.under_voltage_stage_2 = 0;
	paras.over_voltage_slow = 0;
	paras.under_frequency_fast = 0;
	paras.over_frequency_fast = 0;
	paras.under_frequency_slow = 0;
	paras.over_frequency_slow = 0;
	paras.voltage_triptime_fast = 0;
	paras.voltage_triptime_slow = 0;
	paras.frequency_triptime_fast = 0;
	paras.frequency_triptime_slow = 0;
	paras.grid_recovery_time = 0;

	return 0;
}

/*读取逆变器的返回结果*/
int get_reply_from_inverter(struct inverter_info_t *inverter, char *data)			//读取逆变器的返回帧
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
}

/*读取设置的参数值和标志位*/
int get_value_flag(char *para_name, char *value)
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

	strcpy(sql, "SELECT parameter_name,parameter_value FROM set_protection_parameters WHERE set_flag=1 LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		strcpy(para_name, azResult[2]);
		strcpy(value, azResult[3]);
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	if(1 == nrow)
		return 1;
	else
		return 0;
}

int get_under_voltage_slow(struct inverter_info_t *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int under_voltage_slow=0xFFFF, res, i;
	struct inverter_info_t *inverter = firstinverter;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='under_voltage_slow'");
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
	}

	return under_voltage_slow;
}

int get_over_voltage_slow(struct inverter_info_t *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int over_voltage_slow=0xFFFF, res, i;
	struct inverter_info_t *inverter = firstinverter;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='over_voltage_slow'");
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
	}

	return over_voltage_slow;
}

char get_under_frequency_slow(struct inverter_info_t *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char under_frequency_slow=0xFF, res, i;
	struct inverter_info_t *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='under_frequency_slow'");
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
	}

	return under_frequency_slow;
}

char get_over_frequency_slow(struct inverter_info_t *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char over_frequency_slow=0xFF, res, i;
	struct inverter_info_t *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res){		//create a database
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='over_frequency_slow'");
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
	}

	return over_frequency_slow;
}

int get_recovery_time_from_inverter(struct inverter_info_t *inverter)
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
}

int get_grid_recovery_time(struct inverter_info_t *firstinverter)
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

char change_1_clearance_time_to_char(float time)		//外围过欠压、过欠频延保时间转成char型
{
	float temp;
	char result;

	if(caltype)		//60Hz环境
	{
		if(time>0.8333)
			temp = (time*60.0+6400-50)/64;
		else
			temp = time*60.0*2;
	}
	else			//50Hz环境
	{
		if(time>1)
			temp = (time*50.0+6400-50)/64;
		else
			temp = time*50.0*2;
	}

	result = temp;

	return result;
}

char change_2_clearance_time_to_char(float time)		//内围过欠压、内围过欠频、内内围欠压延保时间转成char型
{
	float temp;
	char result;

	if(caltype)		//60Hz环境
	{
		if(time>29.8)
			temp = (time*60.0-120*0.5-54*32)/512 + 174;
		else if(time>1)
			temp = (time*60.0-60)*2/64 + 120;
		else
			temp = time*60.0*2;
	}
	else			//50Hz环境
	{
		if(time>35.76)
			temp = (time*50.0-120*0.5-54*32)/512 + 174;
		else if(time>1.2)
			temp = (time*50.0-60)*2/64 + 120;
		else
			temp = time*50.0*2;
	}

	result = temp;

	return result;
}

char change_start_time_to_char(float time)		//并网恢复时间、开机时间转成char型
{
	float temp;
	char result;

	if(caltype)
	{
		if(time>34.144)
			temp = (time*60.0*2-32*128)/512 + 32;
		else
			temp = time*60.0*2/128;
	}
	else
	{
		if(time>40.96)
			temp = (time*50.0*2-32*128)/512 + 32;
		else
			temp = time*50.0*2/128;
	}

	result = temp;

	return result;
}

float change_1_clearance_time_from_char(char time)		//外围过欠压、过欠频延保时间转成float型
{
	float result;

	if(time>100)
	{
		if(caltype)		//60Hz
		{
			result = (time-100)*64*(1/60.0) + 50*(1/60.0);
		}
		else			//50Hz
		{
			result = (time-100)*64*(1/50.0) + 50*(1/50.0);
		}
	}
	else if(time>0)
	{
		if(caltype)
		{
			result = time*(1/60.0)*0.5;
		}
		else
		{
			result = time*(1/50.0)*0.5;
		}
	}
	else
		result = 0.0;

	return result;
}

float change_2_clearance_time_from_char(char time)		//内围过欠压、内围过欠频、内内围欠压延保时间转成float型
{
	float result;

	if(time>=175)
	{
		if(caltype)
			result = 120*(1/60.0)*0.5 + 54*32*(1/60.0) + (time-174)*512*(1/60.0);
		else
			result = 120*(1/50.0)*0.5 + 54*32*(1/50.0) + (time-174)*512*(1/50.0);
	}
	else if(time>120)
	{
		if(caltype)
			result = 60*(1/60.0) + (time-120)*64*(1/60.0)*0.5;
		else
			result = 60*(1/50.0) + (time-120)*64*(1/50.0)*0.5;
	}
	else if(time>0)
	{
		if(caltype)
			result = time*(1/60.0)*0.5;
		else
			result = time*(1/50.0)*0.5;
	}
	else
		result = 0.0;

	return result;
}

float change_start_time_from_char(char time)		//并网恢复时间、开机时间转成float型
{
	float result;

	if(time>32)
	{
		if(caltype)
			result = (time-32)*512*(1/60.0)*0.5 + 32*128*(1/60.0)*0.5;
		else
			result = (time-32)*512*(1/50.0)*0.5 + 32*128*(1/50.0)*0.5;
	}
	else if(time>0)
	{
		if(caltype)
			result = time*128*(1/60.0)*0.5;
		else
			result = time*128*(1/50.0)*0.5;
	}
	else
		result = 0.0;

	return result;
}

/*设置逆变器的快速欠压保护值*/
int set_undervoltage_fast(char *value)
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xE1;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage fast", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

	return 0;
}

/*设置逆变器的快速过压保护值*/
int set_overvoltage_fast(char *value)
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xE2;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage fast", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

	return 0;
}

/*设置逆变器的慢速欠压保护值，有新老两种逆变器*/
int set_undervoltage_slow(struct inverter_info_t *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;
	float temp;
	int voltage;
	struct inverter_info_t *inverter = firstinverter;

///////////////////////////////以下设置17项参数//////////////////////////////////
	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

/////////////////////////////以下设置13项参数//////////////////////////////////
	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		if(2 == get_inverter_type(inverter->inverterid))
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
			sendbuff[11] = ((inverter->inverterid[0]-0x30)<<4) + (inverter->inverterid[1]-0x30);		//TNID
			sendbuff[12] = ((inverter->inverterid[2]-0x30)<<4) + (inverter->inverterid[3]-0x30);		//TNID
			sendbuff[13] = ((inverter->inverterid[4]-0x30)<<4) + (inverter->inverterid[5]-0x30);		//TNID
			sendbuff[14] = ((inverter->inverterid[6]-0x30)<<4) + (inverter->inverterid[7]-0x30);		//TNID
			sendbuff[15] = ((inverter->inverterid[8]-0x30)<<4) + (inverter->inverterid[9]-0x30);		//TNID
			sendbuff[16] = ((inverter->inverterid[10]-0x30)<<4) + (inverter->inverterid[11]-0x30);		//TNID
			sendbuff[17] = 0x4F;
			sendbuff[18] = 0x00;
			sendbuff[19] = 0x00;
			sendbuff[20] = 0xE3;			//CMD
			sendbuff[21] = data;
			sendbuff[22] = 0x00;

			check = 0x00;
			for(i=2; i<23; i++){
				check = check + sendbuff[i];
			}

			sendbuff[23] = check >> 8;		//CHK
			sendbuff[24] = check;		//CHK
			sendbuff[25] = 0xFE;		//TAIL
			sendbuff[26] = 0xFE;		//TAIL

			printhexmsg("Set undervoltage slow 13", sendbuff, 27);

			write(plcmodem, sendbuff, 27);
			sleep(2);
		}
	}

///////////////////////////////以下设置老参数//////////////////////////////////
	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow(firstinverter);
	over_voltage_slow = get_over_voltage_slow(firstinverter);
	under_frequency_slow = get_under_frequency_slow(firstinverter);
	over_frequency_slow = get_over_frequency_slow(firstinverter);

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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
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

	printhexmsg("Set undervoltage slow 5", sendbuff, 37);

	write(plcmodem, sendbuff, 37);
	sleep(10);

	return 0;
}

/*设置逆变器的慢速过压保护值，有新老两种逆变器*/
int set_overvoltage_slow(struct inverter_info_t *firstinverter, char *value)
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xE4;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage slow", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

//////////////////////////////////////////////////////////////
	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow(firstinverter);
	over_voltage_slow = get_over_voltage_slow(firstinverter);
	under_frequency_slow = get_under_frequency_slow(firstinverter);
	over_frequency_slow = get_over_frequency_slow(firstinverter);

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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
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
	sleep(10);

	return 0;
}

/*设置逆变器的快速欠频保护值*/
int set_underfrequency_fast(char *value)
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xE7;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency fast", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

	return 0;
}

/*设置逆变器的快速过频保护值*/
int set_overfrequency_fast(char *value)
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xE8;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency fast", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

	return 0;
}

/*设置逆变器的慢速欠频保护值，有新老两种逆变器*/
int set_underfrequency_slow(struct inverter_info_t *firstinverter, char *value)
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xE9;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency slow", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

//////////////////////////////////////////////////////////////
	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow(firstinverter);
	over_voltage_slow = get_over_voltage_slow(firstinverter);
	under_frequency_slow = get_under_frequency_slow(firstinverter);
	over_frequency_slow = get_over_frequency_slow(firstinverter);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
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
	sleep(10);

	return 0;
}

/*设置逆变器的慢速过频保护值，有新老两种逆变器*/
int set_overfrequency_slow(struct inverter_info_t *firstinverter, char *value)
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xEA;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency slow", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

////////////////////////////////////////////////////////////
	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow(firstinverter);
	over_voltage_slow = get_over_voltage_slow(firstinverter);
	under_frequency_slow = get_under_frequency_slow(firstinverter);
	over_frequency_slow = get_over_frequency_slow(firstinverter);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
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
	sleep(10);

	return 0;
}

int set_voltage_triptime_fast(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

//以下13项参数
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xE5;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime fast 13", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

//以下17项参数
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

	return 0;
}

int set_voltage_triptime_slow(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

//以下13项
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xE6;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime slow 13", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

//以下17项
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

	return 0;
}

int set_frequency_triptime_fast(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

//13项参数
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xEB;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime fast 13", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

//17项参数
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

	return 0;
}

int set_frequency_triptime_slow(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

//以下13项
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
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xEC;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime slow 13", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

//以下17项
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

	return 0;
}

/*设置逆变器的并网恢复时间，有新老两种逆变器*/
int set_grid_recovery_time(struct inverter_info_t *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;

//以下17项
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

//以下13项
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xED;			//CMD
	sendbuff[21] = atoi(value)/32;
	sendbuff[22] = 0x00;

	check = 0x00;
	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set grid recovery time", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

///////////////////////////////////////////////////////
	under_voltage_slow = get_under_voltage_slow(firstinverter);
	over_voltage_slow = get_over_voltage_slow(firstinverter);
	under_frequency_slow = get_under_frequency_slow(firstinverter);
	over_frequency_slow = get_over_frequency_slow(firstinverter);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
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
	sleep(10);

	return 0;
}

int set_under_voltage_stage_2(struct inverter_info_t *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;
	struct inverter_info_t *inverter = firstinverter;

	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

//17项参数，未修改指令的逆变器
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		if(3 == get_inverter_type(inverter->inverterid))
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
			sendbuff[11] = ((inverter->inverterid[0]-0x30)<<4) + (inverter->inverterid[1]-0x30);		//TNID
			sendbuff[12] = ((inverter->inverterid[2]-0x30)<<4) + (inverter->inverterid[3]-0x30);		//TNID
			sendbuff[13] = ((inverter->inverterid[4]-0x30)<<4) + (inverter->inverterid[5]-0x30);		//TNID
			sendbuff[14] = ((inverter->inverterid[6]-0x30)<<4) + (inverter->inverterid[7]-0x30);		//TNID
			sendbuff[15] = ((inverter->inverterid[8]-0x30)<<4) + (inverter->inverterid[9]-0x30);		//TNID
			sendbuff[16] = ((inverter->inverterid[10]-0x30)<<4) + (inverter->inverterid[11]-0x30);		//TNID
			sendbuff[17] = 0x4F;
			sendbuff[18] = 0x00;
			sendbuff[19] = 0x00;
			sendbuff[20] = 0xE3;			//CMD
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
		}
	}

//17项参数，修改指令的逆变器
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0x00;
	sendbuff[14] = 0x00;
	sendbuff[15] = 0x00;
	sendbuff[16] = 0x00;
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
	sleep(10);

	return 0;
}

int set_voltage_3_clearance_time(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

	return 0;
}

int set_regulated_dc_working_point(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data[2];

	data[0] = ((int)(atof(value)*4096/82.5))/256;
	data[1] = ((int)(atof(value)*4096/82.5))%256;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

	return 0;
}

int set_start_time(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x12;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
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
	sleep(10);

	return 0;
}

/*设置完后，清除设置标志*/
int clear_flag(char *para_name)					//设置后清除数据库中参数的设置标志
{
	int i;
	sqlite3 *db=NULL;
	char sql[1024];
	char *zErrMsg = 0;

	sprintf(sql, "UPDATE set_protection_parameters SET set_flag=0 WHERE parameter_name='%s'", para_name);

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
int set_protection_paras(struct inverter_info_t *firstinverter)
{
	char para_name[64];
	char value[16];
	int count = 0;

	while(1 == get_value_flag(para_name, value))
	{
		count++;
		if(!strcmp("under_voltage_fast", para_name))
		{
			set_undervoltage_fast(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("over_voltage_fast", para_name))
		{
			set_overvoltage_fast(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("under_voltage_slow", para_name))
		{
			set_undervoltage_slow(firstinverter, value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("over_voltage_slow", para_name))
		{
			set_overvoltage_slow(firstinverter, value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("under_frequency_fast", para_name))
		{
			set_underfrequency_fast(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("over_frequency_fast", para_name))
		{
			set_overfrequency_fast(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("under_frequency_slow", para_name))
		{
			set_underfrequency_slow(firstinverter, value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("over_frequency_slow", para_name))
		{
			set_overfrequency_slow(firstinverter, value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("voltage_triptime_fast", para_name))
		{
			set_voltage_triptime_fast(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("voltage_triptime_slow", para_name))
		{
			set_voltage_triptime_slow(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("frequency_triptime_fast", para_name))
		{
			set_frequency_triptime_fast(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("frequency_triptime_slow", para_name))
		{
			set_frequency_triptime_slow(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("grid_recovery_time", para_name))
		{
			set_grid_recovery_time(firstinverter, value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("under_voltage_stage_2", para_name))
		{
			set_under_voltage_stage_2(firstinverter, value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("voltage_3_clearance_time", para_name))
		{
			set_voltage_3_clearance_time(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("regulated_dc_working_point", para_name))
		{
			set_regulated_dc_working_point(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("start_time", para_name))
		{
			set_start_time(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else
			break;
	}

	return count;
}

/*解析参数，并保存到数据库中*/
int resolve_protection_paras(struct inverter_info_t *inverter, char *readbuff, int size)
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
			temp = readbuff[24]*4/1.452;
		else
			temp = readbuff[24]*4/1.48975;

		if((temp-(int)temp)>=0.5)
			under_voltage_fast = (int)temp +1;
		else
			under_voltage_fast = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[25]*4/1.452;
		else
			temp = readbuff[25]*4/1.48975;

		if((temp-(int)temp)>=0.5)
			over_voltage_fast = (int)temp +1;
		else
			over_voltage_fast = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[26]*4/1.452;
		else
			temp = readbuff[26]*4/1.48975;

		if((temp-(int)temp)>0.5)
			under_voltage_slow = (int)temp +1;
		else
			under_voltage_slow = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[27]*4/1.452;
		else
			temp = readbuff[27]*4/1.48975;

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
			voltage_triptime_fast = readbuff[32]*0.0083;
		else
			voltage_triptime_fast = readbuff[32]*0.01;

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

		if((0 == (int)(voltage_triptime_fast)) && (0 == (int)(voltage_triptime_slow)) && (0 == (int)(frequency_triptime_fast)) &&
				(0 == (int)(frequency_triptime_slow)))
		{
			reset_protection_parameters();
			paras.under_voltage_fast = under_voltage_fast;
			paras.over_voltage_fast = over_voltage_fast;
			paras.under_voltage_stage_2 = under_voltage_slow;
			paras.over_voltage_slow = over_voltage_slow;
			paras.under_frequency_fast = under_frequency_fast;
			paras.over_frequency_fast = over_frequency_fast;
			paras.under_frequency_slow = under_frequency_slow;
			paras.over_frequency_slow = over_frequency_slow;

//			sprintf(sql, "REPLACE INTO protection_parameters (id, under_voltage_fast, over_voltage_fast, "
//					"under_voltage_stage_2, over_voltage_slow, under_frequency_fast, over_frequency_fast, "
//					"under_frequency_slow, over_frequency_slow) VALUES('%s', %d, %d, %d, %d, %f, %f, %f, %f) ",
//					inverter->inverterid, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow,
//					under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow);
//			sqlite3_exec_3times(db, sql);

			return 3;
		}
		else
		{
			sprintf(sql, "REPLACE INTO protection_parameters (id, under_voltage_fast, over_voltage_fast, under_voltage_slow, "
					"over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, "
					"voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, "
					"grid_recovery_time) VALUES('%s', %d, %d, %d, %d, %f, %f, %f, %f, %f, %f, %f, %f, %d) ",
					inverter->inverterid, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow,
					under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow,
					voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time);
			sqlite3_exec_3times(db, sql);

			sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAA%03d%03d%03d%03d%06d%06d%06d%06dAAAAAAAAAAAAAAAAAEND",
					inverter->inverterid, under_voltage_slow, over_voltage_slow, (int)(under_frequency_slow*10),
					(int)(over_frequency_slow*10), grid_recovery_time, under_voltage_fast, over_voltage_fast,
					(int)(under_frequency_fast*10), (int)(over_frequency_fast*10), (int)(voltage_triptime_fast*100),
					(int)(voltage_triptime_slow*100), (int)(frequency_triptime_fast*100), (int)(frequency_triptime_slow*100));
			save_inverter_parameters_result(inverter, 131, inverter_result);
		}

		max_power = (readbuff[22] << 16) / 7395;
		sprintf(sql,"UPDATE power SET limitedresult=%d WHERE id=%s ", max_power, inverter->inverterid);
		sqlite3_exec_3times(db, sql);

		sqlite3_close(db);

		return 2;
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

		sprintf(sql, "REPLACE INTO protection_parameters "
				"(id, under_voltage_slow, over_voltage_slow, under_frequency_slow, over_frequency_slow, grid_recovery_time) "
				"VALUES('%s', %d, %d, %f, %f, %d) ",
				inverter->inverterid, under_voltage_slow, over_voltage_slow, under_frequency_slow, over_frequency_slow,
				grid_recovery_time);
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		sqlite3_close( db );

		sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEND",
				inverter->inverterid, under_voltage_slow, over_voltage_slow, (int)(under_frequency_slow*10),
				(int)(over_frequency_slow*10), grid_recovery_time);
		save_inverter_parameters_result(inverter, 131, inverter_result);

		return 1;
	}

	return 0;
}

/*解析DE帧参数，并保存到数据库中*/
int resolve_protection_paras_de(char *id, char *readbuff, int size)
{
	sqlite3 *db=NULL;
	char sql[1024];
	char inverter_result[65535]={'\0'};
	char *zErrMsg = 0;
	int i, max_power;

	float voltage_triptime_fast;
	float voltage_triptime_slow;
	float frequency_triptime_fast;
	float frequency_triptime_slow;
	int grid_recovery_time;
	int under_voltage_slow;
	float voltage_3_clearance_time;
	float regulated_dc_working_point;
	int start_time;
	float temp;

	if(37 == size){
//		if(readbuff[21]>100)
//		{
//			if(caltype)		//60Hz
//			{
//				voltage_triptime_fast = (readbuff[21]-100)*64*(1/60.0) + readbuff[21]*50*(1/60.0);
//			}
//			else			//50Hz
//			{
//				voltage_triptime_fast = (readbuff[21]-100)*64*(1/50.0) + readbuff[21]*50*(1/50.0);
//			}
//		}
//		else if(readbuff[21]>0)
//		{
//			if(caltype)
//			{
//				voltage_triptime_fast = readbuff[21]*(1/60.0)*0.5;
//			}
//			else
//			{
//				voltage_triptime_fast = readbuff[21]*(1/50.0)*0.5;
//			}
//		}
//		else
//			voltage_triptime_fast = 0.0;
//
//		if(readbuff[22]>=175)
//		{
//			if(caltype)
//				voltage_triptime_slow = 120*(1/60.0)*0.5 + 54*32*(1/60.0) + (readbuff[22]-174)*512*(1/60.0);
//			else
//				voltage_triptime_slow = 120*(1/50.0)*0.5 + 54*32*(1/50.0) + (readbuff[22]-174)*512*(1/50.0);
//		}
//		else if(readbuff[22]>120)
//		{
//			if(caltype)
//				voltage_triptime_slow = 60*(1/60.0) + (readbuff[22]-120)*64*(1/60.0);
//			else
//				voltage_triptime_slow = 60*(1/50.0) + (readbuff[22]-120)*64*(1/50.0);
//		}
//		else if(readbuff[22]>0)
//		{
//			if(caltype)
//				voltage_triptime_slow = readbuff[22]*(1/60.0)*0.5;
//			else
//				voltage_triptime_slow = readbuff[22]*(1/50.0)*0.5;
//		}
//		else
//			voltage_triptime_slow = 0.0;
//
//		if(readbuff[23]>100)
//		{
//			if(caltype)		//60Hz
//			{
//				frequency_triptime_fast = (readbuff[23]-100)*64*(1/60.0) + readbuff[23]*50*(1/60.0);
//			}
//			else			//50Hz
//			{
//				frequency_triptime_fast = (readbuff[23]-100)*64*(1/50.0) + readbuff[23]*50*(1/50.0);
//			}
//		}
//		else if(readbuff[23]>0)
//		{
//			if(caltype)
//			{
//				frequency_triptime_fast = readbuff[23]*(1/60.0)*0.5;
//			}
//			else
//			{
//				frequency_triptime_fast = readbuff[23]*(1/50.0)*0.5;
//			}
//		}
//		else
//			frequency_triptime_fast = 0.0;
//
//		if(readbuff[24]>=175)
//		{
//			if(caltype)
//				frequency_triptime_slow = 120*(1/60.0)*0.5 + 54*32*(1/60.0) + (readbuff[24]-174)*512*(1/60.0);
//			else
//				frequency_triptime_slow = 120*(1/50.0)*0.5 + 54*32*(1/50.0) + (readbuff[24]-174)*512*(1/50.0);
//		}
//		else if(readbuff[24]>120)
//		{
//			if(caltype)
//				frequency_triptime_slow = 60*(1/60.0) + (readbuff[24]-120)*64*(1/60.0);
//			else
//				frequency_triptime_slow = 60*(1/50.0) + (readbuff[24]-120)*64*(1/50.0);
//		}
//		else if(readbuff[24]>0)
//		{
//			if(caltype)
//				frequency_triptime_slow = readbuff[24]*(1/60.0)*0.5;
//			else
//				frequency_triptime_slow = readbuff[24]*(1/50.0)*0.5;
//		}
//		else
//			frequency_triptime_slow = 0.0;
//
//		if(readbuff[25]>32)
//		{
//			if(caltype)
//				grid_recovery_time = (readbuff[25]-32)*512*(1/60.0)*0.5 + 32*128*(1/60.0)*0.5;
//			else
//				grid_recovery_time = (readbuff[25]-32)*512*(1/50.0)*0.5 + 32*128*(1/50.0)*0.5;
//		}
//		else if(readbuff[25]>0)
//		{
//			if(caltype)
//				grid_recovery_time = readbuff[25]*128*(1/60.0)*0.5;
//			else
//				grid_recovery_time = readbuff[25]*128*(1/50.0)*0.5;
//		}
//		else
//			grid_recovery_time = 0;
//
		regulated_dc_working_point = (readbuff[26]*256+readbuff[27])*82.5/4096;

		if(caltype)
			under_voltage_slow = readbuff[28]*4/1.452;
		else
			under_voltage_slow = readbuff[28]*4/1.48975;

//		if(readbuff[29]>=175)
//		{
//			if(caltype)
//				voltage_3_clearance_time = 120*(1/60.0)*0.5 + 54*32*(1/60.0) + (readbuff[29]-174)*512*(1/60.0);
//			else
//				voltage_3_clearance_time = 120*(1/50.0)*0.5 + 54*32*(1/50.0) + (readbuff[29]-174)*512*(1/50.0);
//		}
//		else if(readbuff[29]>120)
//		{
//			if(caltype)
//				voltage_3_clearance_time = 60*(1/60.0) + (readbuff[29]-120)*64*(1/60.0);
//			else
//				voltage_3_clearance_time = 60*(1/50.0) + (readbuff[29]-120)*64*(1/50.0);
//		}
//		else if(readbuff[29]>0)
//		{
//			if(caltype)
//				voltage_3_clearance_time = readbuff[29]*(1/60.0)*0.5;
//			else
//				voltage_3_clearance_time = readbuff[29]*(1/50.0)*0.5;
//		}
//		else
//			voltage_3_clearance_time = 0.0;

//		if(readbuff[30]>32)
//		{
//			if(caltype)
//				start_time = (readbuff[30]-32)*512*(1/60.0)*0.5 + 32*128*(1/60.0)*0.5;
//			else
//				start_time = (readbuff[30]-32)*512*(1/50.0)*0.5 + 32*128*(1/50.0)*0.5;
//		}
//		else if(readbuff[30]>0)
//		{
//			if(caltype)
//				start_time = readbuff[30]*128*(1/60.0)*0.5;
//			else
//				start_time = readbuff[30]*128*(1/50.0)*0.5;
//		}
//		else
//			start_time = 0;
		voltage_triptime_fast = change_1_clearance_time_from_char(readbuff[21]);
		voltage_triptime_slow = change_2_clearance_time_from_char(readbuff[22]);
		frequency_triptime_fast = change_1_clearance_time_from_char(readbuff[23]);
		frequency_triptime_slow = change_2_clearance_time_from_char(readbuff[24]);
		grid_recovery_time = change_start_time_from_char(readbuff[25]);
		voltage_3_clearance_time = change_2_clearance_time_from_char(readbuff[29]);
		start_time = change_start_time_from_char(readbuff[30]);

		if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
			return -1;

//		sprintf(sql, "UPDATE protection_parameters SET "
//				"voltage_triptime_fast=%f, voltage_triptime_slow=%f, frequency_triptime_fast=%f, frequency_triptime_slow=%f, "
//				"grid_recovery_time=%d, under_voltage_slow=%d, voltage_3_clearance_time=%f, regulated_dc_working_point=%d, "
//				"start_time=%d WHERE id='%s'", voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast,
//				frequency_triptime_slow, grid_recovery_time, under_voltage_slow, voltage_3_clearance_time, start_time,
//				start_time, id);
//		printdecmsg("under_voltage_slow", under_voltage_slow);
//		printfloatmsg("voltage_3_clearance_time", voltage_3_clearance_time);
//		printdecmsg("regulated_dc_working_point", regulated_dc_working_point);
//		printdecmsg("start_time", start_time);
//		sprintf(sql, "UPDATE protection_parameters SET "
//				"voltage_triptime_fast=%f,voltage_triptime_slow=%f,frequency_triptime_fast=%f,frequency_triptime_slow=%f,"
//				"grid_recovery_time=%d,under_voltage_slow=%d,voltage_3_clearance_time=%f,regulated_dc_working_point=%f, "
//				"start_time=%d WHERE id='%s'",
//				voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow,
//				grid_recovery_time, under_voltage_slow, voltage_3_clearance_time, regulated_dc_working_point,
//				start_time, id);

		sprintf(sql, "REPLACE INTO protection_parameters (id, under_voltage_fast, over_voltage_fast, "
				"under_voltage_stage_2, over_voltage_slow, under_frequency_fast, over_frequency_fast, "
				"under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, "
				"frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, under_voltage_slow, "
				"voltage_3_clearance_time, regulated_dc_working_point, start_time) "
				"VALUES('%s', %d, %d, %d, %d, %f, %f, %f, %f, %f, %f, %f, %f, %d, %d, %f, %f, %d) ",
				id, paras.under_voltage_fast, paras.over_voltage_fast, paras.under_voltage_stage_2, paras.over_voltage_slow,
				paras.under_frequency_fast, paras.over_frequency_fast, paras.under_frequency_slow, paras.over_frequency_slow,
				voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow,
				grid_recovery_time, under_voltage_slow, voltage_3_clearance_time, regulated_dc_working_point,
				start_time);
		sqlite3_exec_3times(db, sql);

		printmsg(sql);
		sqlite3_exec_3times(db, sql);

		sqlite3_close( db );

		sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAA%03d%03d%03d%03d%06d%06d%06d%06d%03d%03d%06d%05dEND",
				id, under_voltage_slow, paras.over_voltage_slow, (int)(paras.under_frequency_slow*10), (int)(paras.over_frequency_slow*10),
				grid_recovery_time, paras.under_voltage_fast, paras.over_voltage_fast, (int)(paras.under_frequency_fast*10),
				(int)(paras.over_frequency_fast*10), (int)(voltage_triptime_fast*100), (int)(voltage_triptime_slow*100),
				(int)(frequency_triptime_fast*100),(int)(frequency_triptime_slow*100), (int)(regulated_dc_working_point*10),
				paras.under_voltage_stage_2, (int)(voltage_3_clearance_time*100), start_time);
		save_inverter_parameters_result_id(id, 131, inverter_result);

		return 0;
	}

	return -1;
}

/*解析参数，只用于判断属于13项还是17项逆变器，不保存结果*/
int resolve_type(char *readbuff, int size)
{
	float voltage_triptime_fast;
	float voltage_triptime_slow;
	float frequency_triptime_fast;
	float frequency_triptime_slow;
	float temp;

	if(42 == size){		//13、17项参数
		if((2 == caltype)||(1 == caltype))
			voltage_triptime_fast = readbuff[32]*0.0083;
		else
			voltage_triptime_fast = readbuff[32]*0.01;

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

		if((0 == (int)(voltage_triptime_fast)) && (0 == (int)(voltage_triptime_slow)) && (0 == (int)(frequency_triptime_fast)) &&
				(0 == (int)(frequency_triptime_slow)))
		{
			return 3;
		}
		else
		{
			return 2;
		}
	}

	return 0;
}

/*向逆变器读取设置的值，来判断13项还是17项，只有17项参数的逆变器才能单点设置under voltage stage 2的值*/
int get_inverter_type(char *id)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;
	fd_set rd;
	struct timeval timeout;
	int type;

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

	res = get_reply_from_serial(plcmodem, 2, 0, readbuff);

	if((42 == res) &&		//13项参数
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
			(readbuff[11] == sendbuff[11]) &&
			(readbuff[12] == sendbuff[12]) &&
			(readbuff[13] == sendbuff[13]) &&
			(readbuff[14] == sendbuff[14]) &&
			(readbuff[15] == sendbuff[15]) &&
			(readbuff[16] == sendbuff[16]) &&
			(0x4F == readbuff[17]) &&
			(0x00 == readbuff[18]) &&
			(0x00 == readbuff[19]) &&
			(0xDA == readbuff[20]) &&
			(0xFE == readbuff[40]) &&
			(0xFE == readbuff[41])){
		type = resolve_type(readbuff, res);
		return type;
	}

	return 0;
}

/*向逆变器读取设置的值*/
int get_parameters_from_inverter(struct inverter_info_t *inverter)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;
	fd_set rd;
	struct timeval timeout;
	int type;

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

	if((42 == res) &&		//13项参数
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
		type = resolve_protection_paras(inverter, readbuff, res);
		return type;
	}
	else if((37 == res) &&		//5项参数
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
		return 1;
	}
	else
		return 0;
}

/*向逆变器读取设置的值*/
int get_parameters_from_inverter_de(char *id)
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
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xDE;
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

	res = get_reply_from_serial(plcmodem, 2, 0, readbuff);

	if((37 == res) &&
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
			(readbuff[11] == sendbuff[11]) &&
			(readbuff[12] == sendbuff[12]) &&
			(readbuff[13] == sendbuff[13]) &&
			(readbuff[14] == sendbuff[14]) &&
			(readbuff[15] == sendbuff[15]) &&
			(readbuff[16] == sendbuff[16]) &&
			(0x4F == readbuff[17])&&
			(0x00 == readbuff[18])&&
			(0x00 == readbuff[19])&&
			(0xDE == readbuff[20])&&
			(0xFE == readbuff[35])&&
			(0xFE == readbuff[36])){
		resolve_protection_paras_de(id, readbuff, res);
		return 0;
	}
	else
		return -1;
}

/*读取每一台逆变器的保护参数*/
int read_protection_parameters(struct inverter_info_t *firstinverter)
{
	int i, j, k;
	int result;
	struct inverter_info_t *inverter = firstinverter;

	memset(set_result, '\0', sizeof(set_result));

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		for(j=0; j<3; j++){
			result = get_parameters_from_inverter(inverter);
			if(result > 0) {
				if(3 == result) {
					for(k=0; k<3; k++)
						if(!get_parameters_from_inverter_de(inverter->inverterid))
							break;
					break;
				}

				break;
			}
		}
	}

	return 0;
}

/*设置逆变器的保护参数*/
int set_protection_parameters(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	char buff[16];
	int flag;
	
	if(set_protection_paras(firstinverter) > 0)
		read_protection_parameters(firstinverter);

	fp = fopen("/tmp/presetdata.conf", "r");
	if(fp)
	{
		fgets(buff, 255, fp);
		fclose(fp);
		
		if(!strlen(buff))
			flag = 0;
		if('0' == buff[0])
			flag = 0;
		if('1' == buff[0])
			flag = 1;
		if('2' == buff[0])
			flag = 2;

//		if(1 == flag){
//printmsg("debug1");
//			set_protection_paras();
//			read_protection_parameters(firstinverter);

//			fp = fopen("/tmp/presetdata.conf", "w");
//			fprintf(fp, "0");
//			fclose(fp);
//		}
		if(2 == flag)
		{
			read_protection_parameters(firstinverter);

			fp = fopen("/tmp/presetdata.conf", "w");
			fprintf(fp, "0");
			fclose(fp);
		}
	}
	
//printmsg("debug7");

	return 0;
}
