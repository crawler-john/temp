#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"
#include "datetime.h"
#include "debug.h"
#define DEBUGINFO2 2

sqlite3 *database_init()			//数据库初始化
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db;

	sqlite3_open("/home/database.db", &db);	//create a database

	strcpy(sql,"CREATE TABLE IF NOT EXISTS set_protection_parameters(parameter_name VARCHAR(256), parameter_value REAL, set_flag INTEGER, primary key(parameter_name))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql,"CREATE TABLE IF NOT EXISTS set_protection_parameters_inverter(id VARCHAR(256), parameter_name VARCHAR(256), parameter_value REAL, set_flag INTEGER, primary key(id, parameter_name))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql,"CREATE TABLE IF NOT EXISTS turn_on_off(id VARCHAR(256), set_flag INTEGER, primary key(id))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql,"CREATE TABLE IF NOT EXISTS clear_gfdi(id VARCHAR(256), set_flag INTEGER, primary key(id))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS process_result(item INTEGER, result VARCHAR, flag INTEGER, PRIMARY KEY(item))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS inverter_process_result(item INTEGER, inverter VARCHAR, result VARCHAR, flag INTEGER, PRIMARY KEY(item, inverter))");	//create data table
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS ird(id VARCHAR(256), result INTEGER, set_value INTEGER, set_flag INTEGER, primary key(id))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS grid_environment(id VARCHAR(256), result INTEGER, set_value INTEGER, set_flag INTEGER, primary key(id))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS signal_strength(id VARCHAR(256), signal_strength INTEGER, set_flag INTEGER, primary key(id))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS restore_inverters(id VARCHAR(15) PRIMARY KEY, restore_result INTEGER, restore_time VARCHAR(256), restore_flag INTEGER)");	//create data table
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS inverter_version(id VARCHAR(15) PRIMARY KEY, model INTEGER, version INTEGER, read_time VARCHAR(256), get_flag INTEGER)");	//create data table
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS update_inverter(id VARCHAR(15) PRIMARY KEY, update_result INTEGER, update_time VARCHAR(256), update_flag INTEGER)");	//create data table
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS protection_parameters(id VARCHAR(15), type INTEGER, under_voltage_fast INTEGER, "
			"over_voltage_fast INTEGER, under_voltage_slow INTEGER, over_voltage_slow INTEGER, under_frequency_fast REAL, "
			"over_frequency_fast REAL, under_frequency_slow REAL, over_frequency_slow REAL, voltage_triptime_fast REAL, "
			"voltage_triptime_slow REAL, frequency_triptime_fast REAL, frequency_triptime_slow REAL, grid_recovery_time INTEGER, "
			"under_voltage_stage_2 INTEGER, voltage_3_clearance_time REAL, regulated_dc_working_point REAL, start_time INTEGER, "
			"set_flag INTEGER, primary key(id));");	//create data table
	sqlite3_exec_3times(db, sql);

	strcpy(sql,"CREATE TABLE IF NOT EXISTS need_id(correct_id VARCHAR(256),wrongid VARCHAR(256),set_flag INTEGER, primary key(correct_id))");
	sqlite3_exec_3times(db, sql);

	//sql = "CREATE TABLE Data(item int, record VARCHAR(7440));";
	/*strcpy(sql,"CREATE TABLE Data(item INTEGER, record VARCHAR(74400), resendflag VARCHAR(20));");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );	//create data table

	//sql = "CREATE TABLE Event(eve VARCHAR(50),device VARCHAR(20),date VARCHAR(20),details VARCHAR(50));";
	memset(sql,'\0',100);
	strcpy(sql,"CREATE TABLE Event(eve VARCHAR(20),device VARCHAR(20),date VARCHAR(20));");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );	//create event table

	memset(sql,'\0',100);
	strcpy(sql,"CREATE TABLE ltpower(item INTEGER , power REAL)");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	memset(sql,'\0',100);
	strcpy(sql,"CREATE TABLE tdpower(date VARCHAR(10) , todaypower REAL)");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	memset(sql,'\0',100);
	strcpy(sql,"CREATE TABLE id(item INTEGER,ID VARCHAR(15), flag INTEGER)");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );*/

	return db;
}

int record_db_init()			//数据库初始化
{
	char sql[1024]={'\0'};
	sqlite3 *db;

	sqlite3_open("/home/record.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS inverter_status (item INTEGER PRIMARY KEY AUTOINCREMENT, result VARCHAR, date_time VARCHAR, flag INTEGER)");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS inverter_status_single (item INTEGER PRIMARY KEY AUTOINCREMENT, id VARCHAR, result VARCHAR, date_time VARCHAR, flag INTEGER)");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS Data(item INTEGER, record VARCHAR(74400), resendflag VARCHAR(20), date_time VARCHAR(16))");
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);

	return 0;
}

int operation_db_init()
{
	char sql[1024]={'\0'};
	sqlite3 *db;

	sqlite3_open("/home/operation.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS turned_on_off_operation (id VARCHAR(256), turned_off_opreation INTEGER)");
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);

	return 0;
}

int sqlite3_exec_3times(sqlite3 *db, char *sql)
{
	int i;
	char *zErrMsg = 0;

	for(i=0; i<100; i++){
		if(SQLITE_OK == sqlite3_exec(db, sql , 0, 0, &zErrMsg))
			return 0;
		else{
			printmsg(zErrMsg);
			usleep(500000);
		}
	}

	return -1;
}

int sqlite3_exec_100times(sqlite3 *db, char *sql)
{
	int i;
	char *zErrMsg = 0;

	for(i=0; i<100; i++){
		if(SQLITE_OK == sqlite3_exec(db, sql , 0, 0, &zErrMsg))
			return 0;
		else{
			printmsg(zErrMsg);
			usleep(500000);
		}
	}

	return -1;
}

int save_inverter_parameters_result(struct inverter_info_t *inverter, int item, char *inverter_result)
{
	sqlite3 *db=NULL;
	char sql[65535];

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sprintf(sql, "REPLACE INTO inverter_process_result(item, inverter, result, flag) VALUES(%d, '%s', '%s', 1)", item, inverter->inverterid, inverter_result);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);

	return 0;
}

int save_inverter_parameters_result_id(char *id, int item, char *inverter_result)
{
	sqlite3 *db=NULL;
	char sql[65535];

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sprintf(sql, "REPLACE INTO inverter_process_result(item, inverter, result, flag) VALUES(%d, '%s', '%s', 1)", item, id, inverter_result);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);

	return 0;
}

int save_inverter_parameters_result2(char *id, int item, char *inverter_result)
{
	sqlite3 *db=NULL;
	char sql[65535];

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sprintf(sql, "REPLACE INTO inverter_process_result(item, inverter, result, flag) VALUES(%d, '%s', '%s', 1)", item, id, inverter_result);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);

	return 0;
}

sqlite3 *create_tmpdb()			//创建临时数据库初始化
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *tmpdb;

	sqlite3_open("/home/tmpdb", &tmpdb);	//create a database

	strcpy(sql,"CREATE TABLE IF NOT EXISTS powerinfo(id VARCHAR(15), powera REAL, powerb REAL, time INTEGER, primary key(id));");	//create data table
	sqlite3_exec( tmpdb , sql , 0 , 0 , &zErrMsg );

	print2msg("Create tmpdb", zErrMsg);

	return tmpdb;
}

int init_tmpdb(sqlite3 *tmpdb, struct inverter_info_t *firstinverter)
{
	int i;
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow,ncolumn;
	struct inverter_info_t *curinverter = firstinverter;

	strcpy(sql, "SELECT id FROM powerinfo;");
	sqlite3_get_table( tmpdb , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	sqlite3_free_table( azResult );
	if(0 == nrow){
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
			memset(sql, '\0', sizeof(sql));
			sprintf(sql, "INSERT INTO powerinfo VALUES('%s', 0, 0, 0);", curinverter->inverterid);
			sqlite3_exec(tmpdb, sql, 0, 0, &zErrMsg);
		}

		return 0;
	}

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
		memset(sql, '\0', sizeof(sql));
		sprintf(sql, "SELECT powera,powerb,time FROM powerinfo WHERE id='%s';", curinverter->inverterid);
		sqlite3_get_table( tmpdb , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(1 == nrow){
			curinverter->preaccgen = atof(azResult[3]);
			curinverter->preaccgenb = atof(azResult[4]);
			curinverter->preacctime = atoi(azResult[5]);
		}
		sqlite3_free_table( azResult );
		if(0 == nrow){
			memset(sql, '\0', sizeof(sql));
			sprintf(sql, "INSERT INTO powerinfo VALUES('%s', 0, 0, 0);", curinverter->inverterid);
			sqlite3_exec(tmpdb, sql, 0, 0, &zErrMsg);
			curinverter->preaccgen = 0;
			curinverter->preaccgenb = 0;
			curinverter->preacctime = 0;
		}
	}
}

int update_tmpdb(sqlite3 *tmpdb, struct inverter_info_t *firstinverter)
{
	int i;
	char sql[100]={'\0'};
	char *zErrMsg=0;
	struct inverter_info_t *curinverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
		if('1' == curinverter->curflag){
			memset(sql, '\0', sizeof(sql));
			sprintf(sql, "UPDATE powerinfo SET powera=%f,powerb=%f,time=%d WHERE id='%s';", curinverter->preaccgen, curinverter->preaccgenb, curinverter->preacctime, curinverter->inverterid);
			sqlite3_exec(tmpdb, sql, 0, 0, &zErrMsg);
		}
	}
}

float get_lifetime_power(sqlite3 *db)			//读取系统历史发电量
{
	float lifetime_power=0;
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow,ncolumn,item;
	int i;

	memset(sql,'\0',100);
	strcpy(sql,"SELECT * FROM ltpower ");
	for(i=0; i<3; i++){
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
			break;
		}
		else
			sqlite3_free_table( azResult );
	}
	item=atoi(azResult[2]);
	lifetime_power = atof(azResult[3]);
	sqlite3_free_table( azResult );

	printfloatmsg("lifetime generation", lifetime_power);

	return lifetime_power;
}

void set_lifetime_power(sqlite3 *db, float lifetime_power)			//更新系统历史发电量
{
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE ltpower SET power=%f WHERE item=1 ",lifetime_power);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	printfloatmsg("Write lifetime power", lifetime_power);
}

int getpresetdata(sqlite3 *db, char *presetdata, int type)			//读取用户设置的配置参数值
{
	char sql[100] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	int lv1, uv1, lv2, uv2, i;
	float temp;
	
	strcpy(sql, "SELECT * FROM preset");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	
	if(2 == type)
		temp = atof(azResult[9]) * 2.93;
	else if(1 == type)
		temp = atof(azResult[9]) * 2.90345;
	else
		temp = atof(azResult[9]) * 1.48975;
	if((temp-(int)temp)>=0.5)
		lv1 = temp + 1;
	else
		lv1 = temp;
	
	if(2 == type)
		temp = atof(azResult[10]) * 2.93;
	else if(1 == type)
		temp = atof(azResult[10]) * 2.90345;
	else
		temp = atof(azResult[10]) * 1.48975;
	if((temp-(int)temp)>=0.5)
		uv1 = temp + 1;
	else
		uv1 = temp;
	
	if(2 == type)
		temp = atof(azResult[11]) * 2.93;
	else if(1 == type)
		temp = atof(azResult[11]) * 2.90345;
	else
		temp = atof(azResult[11]) * 1.48975;
	if((temp-(int)temp)>=0.5)
		lv2 = temp + 1;
	else
		lv2 = temp;
	
	if(2 == type)
		temp = atof(azResult[12]) * 2.93;
	else if(1 == type)
		temp = atof(azResult[12]) * 2.90345;
	else
		temp = atof(azResult[12]) * 1.48975;
	if((temp-(int)temp)>=0.5)
		uv2 = temp + 1;
	else
		uv2 = temp;
	
	presetdata[0] = lv1/256;
	presetdata[1] = lv1%256;
	presetdata[2] = uv1/256;
	presetdata[3] = uv1%256;
	presetdata[4] = lv2/256;
	presetdata[5] = lv2%256;
	presetdata[6] = uv2/256;
	presetdata[7] = uv2%256;
	
	if(1 == type){
		//if((atoi(azResult[13])-500)<0)
		presetdata[8] = 600 - atoi(azResult[13]);
		presetdata[9] = atoi(azResult[14]) - 600;
		//else
			//presetdata[8] = atoi(azResult[13])-500;
	}
	else{
		presetdata[8] = 500 - atoi(azResult[13]);
		presetdata[9] = atoi(azResult[14]) - 500;
	}
	
	presetdata[10] = atoi(azResult[15])/256;
	presetdata[11] = atoi(azResult[15])%256;
	
	printhexmsg("Presetdata", presetdata, 12);
	/*presetdata[0] = atoi(azResult[6]);
	presetdata[1] = atoi(azResult[7])>>8;
	presetdata[2] = atoi(azResult[7]);
	presetdata[3] = atoi(azResult[8])-497;
	presetdata[4] = atoi(azResult[9])-503;*/
	
	sqlite3_free_table( azResult );
	
	return 0;
}

void save_record(char sendbuff[], char *date_time)			//ECU发送记录给EMA的同时，本地也保存一份
{
    char sql[1024]={'\0'};
    char db_buff[MAXINVERTERCOUNT*74+100]={'\0'};
	sqlite3 *db;

    sqlite3_open("/home/record.db", &db);
	if(strlen(sendbuff)>0) {
		if(1 == getdbsize()) {
			memset(sql, '\0', sizeof(sql));
			strcpy(sql, "DELETE FROM Data WHERE rowid=(SELECT MIN(rowid) FROM Data)");
			sqlite3_exec_100times(db , sql);
		}

		memset(db_buff, '\0', sizeof(db_buff));
		sprintf(db_buff, "INSERT INTO Data (item, record, resendflag, date_time) "
				"VALUES(%d , '%s', '1', '%s');", 1, sendbuff, date_time);
		sqlite3_exec_100times(db, db_buff);
	}
	sqlite3_close(db);
}

void resendrecord(sqlite3 *db)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	int i, res;

	int fd_sock;

	fd_sock = createsocket();
	connect_socket(fd_sock);

	strcpy(sql,"SELECT * FROM Data WHERE resendflag = '1'");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );

	for(i=nrow; i>=1; i--){
		if(1 != i)
			azResult[i*ncolumn+1][78] = '1';
		res = resend_data(fd_sock, azResult[i*ncolumn+1]);
		if((-1 == res) || (0 == res))
			break;
		else{
			memset(sql,'\0',100);
      			sprintf(sql,"UPDATE Data SET resendflag='%c' WHERE item=%d ",'0', atoi(azResult[i*ncolumn]));
      			sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
		}
	}

	sqlite3_free_table( azResult );
	close_socket(fd_sock);

	printdecmsg("nrow", nrow);
}

void gprs_resendrecord(sqlite3 *db, int fd_sock)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	int i;

	strcpy(sql,"SELECT * FROM Data WHERE resendflag = '1'");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );

	for(i=1; i<=nrow; i++){
		if(1 != gprssendrecord(fd_sock, azResult[i*ncolumn+1]))
			break;
		else{
			memset(sql,'\0',100);
      			sprintf(sql,"UPDATE Data SET resendflag='%c' WHERE item=%d ",'0', atoi(azResult[i*ncolumn]));
      			sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
		}
	}

	sqlite3_free_table( azResult );

	printdecmsg("nrow", nrow);
}

void insert_event(sqlite3 *db, char inverter_id[], char status[])
{
	char event_buff[100]={'\0'};
	char db_time[20]={'\0'};
	char *zErrMsg=0;

	get_time(db_time);

	printmsg("Insert event into database");
	printf("status=%x%x,inverter_id=%s,db_time=%s\n",status[0],status[1],inverter_id,db_time);

	sprintf(event_buff,"INSERT INTO \"Event\" VALUES( '%s' , '%s' , '%s' );",status,inverter_id,db_time);
	print2msg(inverter_id, status);
	sqlite3_exec( db , event_buff , 0 , 0 , &zErrMsg );
}

int gettnuid( sqlite3 *db , struct inverter_info_t *firstinverter)	//获取逆变器的ID号，配置标志，禁止自动上报标志
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	int i;
	struct inverter_info_t *inverter = firstinverter;

	//strcpy(sql,"SELECT ID,flag,turnoff_rpt_id_flag FROM id");
	strcpy(sql,"SELECT ID,flag FROM id");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );		//ncolumn=3

	for(i=1;i<=nrow;i++){
		strcpy(inverter->inverterid, azResult[ncolumn*i]);		//获取ID
		inverter->configflag = atoi(azResult[ncolumn*i+1]);		//获取配置标志
		//inverter->turnoffautorpidflag = atoi(azResult[ncolumn*i+2]);	//获取禁止自动上报标志
		inverter++;
	}
	sqlite3_free_table( azResult );

	printmsg("-------------------");
	inverter = firstinverter;
	for(i=1;i<=nrow;i++,inverter++)
		printdecmsg(inverter->inverterid,inverter->configflag);
	printmsg("-------------------");
	printdecmsg("total", nrow);

	return nrow;
}

int getinverterid( sqlite3 *db , struct inverter_info_t *firstinverter)		//获取逆变器的ID号
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	int i;
	struct inverter_info_t *inverter = firstinverter;

	strcpy(sql,"SELECT ID FROM id");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );

	for(i=0;i<nrow;i++){
		strcpy(inverter->inverterid, azResult[i+1]);
		inverter++;
	}
	sqlite3_free_table( azResult );

	return nrow;

}
int update_flag(sqlite3 *db, struct inverter_info_t *inverter)
{
	char sql[100] = {'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE id SET flag=1 WHERE ID=%s ", inverter->inverterid);
      	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
}

/*int update_turnoff_rpt_id_flag(sqlite3 *db, struct inverter_info_t *inverter)	//更新逆变器禁止自动上报标志位
{
	char sql[100] = {'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE id SET turnoff_rpt_id_flag=%d WHERE ID=%s ", inverter->turnoffautorpidflag, inverter->inverterid);
      	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
}*/

void insertid( sqlite3 *db , struct inverter_info_t *firstinverter)		//保存逆变器的ID号等信息
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	int j;
	struct inverter_info_t *inverter = firstinverter;

	memset(sql,'\0',sizeof(sql));
	strcpy(sql,"DELETE FROM id WHERE item >= 0;");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	for(j=0; j<MAXINVERTERCOUNT; j++) {
		if(strlen(inverter->inverterid) == 12) {
			memset(sql,'\0',sizeof(sql));
			//sprintf(sql,"INSERT INTO id (item,ID,flag,turnoff_rpt_id_flag) VALUES(%d,'%s',%d, %d);", j, inverter->inverterid,0,inverter->turnoffautorpidflag);
			sprintf(sql,"INSERT INTO id (item,ID,flag) VALUES(%d,'%s',%d);", j, inverter->inverterid,0);
			sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
			print2msg("Insert ID into database", zErrMsg);
			inverter++;
		}
	}

	inverter = firstinverter;
	memset(sql,'\0',sizeof(sql));
	strcpy(sql, "DELETE FROM power WHERE item >= 0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	for(j=0; j<MAXINVERTERCOUNT; j++){
		if(strlen(inverter->inverterid) == 12){
			memset(sql,'\0',sizeof(sql));
			sprintf(sql,"INSERT INTO \"power\" VALUES(%d,'%s',250,'-',250,'-',%d);", j, inverter->inverterid, 0); //flag初始值置为0
			sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
			inverter++;
		}
	}
}

void todaypower(sqlite3 *db, float currentpower)			//更新系统当天的发电量，每次把当前一轮的发电量传入，把当天的发电量读出，相加后再存入数据库
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	int nrow,ncolumn,i;
	char **azResult;
	char date[10]={'\0'};
	float todaypower=0;

	getdate(date);
	strcpy(sql,"SELECT * FROM tdpower");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(nrow==0){
		memset(sql,'\0',100);
		sprintf(sql,"INSERT INTO \"tdpower\" VALUES('%s',%f);",date,currentpower);
		sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	}
	else if(!strcmp(azResult[2],date)){
		todaypower=atof(azResult[3])+currentpower;
		memset(sql,'\0',100);
		sprintf(sql,"UPDATE tdpower SET todaypower=%f WHERE date='%s' ",todaypower,date);
		sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	}
	else{
		memset(sql,'\0',100);
		sprintf(sql,"DELETE FROM tdpower WHERE date = '%s'",azResult[2]);//
		sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
		memset(sql,'\0',100);
		sprintf(sql,"INSERT INTO \"tdpower\" VALUES('%s',%f);",date,currentpower);
		sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	}

	sqlite3_free_table( azResult );

	printfloatmsg("Write current power", currentpower);
	strcpy(sql,"SELECT * FROM tdpower");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	for(i=0;i<nrow;i=i+2)
		print2msg(azResult[i], azResult[i+1]);
}

/*int saverecord(sqlite3 *db, char sendbuff[], char flag)
{
	insert_record( db, sendbuff, flag);//database;
	
	return 0;
}*/

int saveevent(sqlite3 *db, struct inverter_info_t *inverter, char *sendcommanddatatime)			//保存系统当前一轮出现的7种事件
{
	int i=0;
	char event_buff[1024]={'\0'};
	char datatime[20]={'\0'};
	char *zErrMsg=0;
	char sql[1024]={'\0'};
	int nrow=0,ncolumn=0;
	char **azResult;

	strcpy(sql, "CREATE TABLE IF NOT EXISTS Event(eve VARCHAR(20),device VARCHAR(20),date VARCHAR(20));");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	strcpy(sql, "SELECT COUNT(device) FROM Event");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
		{
			print2msg("SELECT COUNT(device)", zErrMsg);
			printdecmsg("nrow", nrow);
			if(nrow > 0)
			{
				print2msg("result", azResult[1]);
				if(atoi(azResult[1]) > 10000)
				{
					memset(sql, '\0', sizeof(sql));
					sprintf(sql, "DELETE FROM Event WHERE date IN (SELECT date FROM Event ORDER BY date LIMIT 1)");
					for(i=0;i<3;i++)
					{
						if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
						{
							print2msg("DELETE event", zErrMsg);
							break;
						}
						else
							print2msg("DELETE event", zErrMsg);
						sleep(1);
					}
				}
			}
			sqlite3_free_table(azResult);
			break;
		}
		print2msg("SELECT COUNT(device)", zErrMsg);
		sqlite3_free_table(azResult);
		sleep(1);
	}



	datatime[0] = sendcommanddatatime[0];
	datatime[1] = sendcommanddatatime[1];
	datatime[2] = sendcommanddatatime[2];
	datatime[3] = sendcommanddatatime[3];
	datatime[4] = '-';
	datatime[5] = sendcommanddatatime[4];
	datatime[6] = sendcommanddatatime[5];
	datatime[7] = '-';
	datatime[8] = sendcommanddatatime[6];
	datatime[9] = sendcommanddatatime[7];
	datatime[10] = ' ';
	
	datatime[11] = sendcommanddatatime[8];
	datatime[12] = sendcommanddatatime[9];
	datatime[13] = ':';
	datatime[14] = sendcommanddatatime[10];
	datatime[15] = sendcommanddatatime[11];
	datatime[16] = ':';
	datatime[17] = sendcommanddatatime[12];
	datatime[18] = sendcommanddatatime[13];
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){
		if('1'==inverter->flag){
			if(strncmp(inverter->status_web, "000000000000000000000000", strlen(inverter->status_web))){
				memset(event_buff, '\0', sizeof(event_buff));
				sprintf(event_buff,"INSERT INTO \"Event\" VALUES( '%s' , '%s' , '%s' );", inverter->status_web, inverter->inverterid, datatime);
				sqlite3_exec( db , event_buff , 0 , 0 , &zErrMsg );
			}
		}
		inverter++;
	}
}

int get_protect_parameters(int *max_vol, int *min_vol, int *max_fre, int *min_fre, int *boot_t)
{
	sqlite3 *db;
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	char sql[1024];
	int i;

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "SELECT lv2,uv2,lf,uf,rt FROM preset");
	for(i=0; i<3; i++)
	{
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
		{
			*max_vol = atoi(azResult[nrow*ncolumn]);
			*min_vol = atoi(azResult[nrow*ncolumn + 1]);
			*max_fre = atoi(azResult[nrow*ncolumn + 2]);
			*min_fre = atoi(azResult[nrow*ncolumn + 3]);
			*boot_t = atoi(azResult[nrow*ncolumn + 4]);
			sqlite3_free_table(azResult);
			break;
		}
		sqlite3_free_table(azResult);
	}
	sqlite3_close(db);

	return 0;
}

/*int save_process_result(char *result)	//设置保护参数，功率等完成后，把结果保存到数据库中，control_client把结果发送到EMA
{
	sqlite3 *db;
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	int i;
	char sql[1024];

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "CREATE TABLE IF NOT EXISTS process_result(item INTEGER, result VARCHAR, flag INTEGER)");

	for(i=0;i<3;i++)
	{
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
		{
			strcpy(sql, "SELECT COUNT(item) FROM process_result");
			for(i=0;i<3;i++)
			{
				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
				{
					if(nrow > 0)
					{
						if(atoi(azResult[1]) > 100)
						{
							sprintf(sql, "DELETE FROM process_result WHERE date IN (SELECT item FROM process_result ORDER BY item LIMIT %d)", atoi(azResult[1]) - 100);
							for(i=0;i<3;i++)
							{
								if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
								{
									break;
								}
								else
									print2msg("DELETE process_result", zErrMsg);
								sleep(1);
							}
						}
					}
					sqlite3_free_table(azResult);

					break;
				}
				else
					print2msg("SELECT COUNT(item) FROM process_result", zErrMsg);
				sqlite3_free_table(azResult);
				sleep(1);
			}
			break;
		}
		else
			print2msg("CREATE TABLE process_result", zErrMsg);
		sleep(1);
	}

	strcpy(sql, "SELECT item FROM process_result ORDER BY item DESC LIMIT 1");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
		{
			if(0 == nrow)
				sprintf(sql, "INSERT INTO process_result (item, result, flag) VALUES(1, '%s', 1)", result);
			else
				sprintf(sql, "INSERT INTO process_result (item, result, flag) VALUES(%d, '%s', 1)", atoi(azResult[1])+1, result);
			for(i=0;i<3;i++)
			{
				if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				{
					break;
				}
				else
					print2msg("INSERT INTO process_result", zErrMsg);
				sleep(1);
			}
			sqlite3_free_table(azResult);
			break;
		}
		else
			print2msg("SELECT item FROM process_result", zErrMsg);
		sqlite3_free_table(azResult);
		sleep(1);
	}

	sqlite3_close(db);

}*/

int save_process_result(int item, char *result)	//设置保护参数，功率等完成后，把结果保存到数据库中，control_client把结果发送到EMA
{
	sqlite3 *db;
	char *zErrMsg=0;
	char sql[65535];
	int i;

	sqlite3_open("/home/database.db", &db);

	sprintf(sql, "REPLACE INTO process_result (item, result, flag) VALUES(%d, '%s', 1)", item, result);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);

}

int save_status(char *result, char *date_time)	//设置保护参数，功率等完成后，把结果保存到数据库中，control_client把结果发送到EMA
{
	sqlite3 *db;
	char *zErrMsg=0;
	char sql[65535]={'\0'};
	int i;

	sqlite3_open("/home/record.db", &db);

	strcpy(sql, "DELETE FROM inverter_status WHERE item<(SELECT MAX(item)-10000 FROM inverter_status)");
	sqlite3_exec_3times(db, sql);

	memset(sql ,'\0', sizeof(sql));
	sprintf(sql, "INSERT INTO inverter_status (result, date_time, flag) VALUES('%s', '%s', 1)", result, date_time);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);
}

int save_status_single(char *id, char *result, char *date_time)	//单台逆变器的补发事件后，把结果保存到数据库中，control_client把结果发送到EMA
{
	sqlite3 *db;
	char *zErrMsg=0;
	char sql[65535]={'\0'};
	int i;

	sqlite3_open("/home/record.db", &db);

	strcpy(sql, "DELETE FROM inverter_status_single WHERE item<(SELECT MAX(item)-10000 FROM inverter_status_single)");
	sqlite3_exec_3times(db, sql);

	memset(sql ,'\0', sizeof(sql));
	sprintf(sql, "INSERT INTO inverter_status_single (id, result, date_time, flag) VALUES('%s', '%s', '%s', 1)", id, result, date_time);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);
}
