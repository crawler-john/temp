/*
 * 本文件的主要功能是设置逆变器的保护参数。同时兼容新的13项保护参数和老的5项参数。
 * Created by Zhyf
 * Created on 2014/04/24
 */

#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "variation.h"
#include <string.h>

extern int zbmodem;				//zigbee串口
extern unsigned char ccuid[7];		//ECU3501的ID
extern int caltype;		//计算方式，NA版和非NA版的区别
char set_result[65535];
extern int zb_get_reply(char *data,inverter_info *inverter);			//读取逆变器的返回帧
extern int zb_broadcast_cmd(char *buff, int length);		//zigbee广播包头
extern int zb_send_cmd(inverter_info *inverter, char *buff, int length);		//zigbee包头

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

int get_under_voltage_slow(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int under_voltage_slow=0xFFFF, res, i;
	inverter_info *inverter = firstinverter;
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

int get_over_voltage_slow(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int over_voltage_slow=0xFFFF, res, i;
	inverter_info *inverter = firstinverter;
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

char get_under_frequency_slow(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char under_frequency_slow=0xFF, res, i;
	inverter_info *inverter = firstinverter;

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

char get_over_frequency_slow(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char over_frequency_slow=0xFF, res, i;
	inverter_info *inverter = firstinverter;

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

int get_recovery_time_from_inverter(inverter_info *inverter)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;
	int grid_recovery_time=-1;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xDD;
	sendbuff[6] = 0x11;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;
	for(i=2; i<9; i++)
	{
		check = check + sendbuff[i];
	}
	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	zb_send_cmd(inverter, sendbuff, 13);

	res = zb_get_reply(readbuff,inverter);


	if((29 == res) &&
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0x16 == readbuff[2]) &&
			(0x00 == readbuff[3]) &&
			(0x00 == readbuff[4]) &&
			(0xDA == readbuff[5]) &&
			(0x4F == readbuff[17]) &&
			(0x00 == readbuff[18]) &&
			(0x00 == readbuff[19]) &&
			(0xDA == readbuff[20]) &&
			(0xFE == readbuff[27]) &&
			(0xFE == readbuff[28])){
		if(9 == readbuff[21])
			grid_recovery_time = 300;
		else
			grid_recovery_time = readbuff[21]*32;
	}

	return grid_recovery_time;
}

int get_grid_recovery_time(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int recovery_time=300, recovery_time_result=-1, res, i, flag=0;
	inverter_info *inverter = firstinverter;

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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE1;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage fast", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE2;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage fast", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

/*设置逆变器的慢速欠压保护值，有新老两种逆变器*/
int set_undervoltage_slow(char *value)
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE3;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage slow", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

/*设置逆变器的慢速过压保护值，有新老两种逆变器*/
int set_overvoltage_slow(char *value)
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE4;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage slow", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE7;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency fast", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE8;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency fast", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

/*设置逆变器的慢速欠频保护值，有新老两种逆变器*/
int set_underfrequency_slow(char *value)
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE9;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency slow", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

/*设置逆变器的慢速过频保护值，有新老两种逆变器*/
int set_overfrequency_slow(char *value)
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xEA;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency slow", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

int set_voltage_triptime_fast(char *value)
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE5;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime fast", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

int set_voltage_triptime_slow(char *value)
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE6;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime slow", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

int set_frequency_triptime_fast(char *value)
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xEB;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL


	printhexmsg("Set frequency triptime fast", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

int set_frequency_triptime_slow(char *value)
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
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xEC;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime slow", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
	sleep(10);

	return 0;
}

/*设置逆变器的并网恢复时间，有新老两种逆变器*/
int set_grid_recovery_time(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x07;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xED;		//ccuid
	sendbuff[6] = atoi(value)/32;
	sendbuff[7] = 0x00;		//ccuid
	sendbuff[8] = 0x05;		//ccuid
	sendbuff[9] = 0x01;		//ccuid

	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}

	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;		//TAIL
	sendbuff[13] = 0xFE;		//TAIL


	printhexmsg("Set grid recovery time", sendbuff, 14);

	zb_broadcast_cmd(sendbuff, 14);
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
int set_protection_paras(inverter_info *firstinverter)
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
			set_undervoltage_slow(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("over_voltage_slow", para_name))
		{
			set_overvoltage_slow(value);
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
			set_underfrequency_slow(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("over_frequency_slow", para_name))
		{
			set_overfrequency_slow(value);
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
			set_grid_recovery_time(value);
			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else
			break;
	}

	return count;
}

/*解析参数，并保存到数据库中*/
int resolve_protection_paras(inverter_info *inverter, char *readbuff, int size)
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


	if(29 == size){		//13项参数
		if((2 == caltype)||(1 == caltype))
			temp = readbuff[24-15]*4/1.452;
		else
			temp = readbuff[24-15]*4/1.48975;

		if((temp-(int)temp)>=0.5)
			under_voltage_fast = (int)temp +1;
		else
			under_voltage_fast = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[25-15]*4/1.452;
		else
			temp = readbuff[25-15]*4/1.48975;

		if((temp-(int)temp)>=0.5)
			over_voltage_fast = (int)temp +1;
		else
			over_voltage_fast = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[26-15]*4/1.452;
		else
			temp = readbuff[26-15]*4/1.48975;

		if((temp-(int)temp)>0.5)
			under_voltage_slow = (int)temp +1;
		else
			under_voltage_slow = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[27-15]*4/1.452;
		else
			temp = readbuff[27-15]*4/1.48975;

		if((temp-(int)temp)>0.5)
			over_voltage_slow = (int)temp +1;
		else
			over_voltage_slow = (int)temp;

		if((2 == caltype)||(1 == caltype))
			under_frequency_fast = (600 - readbuff[28-15])/10.0;
		else
			under_frequency_fast = (500 - readbuff[28-15])/10.0;

		if((2 == caltype)||(1 == caltype))
			over_frequency_fast = (600 + readbuff[29-15])/10.0;
		else
			over_frequency_fast = (500 + readbuff[29-15])/10.0;

		if((2 == caltype)||(1 == caltype))
			under_frequency_slow = (600 - readbuff[30-15])/10.0;
		else
			under_frequency_slow = (500 - readbuff[30-15])/10.0;

		if((2 == caltype)||(1 == caltype))
			over_frequency_slow = (600 + readbuff[31-15])/10.0;
		else
			over_frequency_slow = (500 + readbuff[31-15])/10.0;

		if((2 == caltype)||(1 == caltype))
			voltage_triptime_fast = readbuff[32-15]*0.0083;
		else
			voltage_triptime_fast = readbuff[32-15]*0.01;

		if((2 == caltype)||(1 == caltype))
			voltage_triptime_slow = readbuff[33-15]*1.07;
		else
			voltage_triptime_slow = readbuff[33-15]*1.28;

		if((2 == caltype)||(1 == caltype))
			frequency_triptime_fast = readbuff[34-15]*0.033;
		else
			frequency_triptime_fast = readbuff[34-15]*0.04;

		if((2 == caltype)||(1 == caltype))
			frequency_triptime_slow = readbuff[35-15]*1.07;
		else
			frequency_triptime_slow = readbuff[35-15]*1.28;

		if(9 == readbuff[36-15])
			grid_recovery_time = 300;
		else
			grid_recovery_time = readbuff[36-15]*32;

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

		max_power = (readbuff[22-15] << 16) / 7395;
		sprintf(sql,"UPDATE power SET limitedresult=%d WHERE id=%s ", max_power, inverter->inverterid);
		sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

		sqlite3_close( db );

		sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAA%03d%03d%03d%03d%02d%04d%02d%04dEND", inverter->inverterid, under_voltage_slow, over_voltage_slow, (int)(under_frequency_slow*10), (int)(over_frequency_slow*10), grid_recovery_time, under_voltage_fast, over_voltage_fast, (int)(under_frequency_fast*10), (int)(over_frequency_fast*10), (int)(voltage_triptime_fast*100), (int)(voltage_triptime_slow*100), (int)(frequency_triptime_fast*100), (int)(frequency_triptime_slow*100));
		save_inverter_parameters_result(inverter, 121, inverter_result);

		return 0;
	}
}

/*向逆变器读取设置的值*/
int get_parameters_from_inverter(inverter_info *inverter)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xDD;
	sendbuff[6] = 0x11;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;
	for(i=2; i<9; i++)
	{
		check = check + sendbuff[i];
	}
	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	zb_send_cmd(inverter, sendbuff, 13);

	res = zb_get_reply(readbuff,inverter);


	if((29 == res) &&
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0x16 == readbuff[2]) &&
			(0x00 == readbuff[3]) &&
			(0x00 == readbuff[4]) &&
			(0xDA == readbuff[5]) &&
			(0xFE == readbuff[27]) &&
			(0xFE == readbuff[28])){
		resolve_protection_paras(inverter, readbuff, res);
		return 0;
	}
	else
		return -1;

}

/*读取每一台逆变器的保护参数*/
int read_protection_parameters(inverter_info *firstinverter)
{
	int i, j;
	inverter_info *inverter = firstinverter;

	memset(set_result, '\0', sizeof(set_result));

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		for(j=0; j<3; j++){
			if(!get_parameters_from_inverter(inverter))
				break;
		}
	}

	return 0;
}

/*设置逆变器的保护参数*/
int set_protection_parameters(inverter_info *firstinverter)
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
#if 0
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
	if(SQLITE_OK != res)		//create a database
	{
		return -1;
	}

	strcpy(sql, "SELECT parameter_name,parameter_value FROM set_protection_parameters WHERE set_flag=1 LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow)
	{
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

int get_under_voltage_slow_yc500(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int under_voltage_slow=0xFFFF, res, i;
	inverter_info *inverter = firstinverter;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)	//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='under_voltage_slow'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					temp = atof(azResult[1]) * 26204.64;
					if((temp-(int)temp)>=0.5)
						under_voltage_slow = temp + 1;
					else
						under_voltage_slow = temp;
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}

	return under_voltage_slow;
}

int get_over_voltage_slow_yc500(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int over_voltage_slow=0xFFFF, res, i;
	inverter_info *inverter = firstinverter;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)		//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='over_voltage_slow'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					temp = atof(azResult[1]) * 26204.64;
					if((temp-(int)temp)>=0.5)
						over_voltage_slow = temp + 1;
					else
						over_voltage_slow = temp;
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}

	return over_voltage_slow;
}

char get_under_frequency_slow_yc500(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char under_frequency_slow=0xFF, res, i;
	inverter_info *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)		//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='under_frequency_slow'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					under_frequency_slow = (2237500/atoi(azResult[1]));
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}

	return under_frequency_slow;
}

char get_over_frequency_slow_yc500(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char over_frequency_slow=0xFF, res, i;
	inverter_info *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)		//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='over_frequency_slow'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					over_frequency_slow = (2237500/atoi(azResult[1]));
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}

	return over_frequency_slow;
}

int get_under_voltage_slow_yc1000(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int under_voltage_slow=0xFFFF, res, i;
	inverter_info *inverter = firstinverter;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)	//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='under_voltage_slow'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					temp = atof(azResult[1]) * 11614.45;
					if((temp-(int)temp)>=0.5)
						under_voltage_slow = temp + 1;
					else
						under_voltage_slow = temp;
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}

	return under_voltage_slow;
}

int get_over_voltage_slow_yc1000(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int over_voltage_slow=0xFFFF, res, i;
	inverter_info *inverter = firstinverter;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)		//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='over_voltage_slow'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					temp = atof(azResult[1]) * 11614.45;
					if((temp-(int)temp)>=0.5)
						over_voltage_slow = temp + 1;
					else
						over_voltage_slow = temp;
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}

	return over_voltage_slow;
}

char get_under_frequency_slow_yc1000(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char under_frequency_slow=0xFF, res, i;
	inverter_info *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)		//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='under_frequency_slow'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					under_frequency_slow = (2560000/atoi(azResult[1]));
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}

	return under_frequency_slow;
}

char get_over_frequency_slow_yc1000(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char over_frequency_slow=0xFF, res, i;
	inverter_info *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)		//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='over_frequency_slow'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					over_frequency_slow = (2560000/atoi(azResult[1]));
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}

	return over_frequency_slow;
}


/*
int zb_query_protect_parameter(inverter_info *inverter, char *protect_data_DD_reply)		//存储参数查询DD指令
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(inverter->inverterid, "Query protect parameter");
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(protect_data_DD_reply,inverter);
	if((33 == ret) && (0xDD == protect_data_DD_reply[3]) && (0xFB == protect_data_DD_reply[0]) && (0xFB == protect_data_DD_reply[1]) && (0xFE == protect_data_DD_reply[31]) && (0xFE == protect_data_DD_reply[32]))
		return 1;
	else
		return -1;
}
*/

int get_recovery_time_from_inverter(inverter_info *inverter)
{
	int i=0, ret;
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	int grid_recovery_time=-1;

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(inverter->inverterid, "Query protect parameter");
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(readbuff,inverter);
	if((33 == ret) && (0xDD == readbuff[3]) && (0xFB == readbuff[0]) && (0xFB == readbuff[1]) && (0xFE == readbuff[31]) && (0xFE == readbuff[32]))
	{
		grid_recovery_time = readbuff[17]*256 + readbuff[18];
	}

	return grid_recovery_time;

}

int get_grid_recovery_time(inverter_info *firstinverter)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int recovery_time=300, recovery_time_result=-1, res, i, flag=0;
	inverter_info *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)		//create a database
	{
		strcpy(sql, "SELECT parameter_value FROM set_protection_parameters WHERE parameter_name='grid_recovery_time'");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					flag = 1;
					recovery_time = atoi(azResult[1]);
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get grid recovery time", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}

		if(!flag)
		{
			strcpy(sql, "SELECT grid_recovery_time FROM protection_parameters WHERE grid_recovery_time IS NOT NULL LIMIT 0,1");
			for(i=0; i<3; i++)
			{
				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
				{
					if(1 == nrow)
					{
						flag = 1;
						recovery_time = atoi(azResult[1]);
					}
					sqlite3_free_table( azResult );
					break;
				}
				else
				{
					print2msg("Get grid recovery time", zErrMsg);
					sqlite3_free_table( azResult );
				}
			}
		}
		sqlite3_close(db);
	}

	if(!flag)
	{
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
		{
			recovery_time_result = get_recovery_time_from_inverter(inverter);
			if(-1 != recovery_time_result)
				return recovery_time_result;
		}
	}

	return recovery_time;
}

#if 0

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

	for(i=2; i<23; i++)
	{
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

	for(i=2; i<23; i++)
	{
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
#endif

/*设置逆变器的慢速欠压保护值，有新老两种逆变器*/
int set_undervoltage_slow_yc500(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;
	float temp;
	int voltage;

	/*
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
	sendbuff[20] = 0xE3;			//CMD
	sendbuff[21] = data;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage slow", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);
	 */
///////////////////////////////以下设置老参数//////////////////////////////////
	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow_yc500(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc500(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc500(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc500(firstinverter);


	temp = atof(value) * 26204.64;
	if((temp-(int)temp)>=0.5)
		voltage = temp + 1;
	else
		voltage = temp;


	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = voltage/65535;
	sendbuff[i++] = voltage%65535/256;
	sendbuff[i++] = voltage%256;
	sendbuff[i++] = over_voltage_slow/65535;
	sendbuff[i++] = over_voltage_slow%65535/256;
	sendbuff[i++] = over_voltage_slow%256;
	sendbuff[i++] = under_frequency_slow/256;
	sendbuff[i++] =	under_frequency_slow%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] = over_frequency_slow%256;
	sendbuff[i++] = grid_recovery_time/256;
	sendbuff[i++] = grid_recovery_time%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set yc500 undervoltage slow (old)");
	printhexmsg("Set yc500 undervoltage slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);

	sleep(10);

	return 0;
}

int set_undervoltage_slow_yc1000(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;
	float temp;
	int voltage;

///////////////////////////////以下设置老参数//////////////////////////////////
	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow_yc1000(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc1000(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc1000(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc1000(firstinverter);


	temp = atof(value) * 11614.45;
	if((temp-(int)temp)>=0.5)
		voltage = temp + 1;
	else
		voltage = temp;


	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = voltage/65535;
	sendbuff[i++] = voltage%65535/256;
	sendbuff[i++] = voltage%256;
	sendbuff[i++] = over_voltage_slow/65535;
	sendbuff[i++] = over_voltage_slow%65535/256;
	sendbuff[i++] = over_voltage_slow%256;
	sendbuff[i++] = under_frequency_slow/256;
	sendbuff[i++] =	under_frequency_slow%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] = over_frequency_slow%256;
	sendbuff[i++] = grid_recovery_time/256;
	sendbuff[i++] = grid_recovery_time%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set yc1000 undervoltage slow (old)");
	printhexmsg("Set yc1000 undervoltage slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);

	sleep(10);

	return 0;
}

/*设置逆变器的慢速过压保护值，有新老两种逆变器*/
int set_overvoltage_slow_yc500(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;
	float temp;
	int voltage;


	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow_yc500(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc500(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc500(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc500(firstinverter);


	temp = atof(value) * 11614.45;
	if((temp-(int)temp)>=0.5)
		voltage = temp + 1;
	else
		voltage = temp;

	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = under_voltage_slow/65535;
	sendbuff[i++] = under_voltage_slow%65535/256;
	sendbuff[i++] = under_voltage_slow%256;
	sendbuff[i++] = voltage/65535;
	sendbuff[i++] = voltage%65535/256;
	sendbuff[i++] = voltage%256;
	sendbuff[i++] = under_frequency_slow/256;
	sendbuff[i++] =	under_frequency_slow%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] = over_frequency_slow%256;
	sendbuff[i++] = grid_recovery_time/256;
	sendbuff[i++] = grid_recovery_time%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set yc500 undervoltage slow (old)");
	printhexmsg("Set yc500 undervoltage slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);
	sleep(10);

	return 0;
}

int set_overvoltage_slow_yc1000(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;
	float temp;
	int voltage;


	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow_yc1000(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc1000(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc1000(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc1000(firstinverter);


	temp = atof(value) * 11614.45;
	if((temp-(int)temp)>=0.5)
		voltage = temp + 1;
	else
		voltage = temp;

	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = under_voltage_slow/65535;
	sendbuff[i++] = under_voltage_slow%65535/256;
	sendbuff[i++] = under_voltage_slow%256;
	sendbuff[i++] = voltage/65535;
	sendbuff[i++] = voltage%65535/256;
	sendbuff[i++] = voltage%256;
	sendbuff[i++] = under_frequency_slow/256;
	sendbuff[i++] =	under_frequency_slow%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] = over_frequency_slow%256;
	sendbuff[i++] = grid_recovery_time/256;
	sendbuff[i++] = grid_recovery_time%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set yc1000 undervoltage slow (old)");
	printhexmsg("Set yc1000 undervoltage slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);
	sleep(10);

	return 0;
}


/*设置逆变器的快速欠频保护值*/
#if 0
int set_underfrequency_fast(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
	{
		if((600 - (int)(atof(value)*10))<0)
			data = 0xFF;
		else
			data = 600 - (int)(atof(value)*10);
	}
	else
	{
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

	for(i=2; i<23; i++)
	{
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

	if((2 == caltype)||(1 == caltype))
	{
		if(((int)(atof(value)*10)-600)<0)
			data = 0xFF;
		else
			data = (int)(atof(value)*10)-600;
	}
	else
	{
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

	for(i=2; i<23; i++)
	{
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
#endif

/*设置逆变器的慢速欠频保护值，有新老两种逆变器*/
int set_underfrequency_slow_yc500(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;

	/*
	if((2 == caltype)||(1 == caltype))
	{
		if((600 - (int)(atof(value)*10))<0)
			data = 0xFF;
		else
			data = 600 - (int)(atof(value)*10);
	}
	else
	{
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

	for(i=2; i<23; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency slow", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);
	 */
//////////////////////////////////////////////////////////////

	data = (2237500/atoi(value));

	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow_yc500(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc500(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc500(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc500(firstinverter);


	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = under_voltage_slow/65535;
	sendbuff[i++] = under_voltage_slow%65535/256;
	sendbuff[i++] = under_voltage_slow%256;
	sendbuff[i++] = over_voltage_slow/65535;
	sendbuff[i++] = over_voltage_slow%65535/256;
	sendbuff[i++] = over_voltage_slow%256;
	sendbuff[i++] = data/256;
	sendbuff[i++] =	data%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] = over_frequency_slow%256;
	sendbuff[i++] = grid_recovery_time/256;
	sendbuff[i++] = grid_recovery_time%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set yc500 underfrequency slow (old)");
	printhexmsg("Set yc500 underfrequency slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);
	sleep(10);

	return 0;
}

int set_underfrequency_slow_yc1000(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;


	data = (2560000/atoi(value));

	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow_yc1000(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc1000(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc1000(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc1000(firstinverter);


	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = under_voltage_slow/65535;
	sendbuff[i++] = under_voltage_slow%65535/256;
	sendbuff[i++] = under_voltage_slow%256;
	sendbuff[i++] = over_voltage_slow/65535;
	sendbuff[i++] = over_voltage_slow%65535/256;
	sendbuff[i++] = over_voltage_slow%256;
	sendbuff[i++] = data/256;
	sendbuff[i++] =	data%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] = over_frequency_slow%256;
	sendbuff[i++] = grid_recovery_time/256;
	sendbuff[i++] = grid_recovery_time%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set yc1000 underfrequency slow (old)");
	printhexmsg("Set yc1000 underfrequency slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);
	sleep(10);

	return 0;
}


/*设置逆变器的慢速过频保护值，有新老两种逆变器*/
int set_overfrequency_slow_yc500(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;

	/*
	if((2 == caltype)||(1 == caltype))
	{
		if(((int)(atof(value)*10)-600)<0)
			data = 0xFF;
		else
			data = (int)(atof(value)*10)-600;
	}
	else
	{
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

	for(i=2; i<23; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency slow", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);
*/
////////////////////////////////////////////////////////////

	data = (2237500/atoi(value));

	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow_yc500(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc500(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc500(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc500(firstinverter);


	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = under_voltage_slow/65535;
	sendbuff[i++] = under_voltage_slow%65535/256;
	sendbuff[i++] = under_voltage_slow%256;
	sendbuff[i++] = over_voltage_slow/65535;
	sendbuff[i++] = over_voltage_slow%65535/256;
	sendbuff[i++] = over_voltage_slow%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] =	over_frequency_slow%256;
	sendbuff[i++] = data/256;
	sendbuff[i++] = data%256;
	sendbuff[i++] = grid_recovery_time/256;
	sendbuff[i++] = grid_recovery_time%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set yc500 overfrequency slow (old)");
	printhexmsg("Set yc500 overfrequency slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);
	sleep(10);

	return 0;
}

int set_overfrequency_slow_yc1000(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, grid_recovery_time, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
	char data;


	data = (2560000/atoi(value));

	grid_recovery_time = get_grid_recovery_time(firstinverter);
	under_voltage_slow = get_under_voltage_slow_yc1000(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc1000(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc1000(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc1000(firstinverter);


	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = under_voltage_slow/65535;
	sendbuff[i++] = under_voltage_slow%65535/256;
	sendbuff[i++] = under_voltage_slow%256;
	sendbuff[i++] = over_voltage_slow/65535;
	sendbuff[i++] = over_voltage_slow%65535/256;
	sendbuff[i++] = over_voltage_slow%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] =	over_frequency_slow%256;
	sendbuff[i++] = data/256;
	sendbuff[i++] = data%256;
	sendbuff[i++] = grid_recovery_time/256;
	sendbuff[i++] = grid_recovery_time%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set yc1000 overfrequency slow (old)");
	printhexmsg("Set yc1000 overfrequency slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);
	sleep(10);

	return 0;
}

#if 0
int set_voltage_triptime_fast(char *value)
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

	for(i=2; i<23; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime fast", sendbuff, 27);

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

	for(i=2; i<23; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime slow", sendbuff, 27);

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

	for(i=2; i<23; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime fast", sendbuff, 27);

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

	for(i=2; i<23; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime slow", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);

	return 0;
}
#endif

/*设置逆变器的并网恢复时间，有新老两种逆变器*/
int set_grid_recovery_time_yc500(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;
/*
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

	for(i=2; i<23; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set grid recovery time", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);
*/
///////////////////////////////////////////////////////
	under_voltage_slow = get_under_voltage_slow_yc500(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc500(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc500(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc500(firstinverter);


	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = under_voltage_slow/65535;
	sendbuff[i++] = under_voltage_slow%65535/256;
	sendbuff[i++] = under_voltage_slow%256;
	sendbuff[i++] = over_voltage_slow/65535;
	sendbuff[i++] = over_voltage_slow%65535/256;
	sendbuff[i++] = over_voltage_slow%256;
	sendbuff[i++] = under_frequency_slow/256;
	sendbuff[i++] =	under_frequency_slow%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] = over_frequency_slow%256;
	sendbuff[i++] = atoi(value)/256;
	sendbuff[i++] = atoi(value)%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set grid recovery slow (old)");
	printhexmsg("Set grid recovery slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);
	sleep(10);


	return 0;
}

int set_grid_recovery_time_yc1000(inverter_info *firstinverter, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, under_voltage_slow, over_voltage_slow;
	char under_frequency_slow, over_frequency_slow;


	under_voltage_slow = get_under_voltage_slow_yc1000(firstinverter);
	over_voltage_slow = get_over_voltage_slow_yc1000(firstinverter);
	under_frequency_slow = get_under_frequency_slow_yc1000(firstinverter);
	over_frequency_slow = get_over_frequency_slow_yc1000(firstinverter);


	clear_zbmodem();			//发送指令前，先清空缓冲区
	i=0;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = under_voltage_slow/65535;
	sendbuff[i++] = under_voltage_slow%65535/256;
	sendbuff[i++] = under_voltage_slow%256;
	sendbuff[i++] = over_voltage_slow/65535;
	sendbuff[i++] = over_voltage_slow%65535/256;
	sendbuff[i++] = over_voltage_slow%256;
	sendbuff[i++] = under_frequency_slow/256;
	sendbuff[i++] =	under_frequency_slow%256;
	sendbuff[i++] = over_frequency_slow/256;
	sendbuff[i++] = over_frequency_slow%256;
	sendbuff[i++] = atoi(value)/256;
	sendbuff[i++] = atoi(value)%256;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(firstinverter->inverterid,"Set grid recovery slow (old)");
	printhexmsg("Set grid recovery slow (old)", sendbuff, i);
	zb_send_cmd(firstinverter, sendbuff, i);
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

	for(i=0; i<3; i++)
	{
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
int set_protection_paras(inverter_info *firstinverter)
{
	char para_name[64];
	char value[16];
	int count = 0;

	while(1 == get_value_flag(para_name, value))
	{
		count++;
	//	if(!strcmp("under_voltage_fast", para_name))
	//	{
	//		set_undervoltage_fast(value);
	//		clear_flag(para_name);					//设置后清除数据库中参数的设置标志
	//	}
	//	else if(!strcmp("over_voltage_fast", para_name))
	//	{
	//		set_overvoltage_fast(value);
	//		clear_flag(para_name);					//设置后清除数据库中参数的设置标志
	//	}
		if(!strcmp("under_voltage_slow", para_name))
		{
			if((5==firstinverter->model)||(6==firstinverter->model))
				set_undervoltage_slow_yc1000(firstinverter, value);
			else
				set_undervoltage_slow_yc500(firstinverter, value);

			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("over_voltage_slow", para_name))
		{
			if((5==firstinverter->model)||(6==firstinverter->model))
				set_overvoltage_slow_yc1000(firstinverter, value);
			else
				set_overvoltage_slow_yc500(firstinverter, value);

			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
	//	else if(!strcmp("under_frequency_fast", para_name))
	//	{
	//		set_underfrequency_fast(value);
	//		clear_flag(para_name);					//设置后清除数据库中参数的设置标志
	//	}
	//	else if(!strcmp("over_frequency_fast", para_name))
	//	{
	//		set_overfrequency_fast(value);
	//		clear_flag(para_name);					//设置后清除数据库中参数的设置标志
	//	}
		else if(!strcmp("under_frequency_slow", para_name))
		{
			if((5==firstinverter->model)||(6==firstinverter->model))
				set_underfrequency_slow_yc1000(firstinverter, value);
			else
				set_underfrequency_slow_yc500(firstinverter, value);

			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else if(!strcmp("over_frequency_slow", para_name))
		{
			if((5==firstinverter->model)||(6==firstinverter->model))
				set_overfrequency_slow_yc1000(firstinverter, value);
			else
				set_overfrequency_slow_yc500(firstinverter, value);

			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
	//	else if(!strcmp("voltage_triptime_fast", para_name))
	//	{
	//		set_voltage_triptime_fast(value);
	//		clear_flag(para_name);					//设置后清除数据库中参数的设置标志
	//	}
	//	else if(!strcmp("voltage_triptime_slow", para_name))
	//	{
	//		set_voltage_triptime_slow(value);
	//		clear_flag(para_name);					//设置后清除数据库中参数的设置标志
	//	}
	//	else if(!strcmp("frequency_triptime_fast", para_name))
	//	{
	//		set_frequency_triptime_fast(value);
	//		clear_flag(para_name);					//设置后清除数据库中参数的设置标志
	//	}
	//	else if(!strcmp("frequency_triptime_slow", para_name))
	//	{
	//		set_frequency_triptime_slow(value);
	//		clear_flag(para_name);					//设置后清除数据库中参数的设置标志
	//	}
		else if(!strcmp("grid_recovery_time", para_name))
		{
			if((5==firstinverter->model)||(6==firstinverter->model))
				set_grid_recovery_time_yc1000(firstinverter, value);
			else
				set_grid_recovery_time_yc500(firstinverter, value);

			clear_flag(para_name);					//设置后清除数据库中参数的设置标志
		}
		else
			break;
	}

	return count;
}

/*解析参数，并保存到数据库中*/
int resolve_protection_paras(inverter_info *inverter, char *readbuff, int size)
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


	/*
	if(42 == size)		//13项参数
	{
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
	*/
	if(33 == size)		//5项参数
	{
		/*
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
		 */

		if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))	//电压保护下限
			temp = (readbuff[7]*65535 + readbuff[8]*256 + readbuff[9])/26204.64;

		if((5==inverter->model)||(6==inverter->model))
			temp = (readbuff[7]*65535 + readbuff[8]*256 + readbuff[9])/11614.45;

		if((temp-(int)temp)>0.5)
			under_voltage_slow = (int)temp +1;
		else
			under_voltage_slow = (int)temp;

		if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))	//电压保护上限
			temp = (readbuff[10]*65535 + readbuff[11]*256 + readbuff[12])/26204.64;
		if((5==inverter->model)||(6==inverter->model))
			temp = (readbuff[10]*65535 + readbuff[11]*256 + readbuff[12])/11614.45;
		if((temp-(int)temp)>0.5)
			over_voltage_slow = (int)temp +1;
		else
			over_voltage_slow = (int)temp;

		if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))
			under_frequency_slow = 223750.0/(readbuff[13]*256 + readbuff[14]);
		if((5==inverter->model)||(6==inverter->model))
			under_frequency_slow = 256000.0/(readbuff[13]*256 + readbuff[14]);



		if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))
			over_frequency_slow = 223750.0/(readbuff[15]*256 + readbuff[16]);
		if((5==inverter->model)||(6==inverter->model))
			over_frequency_slow = 256000.0/(readbuff[15]*256 + readbuff[16]);

		grid_recovery_time = readbuff[17]*256 + readbuff[18];




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
}

/*向逆变器读取设置的值*/
int get_parameters_from_inverter(inverter_info *inverter)
{
	int i=0, ret;
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};


	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	print2msg(inverter->inverterid, "Query protect parameter");
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(readbuff,inverter);
	if((33 == ret) && (0xDD == readbuff[3]) && (0xFB == readbuff[0]) && (0xFB == readbuff[1]) && (0xFE == readbuff[31]) && (0xFE == readbuff[32]))
	{
		resolve_protection_paras(inverter, readbuff, ret);
		return 0;
	}
	else
		return -1;

}

/*读取每一台逆变器的保护参数*/
int read_protection_parameters(inverter_info *firstinverter)
{
	int i, j;
	inverter_info *inverter = firstinverter;

	memset(set_result, '\0', sizeof(set_result));

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
	{
		for(j=0; j<3; j++)
		{
			if(!get_parameters_from_inverter(inverter))
				break;
		}
	}

	return 0;
}

/*设置逆变器的保护参数*/
int set_protection_parameters(inverter_info *firstinverter)
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
#endif

