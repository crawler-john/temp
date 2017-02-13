/*
 * 本文件的主要功能是设置单个逆变器的保护参数。同时兼容新的13项保护参数和老的5项参数。
 * Created by Zhyf
 * Created on 2014/08/22
 */

#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "variation.h"
#include <string.h>

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID
extern int caltype;		//计算方式，NA版和非NA版的区别
char set_result[65535];
extern int zb_get_reply(char *data,inverter_info *inverter);			//读取逆变器的返回帧
extern int zb_send_cmd(inverter_info *inverter, char *buff, int length);		//zigbee包头
int zbmodem;				//zigbee串口


int zb_send_cmd_protection_parameters(unsigned short shortaddr, char *buff, int length)		//zigbee保护参数包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = shortaddr>>8;
	sendbuff[6]  = shortaddr;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}

	printhexmsg("sendbuff",sendbuff,15+i);
	usleep(50000);
	if(0!=shortaddr)
	{
		write(zbmodem, sendbuff, length+15);
		return 1;
	}
	else
		return -1;
}

/*读取设置的参数值和标志位*/
int get_value_flag_one(char *id, unsigned short *shortaddr, char *para_name, char *value)
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

	memset(id, '\0', sizeof(id));
	strcpy(sql, "SELECT set_protection_parameters_inverter.id,short_address,parameter_name,parameter_value FROM id,set_protection_parameters_inverter WHERE set_protection_parameters_inverter.set_flag=1 AND set_protection_parameters_inverter.id=id.id LIMIT 0,1 ");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		strcpy(id, azResult[4]);
		if(NULL==azResult[5])
		{
			*shortaddr=0;		//短地址为空时候,赋短地址值为0,ZK
		}
		else
		{
			*shortaddr = atoi(azResult[5]);
		}
		strcpy(para_name, azResult[6]);
		strcpy(value, azResult[7]);
	}
	sqlite3_free_table( azResult );

/*
	memset(id, '\0', sizeof(id));
	strcpy(sql, "SELECT id,parameter_name,parameter_value FROM set_protection_parameters_inverter WHERE set_flag=1 LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		strcpy(id, azResult[3]);
		strcpy(para_name, azResult[4]);
		strcpy(value, azResult[5]);
	}
	sqlite3_free_table( azResult );

	memset(sql, '\0', sizeof(sql));
	sprintf(sql,"SELECT short_address FROM id WHERE id='%s'; ",id);
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		shortaddr=atoi(azResult[1]);
	}
	sqlite3_free_table( azResult );
*/
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


int get_grid_recovery_time_one(inverter_info *firstinverter, char *id)
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
int set_undervoltage_fast_one(unsigned short shortaddr, char *value)
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
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE1;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x04;		//ccuid
	sendbuff[8] = 0x01;		//ccuid

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage fast", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

/*设置逆变器的快速过压保护值*/
int set_overvoltage_fast_one(unsigned short shortaddr, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	char data;

	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE2;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x04;		//ccuid
	sendbuff[8] = 0x01;		//ccuid

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage fast", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

/*设置逆变器的慢速欠压保护值，有新老两种逆变器*/
int set_undervoltage_slow_one(unsigned short shortaddr, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	char data;
	int i;

	if((2 == caltype)||(1 == caltype))
		data = (int)((atof(value) * 1.452)/4);
	else
		data = (int)((atof(value) * 1.48975)/4);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE3;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x04;		//ccuid
	sendbuff[8] = 0x01;		//ccuid

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set undervoltage slow", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

/*设置逆变器的慢速过压保护值，有新老两种逆变器*/
int set_overvoltage_slow_one(unsigned short shortaddr, char *value)
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
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE4;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x04;		//ccuid
	sendbuff[8] = 0x01;		//ccuid

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set overvoltage slow", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

/*设置逆变器的快速欠频保护值*/
int set_underfrequency_fast_one(unsigned short shortaddr, char *value)
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
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE7;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x04;		//ccuid
	sendbuff[8] = 0x01;		//ccuid

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency fast", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

/*设置逆变器的快速过频保护值*/
int set_overfrequency_fast_one(unsigned short shortaddr, char *value)
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
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x00;			//LENGTH
	sendbuff[5] = 0xE8;		//ccuid
	sendbuff[6] = data;		//ccuid
	sendbuff[7] = 0x04;		//ccuid
	sendbuff[8] = 0x01;		//ccuid

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency fast", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

/*设置逆变器的慢速欠频保护值，有新老两种逆变器*/
int set_underfrequency_slow_one(unsigned short shortaddr, char *value)
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

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xE9;
	sendbuff[6] = data;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set underfrequency slow", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

/*设置逆变器的慢速过频保护值，有新老两种逆变器*/
int set_overfrequency_slow_one(unsigned short shortaddr, char *value)
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

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xEA;
	sendbuff[6] = data;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set overfrequency slow", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

int set_voltage_triptime_fast_one(unsigned short shortaddr, char *value)
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

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xE5;
	sendbuff[6] = data;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime fast", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

int set_voltage_triptime_slow_one(unsigned short shortaddr, char *value)
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

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xE6;
	sendbuff[6] = data;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set voltage triptime slow", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

int set_frequency_triptime_fast_one(unsigned short shortaddr, char *value)
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

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xEB;
	sendbuff[6] = data;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime fast", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

int set_frequency_triptime_slow_one(unsigned short shortaddr, char *value)
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

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xEC;
	sendbuff[6] = data;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set frequency triptime slow", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

/*设置逆变器的并网恢复时间，有新老两种逆变器*/
int set_grid_recovery_time_one(unsigned short shortaddr, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xED;
	sendbuff[6] = atoi(value)/32;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	printhexmsg("Set grid recovery time", sendbuff, 13);

	zb_send_cmd_protection_parameters(shortaddr, sendbuff, 13);
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
int set_protection_paras_one(inverter_info *firstinverter,char *inverter_id)
{
	char para_name[64];
	char value[16];
	unsigned short shortaddr;

	if(1 == get_value_flag_one(inverter_id, &shortaddr, para_name, value))
	{
		if(!strcmp("under_voltage_fast", para_name))
		{
			set_undervoltage_fast_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("over_voltage_fast", para_name))
		{
			set_overvoltage_fast_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("under_voltage_slow", para_name))
		{
			set_undervoltage_slow_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("over_voltage_slow", para_name))
		{
			set_overvoltage_slow_one(shortaddr,  value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("under_frequency_fast", para_name))
		{
			set_underfrequency_fast_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("over_frequency_fast", para_name))
		{
			set_overfrequency_fast_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("under_frequency_slow", para_name))
		{
			set_underfrequency_slow_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("over_frequency_slow", para_name))
		{
			set_overfrequency_slow_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("voltage_triptime_fast", para_name))
		{
			set_voltage_triptime_fast_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("voltage_triptime_slow", para_name))
		{
			set_voltage_triptime_slow_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("frequency_triptime_fast", para_name))
		{
			set_frequency_triptime_fast_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("frequency_triptime_slow", para_name))
		{
			set_frequency_triptime_slow_one(shortaddr, value);
			clear_flag_one(inverter_id, para_name);					//设置后清除数据库中参数的设置标志
		}
		if(!strcmp("grid_recovery_time", para_name))
		{
			set_grid_recovery_time_one(shortaddr, value);
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
	float temp;return

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
int read_protection_parameters_one(inverter_info *firstinverter, char *id)
{
	int i, j;
	inverter_info *inverter = firstinverter;

	memset(set_result, '\0', sizeof(set_result));

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		if(!strcmp(inverter->inverterid, id)){
			for(j=0; j<3; j++){
				if(!get_parameters_from_inverter(inverter))
					break;
			}
		}
	}

	return 0;
}

/*设置逆变器的保护参数*/
int set_protection_parameters_inverter(inverter_info *firstinverter)
{
	FILE *fp;
	char buff[16];
	int flag;
	char inverter_id[16];
	


	while(1){
		if(1 != set_protection_paras_one(firstinverter,inverter_id))
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
//		fclose(fp);set_protection_parameters_inverter
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
