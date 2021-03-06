#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID

int send_grid_env_command_single(char *id, char value)		//单台设置逆变器的电网类型
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

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
	sendbuff[20] = 0xA8;			//CMD
	sendbuff[21] = value;

	for(i=2; i<22; i++){
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	printhexmsg("Set grid environment to single", sendbuff, 26);

	write(plcmodem, sendbuff, 26);
	sleep(2);

	return 0;
}

int send_grid_env_command_all(char value)		//广播设置逆变器的电网类型
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
	sendbuff[20] = 0xA7;
	sendbuff[21] = value;		//CMD
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check >> 8;		//CHK
	sendbuff[24] = check;		//CHK
	sendbuff[25] = 0xFE;		//TAIL
	sendbuff[26] = 0xFE;		//TAIL

	printhexmsg("Set grid environment to all", sendbuff, 27);

	write(plcmodem, sendbuff, 27);
	sleep(10);
	
	return 0;
}

int get_reply_single(char *id, char *data)			//读取逆变器的返回帧
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
		print2msg(id, "Get reply time out");
		return -1;
	}
	else
	{
		size = read(plcmodem, data, 255);

		printhexmsg(id, data, size);
		return size;
	}
}

int resolve_grid_environment(char *id, char *readbuff)
{
	sqlite3 *db=NULL;
	char sql[1024];
	char inverter_result[65535]={'\0'};
	char *zErrMsg = 0;
	int i, mode;
	
	mode = ((readbuff[21] >> 4) & 0x01) * 2 + ((readbuff[21] >> 3) & 0x01);

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
			return -1;
	strcpy(sql,"CREATE TABLE IF NOT EXISTS grid_environment(id VARCHAR(15), result INTEGER, set_value INTEGER, set_flag INTEGER, primary key(id));");	//create data table
	sqlite3_exec_3times(db, sql);

//	sprintf(sql,"UPDATE grid_environment SET result=%d WHERE id=%s", mode, id);		//把结果更新到数据库，刷新页面可以看到结果
	sprintf(sql, "REPLACE INTO grid_environment (id, result, set_flag) VALUES ('%s', %d, 0)", id, mode);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	sprintf(inverter_result, "%s%01dEND", id, mode);
	save_inverter_parameters_result2(id, 124, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA

	return 0;
}

int get_grid_environment_single(char *id)		//从逆变器读取实际的电网类型
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

	res = get_reply_single(id, readbuff);

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
			(((id[0]-0x30)<<4) + (id[1]-0x30) == readbuff[11]) &&		//TNID
			(((id[2]-0x30)<<4) + (id[3]-0x30) == readbuff[12]) &&		//TNID
			(((id[4]-0x30)<<4) + (id[5]-0x30) == readbuff[13]) &&		//TNID
			(((id[6]-0x30)<<4) + (id[7]-0x30) == readbuff[14]) &&		//TNID
			(((id[8]-0x30)<<4) + (id[9]-0x30) == readbuff[15]) &&		//TNID
			(((id[10]-0x30)<<4) + (id[11]-0x30) == readbuff[16]) &&		//TNID
			(0x4F == readbuff[17]) &&
			(0x00 == readbuff[18]) &&
			(0x00 == readbuff[19]) &&
			(0xDA == readbuff[20]) &&
			(0xFE == readbuff[40]) &&
			(0xFE == readbuff[41])){
		resolve_grid_environment(id, readbuff);		//解析和保存电网类型
		return 0;
	}
	else
		return -1;
}

int get_grid_env_id_value(char *id, char *value)		//从数据库中获取一台要设置电网的逆变器的ID和电网类型
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

	strcpy(sql, "SELECT id,set_value FROM grid_environment WHERE set_flag=1 LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		strcpy(id, azResult[2]);
		strcpy(value, azResult[3]);
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	if(1 == nrow)
		return 1;
	else
		return 0;
}

int clear_grid_env_flag_single(char *id)					//设置后清除数据库中参数的设置标志
{
	sqlite3 *db=NULL;
	char sql[1024];
	char *zErrMsg = 0;

	sprintf(sql, "UPDATE grid_environment SET set_flag=0 WHERE id='%s'", id);		//更新数据库表格

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int clear_grid_env_flag_all()					//设置后清除数据库中参数的设置标志
{
	FILE *fp;

	fp = fopen("/tmp/set_grid_environment.conf", "w");		//清空设置文件
	
	if(fp)
	{
		fclose(fp);
	}

	return 0;
}

int get_grid_environment_all(struct inverter_info_t *firstinverter)		//读取所有逆变器的电网类型
{
	int i, j;
	struct inverter_info_t *inverter = firstinverter;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++){
		for(j=0; j<3; j++){
			if(!get_grid_environment_single(inverter->inverterid))		//读取一台逆变器的电网类型
				break;
		}
	}
	
	return 0;
}

int set_grid_environment_single(struct inverter_info_t *firstinverter)		//设置单台逆变器电网
{
	char id[16]={'\0'};
	char value[16]={'\0'};
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0, i, speed;
	char **azResult;
	sqlite3 *db;
	
	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))		//create a database
		return 0;

	strcpy(sql, "SELECT id,set_value FROM grid_environment WHERE set_flag=1");
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		for(i=1; i<=nrow; i++)
		{
			if(1 == check_acc_time(firstinverter, azResult[i*ncolumn], 1800))
			{
				clear_grid_env_flag_single(azResult[i*ncolumn]);
				send_grid_env_command_single(azResult[i*ncolumn], atoi(azResult[i*ncolumn+1]));	//设置一台逆变器的电网
				get_grid_environment_single(azResult[i*ncolumn]);			//读取逆变器的设置结果
			}
		}
	}
	sqlite3_free_table( azResult );

//	while(1){
//		if(1 != get_grid_env_id_value(id, value))		//从数据库中获取一台要设置电网的逆变器的ID和电网类型，没有就退出
//			break;
//		else{
//			if(1 == check_plc_connected(firstinverter, id))
//			{
//				clear_grid_env_flag_single(id);
//				send_grid_env_command_single(id, atoi(value));	//设置一台逆变器的电网
//				get_grid_environment_single(id);			//读取逆变器的设置结果
//			}
//		}
//	}
	
	sqlite3_close(db);

	return 0;
}

int set_grid_environment_all(struct inverter_info_t *firstinverter)		//设置所有逆变器电网
{
	FILE *fp;
	char buff[256]={'\0'};
	char id[256]={'\0'};
	char value[256]={'\0'};
	
	fp = fopen("/tmp/set_grid_environment.conf", "r");
	if(fp)
	{
		fgets(buff, 255, fp);
		fclose(fp);
		
		if(strlen(buff)>0){
			sscanf(buff, "%[^,],%s", &id, &value);
			
			clear_grid_env_flag_all();

			if(!strcmp(id, "ALL")){
				send_grid_env_command_all(atoi(value));		//广播设置
				get_grid_environment_all(firstinverter);	//读取所有逆变器结果
			}
		}
	}
}

int get_grid_environment_from_inverters(struct inverter_info_t *firstinverter)		//设置所有逆变器电网
{
	FILE *fp;
	char buff[256]={'\0'};
	//set_grid_environment_all(firstinverter);	//在系统中有逆变器上传实时数据时才设置所有

	fp = fopen("/tmp/get_grid_environment.conf", "r");	//不设置只读
	if(fp){
		fgets(buff, sizeof(buff), fp);
		fclose(fp);

		if(!strcmp(buff, "ALL")){
			fp = fopen("/tmp/get_grid_environment.conf", "w");	//清空文件后读取
			if(fp){
				fclose(fp);
			}
			get_grid_environment_all(firstinverter);
		}
	}
}

int process_grid_environment(struct inverter_info_t *firstinverter)
{
//	FILE *fp;
//	char buff[256]={'\0'};
//	//set_grid_environment_all(firstinverter);	//在系统中有逆变器上传实时数据时才设置所有
	set_grid_environment_single(firstinverter);	//设置单台

//	fp = fopen("/tmp/get_grid_environment.conf", "r");	//不设置只读
//	if(fp){
//		fgets(buff, sizeof(buff), fp);
//		fclose(fp);
//
//		if(!strcmp(buff, "ALL")){
//			fp = fopen("/tmp/get_grid_environment.conf", "w");	//清空文件后读取
//			if(fp){
//				fclose(fp);
//			}
//			get_grid_environment_all(firstinverter);
//		}
//	}
}

int process_grid_environment_all(struct inverter_info_t *firstinverter)
{
	set_grid_environment_all(firstinverter);
	get_grid_environment_from_inverters(firstinverter);
}
