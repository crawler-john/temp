#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"
#include "debug.h"


extern int zbmodem;				//zigbee串口
extern unsigned char ccuid[7];		//ECU3501的ID
extern int zb_broadcast_cmd(char *buff, int length);		//zigbee广播包头
extern int zb_shortaddr_cmd(int shortaddr, char *buff, int length);		//zigbee 短地址包头
extern int zb_shortaddr_reply(char *data,int shortaddr,char *id);			//读取逆变器的返回帧,短地址形式


int send_ird_command_single(int shortaddr, char value)		//单台设置逆变器ird
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0xC6;			//CMD
	sendbuff[4] = value;		//DATA
	sendbuff[5] = 0x00;
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

	printhexmsg("Set IRD to single", sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, sendbuff, 13);
	sleep(2);

	return 0;
}

int send_ird_command_all(char value)		//广播设置逆变器ird
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0xA6;			//CMD
	sendbuff[4] = value;		//DATA
	sendbuff[5] = 0x00;
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

	printhexmsg("Set IRD to all", sendbuff, 13);
	zb_broadcast_cmd(sendbuff, 13);
	sleep(10);
	
	return 0;
}

int resolve_ird(char *id, char *readbuff)		//解析并保存IRD设置结果
{
	sqlite3 *db=NULL;
	char sql[1024];
	char inverter_result[65535]={'\0'};
	char *zErrMsg = 0;
	int i, mode;
	

	mode = ((readbuff[3+1] >> 1) & 0x03);

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
			return -1;
//	strcpy(sql,"CREATE TABLE IF NOT EXISTS ird(id VARCHAR(15), result INTEGER, set_value INTEGER, set_flag INTEGER, primary key(id));");	//create data table
//	sqlite3_exec_3times(db, sql);

//	sprintf(sql,"UPDATE ird SET result=%d WHERE id=%s", mode, id);		//把结果更新到数据库，刷新页面可以看到结果
	sprintf(sql, "REPLACE INTO ird (id, result, set_flag) VALUES ('%s', %d, 0)", id, mode);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	sprintf(inverter_result, "%s%01dEND", id, mode);				//这里先注释掉
	save_inverter_parameters_result2(id, 126, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA

	return 0;
}

int resolve_ird_DD(char *id, char *readbuff)		//解析并保存IRD设置结果
{
	sqlite3 *db=NULL;
	char sql[1024];
	char inverter_result[65535]={'\0'};
	char *zErrMsg = 0;
	int i, mode;


	mode = (int)readbuff[3+19];

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
			return -1;
//	strcpy(sql,"CREATE TABLE IF NOT EXISTS ird(id VARCHAR(15), result INTEGER, set_value INTEGER, set_flag INTEGER, primary key(id));");	//create data table
//	sqlite3_exec_3times(db, sql);

//	sprintf(sql,"UPDATE ird SET result=%d WHERE id=%s", mode, id);		//把结果更新到数据库，刷新页面可以看到结果
	sprintf(sql, "REPLACE INTO ird (id, result, set_flag) VALUES ('%s', %d, 0)", id, mode);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	sprintf(inverter_result, "%s%01dEND", id, mode);				//这里先注释掉
	save_inverter_parameters_result2(id, 126, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA

	return 0;
}

int get_ird_single(int shortaddr,char* id)		//从逆变器读取实际IRD
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;
	fd_set rd;
	struct timeval timeout;

	clear_zbmodem();			//发送指令前，先清空缓冲区
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
	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;
	sendbuff[12] = 0xFE;

	print2msg(id, "Query protect parameter");
	zb_shortaddr_cmd(shortaddr, sendbuff, 13);

	res = zb_shortaddr_reply(readbuff,shortaddr,id);


	if((58 == res) &&
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0xDA == readbuff[3]) &&
			(0xFE == readbuff[56]) &&
			(0xFE == readbuff[57]))
	{
		resolve_ird(id, readbuff);		//解析和保存IRD
		return 0;
	}
	else if ((33 == res) &&
				(0xFB == readbuff[0]) &&
				(0xFB == readbuff[1]) &&
				(0xDD == readbuff[3]) &&
				(0xFE == readbuff[31]) &&
				(0xFE == readbuff[32])) {
		resolve_ird_DD(id, readbuff);		//解析和保存IRD
		return 0;
	}
	else
		return -1;

}

int get_ird_id_value(char *id, char *value)		//从数据库中获取一台要设置电网的逆变器的ID和IRD
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

	strcpy(sql, "SELECT ird.id,ird.set_value,id.short_address FROM ird LEFT JOIN id ON ird.id=id.id WHERE ird.set_flag=1 LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		strcpy(id, azResult[3]);
		strcpy(value, azResult[4]);
		if (NULL != azResult[5]) {
			shortaddr = atoi(azResult[5]);
		} else {
			shortaddr = 0;
		}
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	if(1 == nrow)
		return shortaddr;
	else
		return -1;
}

int clear_ird_flag_single(char *id)					//设置后清除数据库中参数的设置标志
{
	sqlite3 *db=NULL;
	char sql[1024];
	char *zErrMsg = 0;

	sprintf(sql, "UPDATE ird SET set_flag=0 WHERE id='%s'", id);		//更新数据库表格

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int clear_ird_flag_all()					//设置后清除数据库中参数的设置标志
{
	FILE *fp;

	fp = fopen("/tmp/set_ird.conf", "w");		//清空设置文件
	
	if(fp)
	{
		fclose(fp);
	}

	return 0;
}

int get_ird_all(struct inverter_info_t *firstinverter)		//读取所有逆变器的IRD
{
	int i, j;
	struct inverter_info_t *inverter = firstinverter;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++){
		for(j=0; j<3; j++){
			if(!get_ird_single(inverter->shortaddr,inverter->id))		//读取一台逆变器的IRD
				break;
		}
	}
	
	return 0;
}

int set_ird_single()		//设置单台逆变器IRD
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	float temp;
	sqlite3 *db;
	int res,i;
	int shortaddr;
	char id[16]={'\0'};
	char value[16]={'\0'};

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK != res){		//create a database
		return -1;
	}

	strcpy(sql, "SELECT ird.id,ird.set_value,id.short_address FROM ird LEFT JOIN id ON ird.id=id.id WHERE ird.set_flag=1 ");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(nrow>0){
		memset(sql,'\0',1024);
		sprintf(sql, "UPDATE ird SET set_flag=0 ");
		sqlite3_exec_3times(db, sql);
		for(i=0;i<nrow;i++)
		{
			memset(id,'\0',16);
			memset(value,'\0',16);
			strcpy(id, azResult[3*i+3]);
			strcpy(value, azResult[3*i+4]);
//			clear_ird_flag_single(id);
			if (NULL != azResult[3*i+5]) {
				shortaddr = atoi(azResult[3*i+5]);
			} else {
				shortaddr = 0;
			}
			if(shortaddr>0)
			{
				send_ird_command_single(shortaddr, atoi(value));	//设置一台逆变器IRD
				get_ird_single(shortaddr,id);			//读取逆变器的设置结果
			}
		}
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	return 0;
}

int set_ird_all(inverter_info *firstinverter)		//设置所有逆变器IRD
{
	FILE *fp;
	char buff[256]={'\0'};
	char id[256]={'\0'};
	char value[256]={'\0'};
	
	fp = fopen("/tmp/set_ird.conf", "r");
	if(fp)
	{
		fgets(buff, 255, fp);
		fclose(fp);
		
		if(strlen(buff)>0){
			sscanf(buff, "%[^,],%s", &id, &value);
			
			clear_ird_flag_all();

			if(!strcmp(id, "ALL")){
				send_ird_command_all(atoi(value));		//广播设置
				get_ird_all(firstinverter);	//读取所有逆变器结果
			}
		}
	}
}

int get_ird_from_inverters(inverter_info *firstinverter)		//设置所有逆变器IRD
{
	FILE *fp;
	char buff[256]={'\0'};
	//set_grid_environment_all(firstinverter);	//在系统中有逆变器上传实时数据时才设置所有

	fp = fopen("/tmp/get_ird.conf", "r");	//不设置只读
	if(fp){
		fgets(buff, sizeof(buff), fp);
		fclose(fp);

		if(!strcmp(buff, "ALL")){
			fp = fopen("/tmp/get_ird.conf", "w");	//清空文件后读取
			if(fp){
				fclose(fp);
			}
			get_ird_all(firstinverter);
		}
	}
}

int process_ird(inverter_info *firstinverter)
{
	set_ird_single();
//	FILE *fp;
//	char buff[256]={'\0'};
//	//set_grid_environment_all(firstinverter);	//在系统中有逆变器上传实时数据时才设置所有

//	char inverter_id[16];
//	int shortaddr;
//
////	set_ird_single(firstinverter);	//设置单台
//
//	while(1){
//		if(1 != set_ird_single(inverter_id, shortaddr))
//			break;
//	}

//	fp = fopen("/tmp/get_ird.conf", "r");	//不设置只读
//	if(fp){
//		fgets(buff, sizeof(buff), fp);
//		fclose(fp);
//
//		if(!strcmp(buff, "ALL")){
//			fp = fopen("/tmp/get_ird.conf", "w");	//清空文件后读取
//			if(fp){
//				fclose(fp);
//			}
//			get_ird_all(firstinverter);
//		}
//	}
}

int process_ird_all(inverter_info *firstinverter)
{
	set_ird_all(firstinverter);
	get_ird_from_inverters(firstinverter);
}
