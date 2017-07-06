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
#include "debug.h"

extern int zbmodem;				//zigbee串口
extern unsigned char ccuid[7];		//ECU3501的ID
extern int caltype;		//计算方式，NA版和非NA版的区别
char set_result[65535];
extern int zb_get_reply(char *data,inverter_info *inverter);			//读取逆变器的返回帧
extern int zb_send_cmd(inverter_info *inverter, char *buff, int length);		//zigbee包头
extern int zb_broadcast_cmd(char *buff, int length);		//zigbee广播包头

/*yc600*/
int set_protection_yc600(int order,int data,int num)
{	printf("data=%d\n",data);

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

	printhexmsg("Set yc600 all", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;

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
	sqlite3_close(db);
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
	sqlite3_close(db);
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
					under_frequency_slow = (256000/atoi(azResult[1]));
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

	print2msg(inverter->id, "Query protect parameter");
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
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
		{
			recovery_time_result = get_recovery_time_from_inverter(inverter);
			if(-1 != recovery_time_result)
				return recovery_time_result;
		}
	}

	return recovery_time;
}



int set_regulated_dc_working_point_yc1000(char *value)  			//直流稳压设置
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 4096/82.5);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x50;			//CMD
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

	printhexmsg("Set dc voltage stabilization", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}




int set_undervoltage_slow_yc1000(char *value)  //内围电压下限值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x53;			//LENGTH
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

	printhexmsg("Set undervoltage slow", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}



int set_overvoltage_slow_yc1000(char *value)  //内围电压上限值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x54;			//LENGTH
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

	printhexmsg("Set overvoltage slow", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}


int set_overvoltage_fast_yc1000(char *value)		//外围电压上限值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value) * 16500);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x52;			//LENGTH
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

	printhexmsg("Set overvoltage fast", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}


int set_undervoltage_fast_yc1000(char *value)		//外围电压下限值
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value) * 16500);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x51;			//LENGTH
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

	printhexmsg("Set undervoltage fast", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}




int set_underfrequency_fast_yc1000(char *value)				//外围频率下限
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x57;			//LENGTH
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

	printhexmsg("Set underfrequency fast", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}


int set_overfrequency_fast_yc1000(char *value)				//外围频率上限
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x58;			//LENGTH
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

	printhexmsg("Set overfrequency fast", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}


int set_underfrequency_slow_yc1000(char *value)				//内围频率下限
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x59;			//LENGTH
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

	printhexmsg("Set underfrequency slow", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}


int set_overfrequency_slow_yc1000(char *value)				//内围频率上限
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x5A;			//LENGTH
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

	printhexmsg("Set overfrequency slow", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}



int set_voltage_triptime_fast_yc1000(char *value)				//外围电压延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x55;			//CMD
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

	printhexmsg("Set voltage triptime fast", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}


int set_voltage_triptime_slow_yc1000(char *value)				//内围电压延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x56;			//CMD
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

	printhexmsg("Set voltage triptime slow", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}



int set_frequency_triptime_fast_yc1000(char *value)				//外围频率延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x5B;			//CMD
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

	printhexmsg("Set frequency triptime fast", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}



int set_frequency_triptime_slow_yc1000(char *value)				//内围频率延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x5C;			//CMD
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

	printhexmsg("Set frequency triptime slow", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}



int set_grid_recovery_time_yc1000(char *value)						//并网恢复时间设置
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x5D;			//CMD
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


	printhexmsg("Set grid recovery time", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}



int set_under_voltage_stage_2_yc1000(char *value)  			//内内围电压设置
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x5E;			//CMD
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

	printhexmsg("Set voltage slow2", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}



int set_voltage_3_clearance_time_yc1000(char *value)				//内内围电压延迟保护时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x5F;			//CMD
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

	printhexmsg("Set voltage triptime slow2", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}

int set_start_time_yc1000(char *value)				//直流启动时间
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value);


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x3D;			//CMD
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

	zb_broadcast_cmd(sendbuff, 13);
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



/*给逆变器设置保护参数，并且读取逆变器设置后的保护参数*/
int set_protection_paras(inverter_info *firstinverter)
{
	char para_name[64];
	char value[16];
	int count = 0;
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0,data=0;
	char **azResult;
	float temp;
	sqlite3 *db;
	int res,i;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK != res)		//create a database
	{
		return -1;
	}

	strcpy(sql, "SELECT parameter_name,parameter_value FROM set_protection_parameters WHERE set_flag=1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(nrow > 0)
	{
		memset(sql,'\0',1024);
		sprintf(sql, "UPDATE set_protection_parameters SET set_flag=0");
		sqlite3_exec_3times(db, sql);
		for(i=0;i<nrow;i++)
		{
			memset(para_name,'\0',64);
			memset(value,'\0',16);
			strcpy(para_name, azResult[2*i+2]);
			strcpy(value, azResult[2*i+3]);
			if(!strcmp("under_voltage_fast", para_name))
			{
				data = (int)(atof(value) * 1.3277);
				set_protection_yc600(0x51,data,2);
				set_undervoltage_fast_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("over_voltage_fast", para_name))
			{
				data = (int)(atof(value) * 1.3277);
				set_protection_yc600(0x52,data,2);
				set_overvoltage_fast_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("under_voltage_slow", para_name))
			{
				data = (int)(atof(value) * 1.3277);
				set_protection_yc600(0x73,data,2);
				set_undervoltage_slow_yc1000(value);
				set_undervoltage_slow_yc1000_5(value);
				set_under_voltage_stage_3_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("over_voltage_slow", para_name))
			{
				data = (int)(atof(value) * 1.3277);
				set_protection_yc600(0x54,data,2);
				set_overvoltage_slow_yc1000(value);
				set_overvoltage_slow_yc1000_5(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("under_frequency_fast", para_name))
			{
				data = (int)(50000000/atof(value));
				set_protection_yc600(0x58,data,3);
				set_underfrequency_fast_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("over_frequency_fast", para_name))
			{
				data = (int)(50000000/atof(value));
				set_protection_yc600(0x57,data,3);
				set_overfrequency_fast_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("under_frequency_slow", para_name))
			{
				data = (int)(50000000/atof(value));
				set_protection_yc600(0x5A,data,3);
				set_underfrequency_slow_yc1000(value);
				set_underfrequency_slow_yc1000_5(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("over_frequency_slow", para_name))
			{
				data = (int)(50000000/atof(value));
				set_protection_yc600(0x59,data,3);
				set_overfrequency_slow_yc1000(value);
				set_overfrequency_slow_yc1000_5(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("voltage_triptime_fast", para_name))
			{
				data = (int)(atof(value)*100);
				set_protection_yc600(0x55,data,2);
				set_voltage_triptime_fast_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("voltage_triptime_slow", para_name))
			{
				data = (int)(atof(value)*100);
				set_protection_yc600(0x56,data,2);
				set_voltage_triptime_slow_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("frequency_triptime_fast", para_name))
			{
				data = (int)(atof(value)*100);
				set_protection_yc600(0x5B,data,2);
				set_frequency_triptime_fast_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("frequency_triptime_slow", para_name))
			{
				data = (int)(atof(value)*100);
				set_protection_yc600(0x5C,data,2);
				set_frequency_triptime_slow_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("grid_recovery_time", para_name))
			{
				data = (int)(atof(value)*100);
				set_protection_yc600(0x5D,data,2);
				set_grid_recovery_time_yc1000(value);
				set_grid_recovery_time_yc1000_5(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("regulated_dc_working_point", para_name))
			{
				data = (int)(atof(value) * 4096/82.5);
				set_protection_yc600(0x50,data,2);
				set_regulated_dc_working_point_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("under_voltage_stage_2", para_name))
			{
				data = (int)(atof(value) * 1.3277);
				set_protection_yc600(0x5E,data,2);
				//set_under_voltage_stage_2_yc1000(value);
				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("voltage_3_clearance_time", para_name))
			{
				data = (int)(atof(value)*100);
				set_protection_yc600(0x5F,data,2);
				set_voltage_3_clearance_time_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("start_time", para_name))
			{
				data = (int)(atof(value)*100);
				set_protection_yc600(0x3D,data,2);
				//set_start_time_yc1000(value);
				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("power_factor", para_name))
			{
				data=(int)(atof(value));
				set_protection_yc600(0x3F,data,1);
//				set_voltage_3_clearance_time_yc1000(value);
//				clear_flag(para_name);					//设置后清除数据库中参数的设置标志
			}
			else if((!strcmp("relay_protect", para_name)))
			{
				data=(int)(atof(value));
				if(data==0)
					set_protection_yc600(0x3A,0,0);
				else if(data==1)
					set_protection_yc600(0x3B,0,0);
				else ;
			}
			else ;
		}
		sqlite3_free_table( azResult );
		sqlite3_close(db);

		return nrow;
	}

	sqlite3_free_table( azResult );
	sqlite3_close(db);
	return 0;
}

/*解析参数，并保存到数据库中*/
int resolve_protection_paras_YC600(inverter_info *inverter, char *readbuff, int size)
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
	int regulated_dc_working_point;
	int under_voltage_stage_2;
	float under_frequency_fast;
	float over_frequency_fast;
	float under_frequency_slow;
	float over_frequency_slow;
	float voltage_triptime_fast;
	float voltage_triptime_slow;
	float frequency_triptime_fast;
	float frequency_triptime_slow;
	float grid_recovery_time;
	float voltage_3_clearance_time;
	float start_time;
	int power_factor;
	float relay_protect;
	float temp;


	if(58 == size)								//17项参数
	{

		temp = (readbuff[6+3]*256 + readbuff[7+3])/1.3277;
		if((temp-(int)temp)>=0.5)
			under_voltage_fast = (int)temp +1;
		else
			under_voltage_fast = (int)temp;

		temp = (readbuff[9+3]*256 + readbuff[10+3])/1.3277;

		if((temp-(int)temp)>=0.5)
			over_voltage_fast = (int)temp +1;
		else
			over_voltage_fast = (int)temp;

		temp = (readbuff[12+3]*256 + readbuff[13+3])/1.3277;
		if((temp-(int)temp)>0.5)
			under_voltage_slow = (int)temp +1;
		else
			under_voltage_slow = (int)temp;

		temp = (readbuff[15+3]*256 + readbuff[16+3])/1.3277;
		if((temp-(int)temp)>0.5)
			over_voltage_slow = (int)temp +1;
		else
			over_voltage_slow = (int)temp;


		over_frequency_fast = 50000000.0/(readbuff[17+3]*256*256 + readbuff[18+3]*256+readbuff[19+3]);

		under_frequency_fast = 50000000.0/(readbuff[20+3]*256*256 + readbuff[21+3]*256+readbuff[22+3]);

		over_frequency_slow = 50000000.0/(readbuff[23+3]*256*256 + readbuff[24+3]*256+readbuff[25+3]);

		under_frequency_slow = 50000000.0/(readbuff[26+3]*256*256 + readbuff[27+3]*256+readbuff[28+3]);

		voltage_triptime_fast = (readbuff[29+3]*256 + readbuff[30+3])/100.0;

		voltage_triptime_slow = (readbuff[33+3]*256 + readbuff[34+3])/100.0;

		frequency_triptime_fast = (readbuff[31+3]*256 + readbuff[32+3])/100.0;

		frequency_triptime_slow = (readbuff[35+3]*256 + readbuff[36+3])/100.0;

		grid_recovery_time = (readbuff[37+3]*256 + readbuff[38+3])/100.0;  //pages:read 17 protection intval=>number_format

		temp = (readbuff[39+3]*256 + readbuff[40+3])/1.3277;
		if((temp-(int)temp)>0.5)
			under_voltage_stage_2 = (int)temp +1;
		else
			under_voltage_stage_2 = (int)temp;

		voltage_3_clearance_time = (readbuff[41+3]*256 + readbuff[42+3])/100.0;

		temp = (readbuff[4+3]*256 + readbuff[44+3])*82.5/4096;
		if((temp-(int)temp)>0.5)
			regulated_dc_working_point = (int)temp +1;
		else
			regulated_dc_working_point = (int)temp;

		start_time = (readbuff[48+3]*256 + readbuff[49+3])/100.0;

		relay_protect = readbuff[51+3]*1.0;

		power_factor = (int)(readbuff[52+3]&0x1F);

		if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
				return -1;
//		strcpy(sql,"CREATE TABLE IF NOT EXISTS protection_parameters(id VARCHAR(15), type INTEGER, under_voltage_fast INTEGER, over_voltage_fast INTEGER, under_voltage_slow INTEGER, over_voltage_slow INTEGER, under_frequency_fast REAL, over_frequency_fast REAL, under_frequency_slow REAL, over_frequency_slow REAL, voltage_triptime_fast REAL, voltage_triptime_slow REAL, frequency_triptime_fast REAL, frequency_triptime_slow REAL, grid_recovery_time INTEGER, regulated_dc_working_point INTEGER, under_voltage_stage_2 INTEGER, voltage_3_clearance_time REAL, start_time INTEGER, set_flag INTEGER, primary key(id));");	//create data table
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		sprintf(sql, "REPLACE INTO protection_parameters (id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, regulated_dc_working_point, under_voltage_stage_2, voltage_3_clearance_time, start_time, power_factor,relay_protect ) VALUES('%s', %d, %d, %d, %d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %d, %d, %f, %f , %d, %f ) ", inverter->id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, regulated_dc_working_point, under_voltage_stage_2, voltage_3_clearance_time, start_time, power_factor, relay_protect);
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAA%03d%03d%03d%03d%06d%06d%06d%06d%03d%03d%06d%05dEND",
				inverter->id,
				under_voltage_slow,
				over_voltage_slow,
				(int)(under_frequency_slow*10),
				(int)(over_frequency_slow*10),
				(int)(grid_recovery_time),
				under_voltage_fast,
				over_voltage_fast,
				(int)(under_frequency_fast*10),
				(int)(over_frequency_fast*10),
				(int)(voltage_triptime_fast*100),
				(int)(voltage_triptime_slow*100),
				(int)(frequency_triptime_fast*100),
				(int)(frequency_triptime_slow*100),
				regulated_dc_working_point*10,
				under_voltage_stage_2,
				(int)(voltage_3_clearance_time*100),
				(int)(start_time));
		save_inverter_parameters_result(inverter, 131, inverter_result);

		memset(inverter_result,'\0',sizeof(inverter_result));
		if(power_factor==31)
			sprintf(inverter_result,"%sA100END",inverter->id);
		else if(power_factor>10)
			sprintf(inverter_result,"%sA%03dEND",inverter->id,(power_factor-11)*10);
		else
			sprintf(inverter_result,"%sB%03dEND",inverter->id,(power_factor-1)*10);
		save_inverter_parameters_result(inverter, 145, inverter_result);
		return 0;
	}
	else
		return -1;

}

/*解析参数，并保存到数据库中*/
int resolve_protection_paras_YC1000(inverter_info *inverter, char *readbuff, int size)
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
	int regulated_dc_working_point;
	int under_voltage_stage_2;
	float under_frequency_fast;
	float over_frequency_fast;
	float under_frequency_slow;
	float over_frequency_slow;
	float voltage_triptime_fast;
	float voltage_triptime_slow;
	float frequency_triptime_fast;
	float frequency_triptime_slow;
	int grid_recovery_time;
	float voltage_3_clearance_time;
	int start_time;
	float temp;


	if(58 == size)								//17项参数
	{

		temp = (readbuff[5+3]*65536 + readbuff[6+3]*256 + readbuff[7+3])/16500;
		if((temp-(int)temp)>=0.5)
			under_voltage_fast = (int)temp +1;
		else
			under_voltage_fast = (int)temp;

		temp = (readbuff[8+3]*65536 + readbuff[9+3]*256 + readbuff[10+3])/16500;

		if((temp-(int)temp)>=0.5)
			over_voltage_fast = (int)temp +1;
		else
			over_voltage_fast = (int)temp;

		temp = (readbuff[11+3]*65536 + readbuff[12+3]*256 + readbuff[13+3])/11614.45;
		if((temp-(int)temp)>0.5)
			under_voltage_slow = (int)temp +1;
		else
			under_voltage_slow = (int)temp;

		temp = (readbuff[14+3]*65536 + readbuff[15+3]*256 + readbuff[16+3])/11614.45;
		if((temp-(int)temp)>0.5)
			over_voltage_slow = (int)temp +1;
		else
			over_voltage_slow = (int)temp;


		under_frequency_fast = 256000.0/(readbuff[17+3]*256 + readbuff[18+3]);

		over_frequency_fast = 256000.0/(readbuff[19+3]*256 + readbuff[20+3]);

		under_frequency_slow = 256000.0/(readbuff[21+3]*256 + readbuff[22+3]);

		over_frequency_slow = 256000.0/(readbuff[23+3]*256 + readbuff[24+3]);

		voltage_triptime_fast = (readbuff[25+3]*256 + readbuff[26+3])/50.0;

		voltage_triptime_slow = (readbuff[27+3]*256 + readbuff[28+3])/50.0;

		frequency_triptime_fast = (readbuff[29+3]*256 + readbuff[30+3])/50.0;

		frequency_triptime_slow = (readbuff[31+3]*256 + readbuff[32+3])/50.0;

		grid_recovery_time = readbuff[33+3]*256 + readbuff[34+3];

		temp = (readbuff[35+3]*65536 + readbuff[36+3]*256 + readbuff[37+3])/11614.45;
		if((temp-(int)temp)>0.5)
			under_voltage_stage_2 = (int)temp +1;
		else
			under_voltage_stage_2 = (int)temp;

		voltage_3_clearance_time = (readbuff[38+3]*256 + readbuff[39+3])/50.0;

		temp = (readbuff[40+3]*256 + readbuff[41+3])*82.5/4096;
		if((temp-(int)temp)>0.5)
			regulated_dc_working_point = (int)temp +1;
		else
			regulated_dc_working_point = (int)temp;

		start_time = readbuff[45+3]*256 + readbuff[46+3];

		if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
				return -1;
//		strcpy(sql,"CREATE TABLE IF NOT EXISTS protection_parameters(id VARCHAR(15), type INTEGER, under_voltage_fast INTEGER, over_voltage_fast INTEGER, under_voltage_slow INTEGER, over_voltage_slow INTEGER, under_frequency_fast REAL, over_frequency_fast REAL, under_frequency_slow REAL, over_frequency_slow REAL, voltage_triptime_fast REAL, voltage_triptime_slow REAL, frequency_triptime_fast REAL, frequency_triptime_slow REAL, grid_recovery_time INTEGER, regulated_dc_working_point INTEGER, under_voltage_stage_2 INTEGER, voltage_3_clearance_time REAL, start_time INTEGER, set_flag INTEGER, primary key(id));");	//create data table
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		sprintf(sql, "REPLACE INTO protection_parameters (id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, regulated_dc_working_point, under_voltage_stage_2, voltage_3_clearance_time, start_time ) VALUES('%s', %d, %d, %d, %d, %f, %f, %f, %f, %f, %f, %f, %f, %d, %d, %d, %f, %d) ", inverter->id, under_voltage_fast, over_voltage_fast, under_voltage_slow, over_voltage_slow, under_frequency_fast, over_frequency_fast, under_frequency_slow, over_frequency_slow, voltage_triptime_fast, voltage_triptime_slow, frequency_triptime_fast, frequency_triptime_slow, grid_recovery_time, regulated_dc_working_point, under_voltage_stage_2, voltage_3_clearance_time, start_time);
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAA%03d%03d%03d%03d%06d%06d%06d%06d%03d%03d%06d%05dEND",
				inverter->id,
				under_voltage_slow,
				over_voltage_slow,
				(int)(under_frequency_slow*10),
				(int)(over_frequency_slow*10),
				grid_recovery_time,
				under_voltage_fast,
				over_voltage_fast,
				(int)(under_frequency_fast*10),
				(int)(over_frequency_fast*10),
				(int)(voltage_triptime_fast*100),
				(int)(voltage_triptime_slow*100),
				(int)(frequency_triptime_fast*100),
				(int)(frequency_triptime_slow*100),
				regulated_dc_working_point*10,
				under_voltage_stage_2,
				(int)(voltage_3_clearance_time*100),
				start_time);
		save_inverter_parameters_result(inverter, 131, inverter_result);


		return 0;
	}
	else
		return -1;

}


/*解析参数，并保存到数据库中*/
int resolve_protection_paras5(inverter_info *inverter, char *readbuff, int size)
{
	sqlite3 *db=NULL;
	char sql[1024];
	char inverter_result[65535]={'\0'};
	char *zErrMsg = 0;
	int i, max_power;

	int under_voltage_slow;
	int over_voltage_slow;
	float under_frequency_slow;
	float over_frequency_slow;
	int grid_recovery_time;
	float temp;

	if(33 == size)
	{
		temp = (readbuff[4+3]*65536 + readbuff[5+3]*256 + readbuff[6+3])/11614.45;
		under_voltage_slow = (int)(temp + 0.5);

		temp = (readbuff[7+3]*65536 + readbuff[8+3]*256 + readbuff[9+3])/11614.45;
		over_voltage_slow = (int)(temp + 0.5);

		under_frequency_slow = 256000.0/(readbuff[10+3]*256 + readbuff[11+3]);

		over_frequency_slow = 256000.0/(readbuff[12+3]*256 + readbuff[13+3]);

		grid_recovery_time = readbuff[14+3]*256 + readbuff[15+3];

		if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
				return -1;
//		strcpy(sql,"CREATE TABLE IF NOT EXISTS protection_parameters"
//				"(id VARCHAR(15), type INTEGER, "
//				"under_voltage_fast INTEGER, over_voltage_fast INTEGER, "
//				"under_voltage_slow INTEGER, over_voltage_slow INTEGER, "
//				"under_frequency_fast REAL, over_frequency_fast REAL, "
//				"under_frequency_slow REAL, over_frequency_slow REAL, "
//				"voltage_triptime_fast REAL, voltage_triptime_slow REAL, "
//				"frequency_triptime_fast REAL, frequency_triptime_slow REAL, "
//				"grid_recovery_time INTEGER, regulated_dc_working_point INTEGER, "
//				"under_voltage_stage_2 INTEGER, voltage_3_clearance_time REAL, "
//				"start_time INTEGER, set_flag INTEGER, primary key(id))");
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		sprintf(sql, "REPLACE INTO protection_parameters "
				"(id, under_voltage_slow, over_voltage_slow, "
				"under_frequency_slow, over_frequency_slow, grid_recovery_time) "
				"VALUES('%s', %d, %d, %f, %f, %d)",
				inverter->id,
				under_voltage_slow,
				over_voltage_slow,
				under_frequency_slow,
				over_frequency_slow,
				grid_recovery_time);
		printf("sql:%s\n", sql);
		for(i=0; i<3; i++)
		{
			if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}

		sprintf(inverter_result, "%s%03d%03d%03d%03d%05dAAAAAAAAAAAAAAAAAAAAAAAA"
				"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEND",
				inverter->id,
				under_voltage_slow,
				over_voltage_slow,
				(int)(under_frequency_slow*10),
				(int)(over_frequency_slow*10),
				grid_recovery_time);
		save_inverter_parameters_result(inverter, 131, inverter_result);
		return 0;
	}
	else
		return -1;

}

//向逆变器读取交流保护参数的值
int get_parameters_from_inverter(inverter_info *inverter)
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int i, res;

	clear_zbmodem();
	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0xDD;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0x00;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	for(i=2; i<9; i++)
	{
		check = check + sendbuff[i];
	}
	sendbuff[9] = check >> 8;
	sendbuff[10] = check;
	sendbuff[11] = 0xFE;
	sendbuff[12] = 0xFE;
	print2msg("Query Inverter's Protection Parameters", inverter->id);
	zb_send_cmd(inverter, sendbuff, 13);

	//接收返回消息
	res = zb_get_reply(readbuff,inverter);
	if((33 == res) &&
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0xDD == readbuff[3]) &&
			(0xFE == readbuff[31]) &&
			(0xFE == readbuff[32]))
	{
		//5项参数
		resolve_protection_paras5(inverter, readbuff, res);
		return 0;
	}

	if((58 == res) &&
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0xDA == readbuff[3]) &&
			(0xFE == readbuff[56]) &&
			(0xFE == readbuff[57]))
	{
		//17项参数
		if((inverter->model==5)||(inverter->model==6))
			resolve_protection_paras_YC1000(inverter, readbuff, res);
		else if(inverter->model==7)
			resolve_protection_paras_YC600(inverter, readbuff, res);
		else ;
		return 0;
	}

	return -1;
}

/*读取每一台逆变器的保护参数*/
int read_protection_parameters(inverter_info *firstinverter)
{
	int i, j;
	inverter_info *inverter = firstinverter;

	memset(set_result, '\0', sizeof(set_result));

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
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

		if(2 == flag)
		{
			read_protection_parameters(firstinverter);

			fp = fopen("/tmp/presetdata.conf", "w");
			fprintf(fp, "0");
			fclose(fp);
		}
	}

	return 0;
}

//读取广播设置的交流保护参数（5项）
int read_protection_parameters_5(char *data)
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
	strcpy(sql, "SELECT * FROM set_protection_parameters");
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	for (i=1; i<=nrow; i++) {
		temp = atof(azResult[i*ncolumn + 1]);
		if (!strcmp(azResult[i*ncolumn], "under_voltage_slow")) {
			under_voltage_slow = (int)(temp * 11614.45);
		}
		else if (!strcmp(azResult[i*ncolumn], "over_voltage_slow")) {
			over_voltage_slow = (int)(temp * 11614.45);
		}
		else if (!strcmp(azResult[i*ncolumn], "under_frequency_slow")) {
			under_frequency_slow = (256000/temp);
		}
		else if (!strcmp(azResult[i*ncolumn], "over_frequency_slow")) {
			over_frequency_slow = (256000/temp);
		}
		else if (!strcmp(azResult[i*ncolumn], "grid_recovery_time")) {
			grid_recovery_time = (int)temp;
		}
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
int set_undervoltage_slow_yc1000_5(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int under_voltage_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_5(protection_data);
	under_voltage_slow = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xAC;
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

	printhexmsg("Set undervoltage slow (5)", sendbuff, 23);

	zb_broadcast_cmd(sendbuff, 23);
	sleep(10);

	return 0;
}

//内围电压上限值(5项)
int set_overvoltage_slow_yc1000_5(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int over_voltage_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_5(protection_data);
	over_voltage_slow = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xAC;
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

	printhexmsg("Set overvoltage slow (5)", sendbuff, 23);

	zb_broadcast_cmd(sendbuff, 23);
	sleep(10);

	return 0;
}

//内围频率下限(5项)
int set_underfrequency_slow_yc1000_5(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int under_frequency_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_5(protection_data);
	under_frequency_slow = (256000/atof(value));

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xAC;
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

	printhexmsg("Set underfrequency slow (5)", sendbuff, 23);

	zb_broadcast_cmd(sendbuff, 23);
	sleep(10);

	return 0;
}

//内围频率上限(5项)
int set_overfrequency_slow_yc1000_5(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int over_frequency_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_5(protection_data);
	over_frequency_slow = (256000/atof(value));

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xAC;
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

	printhexmsg("Set overfrequency slow (5)", sendbuff, 23);

	zb_broadcast_cmd(sendbuff, 23);
	sleep(10);

	return 0;
}

//并网恢复时间设置(5项)
int set_grid_recovery_time_yc1000_5(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int grid_recovery_time;
	char protection_data[16] = {'\0'};

	read_protection_parameters_5(protection_data);
	grid_recovery_time = atof(value);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xAC;
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

	printhexmsg("Set grid recovery time (5)", sendbuff, 23);

	zb_broadcast_cmd(sendbuff, 23);
	sleep(10);

	return 0;
}

//欠压门限3阶设置
int set_under_voltage_stage_3_yc1000(char *value)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x73;			//CMD
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

	printhexmsg("Set under voltage (stage 3)", sendbuff, 13);

	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);

	return 0;
}
