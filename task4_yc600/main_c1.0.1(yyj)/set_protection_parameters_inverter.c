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
#include "debug.h"

extern int zbmodem;				//zigbee串口
extern unsigned char ccuid[7];		//ECU3501的ID
extern int caltype;		//计算方式，NA版和非NA版的区别
char set_result[65535];
extern int zb_get_reply(char *data,inverter_info *inverter);			//读取逆变器的返回帧
extern int zb_broadcast_cmd(char *buff, int length);		//zigbee广播包头
extern int get_recovery_time_from_inverter(inverter_info *inverter);
extern int resolve_protection_paras(inverter_info *inverter, char *readbuff, int size);
extern int get_parameters_from_inverter(inverter_info *inverter);
extern int zb_shortaddr_cmd(int shortaddr, char *buff, int length);		//zigbee 短地址包头
extern int zb_shortaddr_reply(char *data,int shortaddr,char *id);			//读取逆变器的返回帧,短地址形式

int set_protection_yc600_one(int shortaddr,int order,int data,int num)
{

	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = order;			//CMD
	sendbuff[4] = 0x00;
	sendbuff[5] = 0x00;
	sendbuff[6] = 0x00;

	if(num==3)
	{
		sendbuff[4]=data/65536;
		sendbuff[5] = (data%65536/256);
		sendbuff[6] = (data%256);
	}
	else if(num==2)
	{
		sendbuff[4] = (data/256);
		sendbuff[5] = (data%256);
	}
	else if(num==1)
		sendbuff[4] = data;
	else ;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set yc600 single", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;

}

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
	int shortaddr;

	
	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK != res){		//create a database
		return -1;
	}

	strcpy(sql, "SELECT set_protection_parameters_inverter.id, set_protection_parameters_inverter.parameter_name , set_protection_parameters_inverter.parameter_value , id.short_address  FROM set_protection_parameters_inverter LEFT JOIN id ON set_protection_parameters_inverter.id=id.id WHERE set_protection_parameters_inverter.set_flag=1 LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );

	if(1 == nrow){
		strcpy(id, azResult[4]);
		strcpy(para_name, azResult[5]);
		strcpy(value, azResult[6]);
		if(azResult[7] != NULL)
			shortaddr = atoi(azResult[7]);
		else
			shortaddr = 0;
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	if(1 == nrow)
		return shortaddr;
	else
		return -1;
}

int get_under_voltage_slow_one_yc1000(char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int under_voltage_slow=0xFFFF, res, i;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)		//create a database
	{
		sprintf(sql, "SELECT parameter_value FROM set_protection_parameters_inverter WHERE parameter_name='under_voltage_slow' AND id='%s'", id);
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
				print2msg("Get under_voltage_slow one", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}

	}
	sqlite3_close(db);

	return under_voltage_slow;
}

int get_over_voltage_slow_one_yc1000(char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	int over_voltage_slow=0xFFFF, res, i;
	float temp;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)	//create a database
	{
		sprintf(sql, "SELECT parameter_value FROM set_protection_parameters_inverter WHERE parameter_name='over_voltage_slow' AND id='%s'", id);
		for(i=0; i<3; i++){
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
				print2msg("Get under_voltage_slow one", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}
	sqlite3_close(db);

	return over_voltage_slow;
}

char get_under_frequency_slow_one_yc1000(char *id)
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
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			{
				if(1 == nrow)
				{
					under_frequency_slow = (256000/atoi(azResult[1]));
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
			{
				print2msg("Get under_voltage_slow one", zErrMsg);
				sqlite3_free_table( azResult );
			}
		}
	}
	sqlite3_close(db);

	return under_frequency_slow;
}

char get_over_frequency_slow_one_yc1000(char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	char over_frequency_slow=0xFF, res, i;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK == res)//create a database
	{
		sprintf(sql, "SELECT parameter_value FROM set_protection_parameters_inverter WHERE parameter_name='over_frequency_slow' AND id='%s'", id);
		for(i=0; i<3; i++){
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
				if(1 == nrow)
				{
					over_frequency_slow = (256000/atoi(azResult[1]));
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
	sqlite3_close(db);

	return over_frequency_slow;
}



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
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++){
			recovery_time_result = get_recovery_time_from_inverter(inverter);
			if(-1 != recovery_time_result)
				return recovery_time_result;
		}
	}

	return recovery_time;
}


int set_regulated_dc_working_point_yc1000_one(int shortaddr, char *value)  			//直流稳压设置
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 4096/82.5);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x60;			//CMD
	sendbuff[4] = data%65536/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set dc voltage stabilization one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}

int set_undervoltage_slow_yc1000_one(int shortaddr, char *value)  //内围电压下限值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x63;			//LENGTH
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set undervoltage slow one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}

int set_overvoltage_slow_yc1000_one(int shortaddr, char *value)  //内围电压上限值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x64;			//LENGTH
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set overvoltage slow one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}

int set_overvoltage_fast_yc1000_one(int shortaddr, char *value)		//外围电压上限值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value) * 16500);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x62;			//LENGTH
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set overvoltage fast one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}

int set_undervoltage_fast_yc1000_one(int shortaddr, char *value)		//外围电压下限值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value) * 16500);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x61;			//LENGTH
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set undervoltage fast one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}


int set_underfrequency_fast_yc1000_one(int shortaddr, char *value)				//外围频率下限
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x67;			//LENGTH
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set underfrequency fast one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}


int set_overfrequency_fast_yc1000_one(int shortaddr, char *value)				//外围频率上限
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x68;			//LENGTH
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set overfrequency fast one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}


int set_underfrequency_slow_yc1000_one(int shortaddr, char *value)				//内围频率下限
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x69;			//LENGTH
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set underfrequency slow one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}


int set_overfrequency_slow_yc1000_one(int shortaddr, char *value)				//内围频率上限
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x6A;			//LENGTH
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set overfrequency slow one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}



int set_voltage_triptime_fast_yc1000_one(int shortaddr, char *value)				//外围电压延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x65;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set voltage triptime fast one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}


int set_voltage_triptime_slow_yc1000_one(int shortaddr, char *value)				//内围电压延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x66;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set voltage triptime slow one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}


int set_frequency_triptime_fast_yc1000_one(int shortaddr, char *value)				//外围频率延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6B;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set frequency triptime fast one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}



int set_frequency_triptime_slow_yc1000_one(int shortaddr, char *value)				//内围频率延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6C;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set frequency triptime slow one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}


int set_grid_recovery_time_yc1000_one(int shortaddr, char *value)						//并网恢复时间设置
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6D;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid


	printhexmsg("Set grid recovery time one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}


int set_under_voltage_stage_2_yc1000_one(int shortaddr, char *value)  			//内内围电压设置
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6E;			//CMD
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set voltage slow2 one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}



int set_voltage_3_clearance_time_yc1000_one(int shortaddr, char *value)				//内内围电压延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6F;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set voltage triptime slow2 one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}

int set_start_time_yc1000_one(int shortaddr, char *value)				//直流启动时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value);


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x4D;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set DC startime ", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

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

//

/*给逆变器设置保护参数，并且读取逆变器设置后的保护参数*/
int set_protection_paras_one(inverter_info *firstinverter)
{
	char inverter_id[16]={'\0'};
	char para_name[64];
	char value[16];
	int shortaddr,model;
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0,data=0;
	char **azResult;
	float temp;
	sqlite3 *db;
	int res,i,j,k;
	inverter_info *inverter = firstinverter;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK != res){		//create a database
		return -1;
	}

	strcpy(sql, "SELECT set_protection_parameters_inverter.id, set_protection_parameters_inverter.parameter_name , set_protection_parameters_inverter.parameter_value , id.short_address ,id.model  FROM set_protection_parameters_inverter LEFT JOIN id ON set_protection_parameters_inverter.id=id.id WHERE set_protection_parameters_inverter.set_flag=1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(nrow > 0)
	{
		memset(sql,'\0',1024);
		sprintf(sql, "DELETE FROM set_protection_parameters_inverter");
		sqlite3_exec_3times(db, sql);
		for(i=0;i<nrow;i++)
		{
			memset(inverter_id,'\0',16);
			memset(para_name,'\0',64);
			memset(value,'\0',16);
			shortaddr = 0;
			model = 0;
			strcpy(inverter_id, azResult[5*i+5]);
			strcpy(para_name, azResult[5*i+6]);
			strcpy(value, azResult[5*i+7]);
			if(azResult[8] != NULL)
				shortaddr = atoi(azResult[8]);
			if(azResult[9] != NULL)
				model = atoi(azResult[9]);

			if((shortaddr>0)&&((model==5)||(model==6)||(model==7)))
			{
				if(!strcmp("under_voltage_fast", para_name))
				{
					if(model==7){
						data = (int)(atof(value) * 1.3277);
						set_protection_yc600_one(shortaddr,0x61,data,2);
					}
					else
						set_undervoltage_fast_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("over_voltage_fast", para_name))
				{
					if(model==7){
						data = (int)(atof(value) * 1.3277);
						set_protection_yc600_one(shortaddr,0x62,data,2);
					}
					else
						set_overvoltage_fast_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("under_voltage_slow", para_name))
				{
					if(model==7){
						data = (int)(atof(value) * 1.3277);
						set_protection_yc600_one(shortaddr,0x83,data,2);
					}
					else{
						set_undervoltage_slow_yc1000_one(shortaddr, value);
						set_undervoltage_slow_yc1000_one_5(shortaddr, value, inverter_id);
						set_under_voltage_stage_3_yc1000_one(shortaddr,value);
					}
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("over_voltage_slow", para_name))
				{
					if(model==7){
						data = (int)(atof(value) * 1.3277);
						set_protection_yc600_one(shortaddr,0x64,data,2);
					}
					else{
						set_overvoltage_slow_yc1000_one(shortaddr, value);
						set_overvoltage_slow_yc1000_one_5(shortaddr, value, inverter_id);
					}
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("under_frequency_fast", para_name))
				{
					if(model==7)
					{
						data = (int)(50000000/atof(value));
						set_protection_yc600_one(shortaddr,0x68,data,3);
					}
					else
						set_underfrequency_fast_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("over_frequency_fast", para_name))
				{
					if(model==7)
					{
						data = (int)(50000000/atof(value));
						set_protection_yc600_one(shortaddr,0x67,data,3);
					}
					else
						set_overfrequency_fast_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("under_frequency_slow", para_name))
				{
					if(model==7){
						data = (int)(50000000/atof(value));
						set_protection_yc600_one(shortaddr,0x6A,data,3);
					}
					else{
						set_underfrequency_slow_yc1000_one(shortaddr, value);
						set_underfrequency_slow_yc1000_one_5(shortaddr, value, inverter_id);
					}
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("over_frequency_slow", para_name))
				{
					if(model==7){
						data = (int)(50000000/atof(value));
						set_protection_yc600_one(shortaddr,0x69,data,3);
					}
					else{
						set_overfrequency_slow_yc1000_one(shortaddr, value);
						set_overfrequency_slow_yc1000_one_5(shortaddr, value, inverter_id);
					}
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("voltage_triptime_fast", para_name))
				{
					if(model==7){
						data = (int)(atof(value)*100);
						set_protection_yc600_one(shortaddr,0x65,data,2);
					}
					else
						set_voltage_triptime_fast_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("voltage_triptime_slow", para_name))
				{
					if(model==7)
					{
						data = (int)(atof(value)*100);
						set_protection_yc600_one(shortaddr,0x66,data,2);
					}
					else
						set_voltage_triptime_slow_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("frequency_triptime_fast", para_name))
				{
					if(model==7){
						data = (int)(atof(value)*100);
						set_protection_yc600_one(shortaddr,0x6B,data,2);
					}
					else
						set_frequency_triptime_fast_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("frequency_triptime_slow", para_name))
				{
					if(model==7){
						data = (int)(atof(value)*100);
						set_protection_yc600_one(shortaddr,0x6C,data,2);
					}
					else
						set_frequency_triptime_slow_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("grid_recovery_time", para_name))
				{
					if(model==7){
						data = (int)(atof(value)*100);
						set_protection_yc600_one(shortaddr,0x6D,data,2);
					}
					else{
						set_grid_recovery_time_yc1000_one(shortaddr,value);
						set_grid_recovery_time_yc1000_one_5(shortaddr, value, inverter_id);
					}
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("regulated_dc_working_point", para_name))
				{
					if(model==7){
						data = (int)(atof(value) * 4096/82.5);
						set_protection_yc600_one(shortaddr,0x60,data,2);
					}
					else
						set_regulated_dc_working_point_yc1000_one(shortaddr,value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("under_voltage_stage_2", para_name))
				{
					if(model==7){
						data = (int)(atof(value) * 1.3277);
						set_protection_yc600_one(shortaddr,0x6E,data,2);
					}
					else
						set_under_voltage_stage_2_yc1000_one(shortaddr,value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("voltage_3_clearance_time", para_name))
				{
					if(model==7){
						data = (int)(atof(value)*100);
						set_protection_yc600_one(shortaddr,0x6F,data,2);
					}
					else
						set_voltage_3_clearance_time_yc1000_one(shortaddr,value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("start_time", para_name))
				{
					if(model==7){
						data = (int)(atof(value)*100);
						set_protection_yc600_one(shortaddr,0x4D,data,2);
					}
					else
						set_start_time_yc1000_one(shortaddr, value);
//					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
				else if(!strcmp("power_factor", para_name))
				{
					if(model==7){
						data=(int)(atof(value));
						set_protection_yc600_one(shortaddr,0x4F,data,1);
					}
				}
				else if(!strcmp("relay_protect", para_name))
				{
					if(model==7){
						data=(int)(atof(value));
						if(data==0)
							set_protection_yc600_one(shortaddr,0x4A,0,0);
						else if(data==1)
							set_protection_yc600_one(shortaddr,0x4B,0,0);
						else ;
							//set_voltage_3_clearance_time_yc1000(value);
//						clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
					}
				}
				else ;
//					clear_flag_one(inverter_id,para_name);
			}
		}
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++){
			for(k=0;k<nrow;k++)
			{
				memset(inverter_id,'\0',16);
				strcpy(inverter_id, azResult[5*i+5]);
				if(!strcmp(inverter->id, inverter_id)){
					for(j=0; j<3; j++){
						if(!get_parameters_from_inverter(inverter))
							break;
					}
					break;
				}
			}
		}
		sqlite3_close(db);
		return 1;
	}
	else
		sqlite3_close(db);
	return 0;
}

/*读取每一台逆变器的保护参数*/
int read_protection_parameters_one(inverter_info *firstinverter, char *id)
{
	int i, j;
	inverter_info *inverter = firstinverter;

	memset(set_result, '\0', sizeof(set_result));

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++){
		if(!strcmp(inverter->id, id)){
			for(j=0; j<3; j++){
				if(!get_parameters_from_inverter(inverter))
					break;
			}
		}
	}

	return 0;
}

/*设置逆变器的保护参数*/
int set_protection_parameters_inverter_one(struct inverter_info_t *firstinverter)
{
	set_protection_paras_one(firstinverter);
//	FILE *fp;
//	char buff[16];
//	int flag;
//	char inverter_id[16];
//
//
//	while(1){
//		if(1 != set_protection_paras_one(inverter_id))
//			break;
//		else
//			read_protection_parameters_one(firstinverter, inverter_id);
//	}

	return 0;
}

//读取单点设置的交流保护参数（5项）
int read_protection_parameters_one_5(char *id, char *data)
{
	sqlite3 *db = NULL;
	char sql[1024] = {'\0'};
	char *zErrMsg = 0;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	int grid_recovery_time = 0;
	int under_voltage_slow = 0;
	int over_voltage_slow = 0;
	int under_frequency_slow = 0;
	int over_frequency_slow = 0;
	int i;
	float temp;

	//查询数据
	sqlite3_open("/home/database.db", &db);
	sprintf(sql, "SELECT "
				"under_voltage_slow, "
				"over_voltage_slow, "
				"under_frequency_slow, "
				"over_frequency_slow, "
				"grid_recovery_time"
			" FROM protection_parameters WHERE id='%s' ", id);
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	for (i=1; i<=nrow; i++) {
		under_voltage_slow = (int)(atof(azResult[i*ncolumn]) * 11614.45);

		over_voltage_slow = (int)(atof(azResult[i*ncolumn + 1]) * 11614.45);

		under_frequency_slow = (256000/atof(azResult[i*ncolumn + 2]));

		over_frequency_slow = (256000/atof(azResult[i*ncolumn + 3]));

		grid_recovery_time = (int)atof(azResult[i*ncolumn + 4]);
	}
	sqlite3_free_table(azResult);
	sqlite3_close(db);

	//拼接数据
	data[0] = under_voltage_slow/65535;
	data[1] = under_voltage_slow%65535/256;
	data[2] = under_voltage_slow%256;

	data[3] = over_voltage_slow/65535;
	data[4] = over_voltage_slow%65535/256;
	data[5] = over_voltage_slow%256;

	data[6] = under_frequency_slow/256;
	data[7] = under_frequency_slow%256;

	data[8] = over_frequency_slow/256;
	data[9] = over_frequency_slow%256;

	data[10] = grid_recovery_time/256;
	data[11] = grid_recovery_time%256;

	data[12] = 0x00;
	data[13] = 0x00;
	data[14] = 0x00;

	return 0;
}

//内内围电压下限值(5项)
int set_undervoltage_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int under_voltage_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	under_voltage_slow = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = under_voltage_slow/65536;
	sendbuff[5] = under_voltage_slow%65536/256;
	sendbuff[6] = under_voltage_slow%256;
	sendbuff[7] = protection_data[3];
	sendbuff[8] = protection_data[4];
	sendbuff[9] = protection_data[5];
	sendbuff[10] = protection_data[6];
	sendbuff[11] = protection_data[7];
	sendbuff[12] = protection_data[8];
	sendbuff[13] = protection_data[9];
	sendbuff[14] = protection_data[10];
	sendbuff[15] = protection_data[11];
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];

	for (i=2; i<19; i++) {
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg("Set undervoltage slow one (5)", sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, sendbuff, 23);
	usleep(200000);

	return 0;
}

//内围电压上限值(5项)
int set_overvoltage_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int over_voltage_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	over_voltage_slow = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = protection_data[0];
	sendbuff[5] = protection_data[1];
	sendbuff[6] = protection_data[2];
	sendbuff[7] = over_voltage_slow/65536;
	sendbuff[8] = over_voltage_slow%65536/256;
	sendbuff[9] = over_voltage_slow%256;
	sendbuff[10] = protection_data[6];
	sendbuff[11] = protection_data[7];
	sendbuff[12] = protection_data[8];
	sendbuff[13] = protection_data[9];
	sendbuff[14] = protection_data[10];
	sendbuff[15] = protection_data[11];
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];

	for(i=2; i<19; i++){
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg("Set overvoltage slow one (5)", sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, sendbuff, 23);
	usleep(200000);

	return 0;
}

//内围频率下限(5项)
int set_underfrequency_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int under_frequency_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	under_frequency_slow = (256000/atof(value));

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = protection_data[0];
	sendbuff[5] = protection_data[1];
	sendbuff[6] = protection_data[2];
	sendbuff[7] = protection_data[3];
	sendbuff[8] = protection_data[4];
	sendbuff[9] = protection_data[5];
	sendbuff[10] = under_frequency_slow/256;
	sendbuff[11] = under_frequency_slow%256;
	sendbuff[12] = protection_data[8];
	sendbuff[13] = protection_data[9];
	sendbuff[14] = protection_data[10];
	sendbuff[15] = protection_data[11];
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];


	for (i=2; i<19; i++) {
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg("Set underfrequency slow one (5)", sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, sendbuff, 23);
	usleep(200000);

	return 0;
}

//内围频率上限(5项)
int set_overfrequency_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int over_frequency_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	over_frequency_slow = (256000/atof(value));

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = protection_data[0];
	sendbuff[5] = protection_data[1];
	sendbuff[6] = protection_data[2];
	sendbuff[7] = protection_data[3];
	sendbuff[8] = protection_data[4];
	sendbuff[9] = protection_data[5];
	sendbuff[10] = protection_data[6];
	sendbuff[11] = protection_data[7];
	sendbuff[12] = over_frequency_slow/256;
	sendbuff[13] = over_frequency_slow%256;
	sendbuff[14] = protection_data[10];
	sendbuff[15] = protection_data[11];
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];

	for (i=2; i<19; i++) {
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg("Set overfrequency slow one (5)", sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, sendbuff, 23);
	usleep(200000);

	return 0;
}

//并网恢复时间设置(5项)
int set_grid_recovery_time_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int grid_recovery_time;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	grid_recovery_time = atof(value);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = protection_data[0];
	sendbuff[5] = protection_data[1];
	sendbuff[6] = protection_data[2];
	sendbuff[7] = protection_data[3];
	sendbuff[8] = protection_data[4];
	sendbuff[9] = protection_data[5];
	sendbuff[10] = protection_data[6];
	sendbuff[11] = protection_data[7];
	sendbuff[12] = protection_data[8];
	sendbuff[13] = protection_data[9];
	sendbuff[14] = grid_recovery_time/256;
	sendbuff[15] = grid_recovery_time%256;
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];

	for (i=2; i<19; i++) {
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg("Set grid recovery time one (5)", sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, sendbuff, 23);
	usleep(200000);

	return 0;
}

//欠压门限3阶
int set_under_voltage_stage_3_yc1000_one(int shortaddr, char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x83;			//CMD
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg("Set under voltage (stage 3) one", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	usleep(200000);

	return 0;
}
