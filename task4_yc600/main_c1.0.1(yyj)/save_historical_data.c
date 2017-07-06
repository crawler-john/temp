#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "sqlite3.h"
#include "variation.h"
#include "debug.h"


//extern ecu_info ecu;

//sqlite3 *db=NULL;			//数据库
//sqlite3 *tmpdb=NULL;			//临时数据库

/*
int getdbsize(void){			//获取数据库的文件大小，如果数据库超过30M后，每插入一条新的数据，就删除数据库中最早的数据
	struct stat fileinfo;
	int filesize=0;

	stat("/home/historical_data.db", &fileinfo);
	filesize=fileinfo.st_size;//filesize.st_size/1024;

	printdecmsg("Size of database", filesize);

	if(filesize >= 30000000)
		return 1;
	else
		return 0;
}

sqlite3 *init_database()			//数据库初始化
{
	sqlite3_open("/home/historical_data.db", &db);	//create a database

	return db;
}
*/
int update_lifetime_energy(float current_energy)			//更新系统开通以来的历史发电量
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;
	int i=0;
	char **azResult;
	int nrow,ncolumn;
	int result;

	sqlite3_open("/home/historical_data.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS lifetime_energy(item INTEGER, lifetime_energy REAL, PRIMARY KEY(item))");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK ==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
			break;
		sleep(1);
	}

	sprintf(sql, "SELECT lifetime_energy FROM lifetime_energy");
	for(i=0;i<3;i++)
	{
		result=sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		sqlite3_free_table( azResult );
		if(result==SQLITE_OK)
			break;
		sleep(1);
	}

	if(1==nrow)
	{
		sprintf(sql,"UPDATE lifetime_energy SET lifetime_energy=%f ",current_energy);
		for(i=0;i<3;i++)
		{
			if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		print2msg("zErrMsg",zErrMsg);
		printfloatmsg("Update lifetime energy", current_energy);
	}

	if(0==nrow)
	{
		sprintf(sql,"INSERT INTO lifetime_energy (lifetime_energy, item) VALUES(%f, %d);",current_energy,1);
		for(i=0;i<3;i++)
		{
			if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		print2msg("zErrMsg",zErrMsg);
		printfloatmsg("Update lifetime energy", current_energy);
	}

	sqlite3_close(db);
}

int update_yearly_energy(float current_energy,char *date_time)
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char date_time_year[5]={'\0'};
	int i=0;
	char **azResult;
	int nrow,ncolumn;
	int result;
	sqlite3 *db=NULL;

	sqlite3_open("/home/historical_data.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS yearly_energy(date VARCHAR(4), yearly_energy REAL, PRIMARY KEY(date))");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK ==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
			break;
		sleep(1);
	}

	strncpy(date_time_year,date_time,4);

	sprintf(sql, "SELECT yearly_energy FROM yearly_energy WHERE date='%s';",date_time_year);
	for(i=0;i<3;i++)
	{
		result=sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		sqlite3_free_table( azResult );
		if(result==SQLITE_OK)
			break;
		sleep(1);
	}


	if(1==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"UPDATE yearly_energy SET yearly_energy=%f+yearly_energy WHERE date='%s';",current_energy,date_time_year);
		for(i=0;i<3;i++)
		{
			if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		print2msg("zErrMsg",zErrMsg);
		printfloatmsg("Update yearly energy", current_energy);
	}


	if(0==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"INSERT INTO yearly_energy (yearly_energy, date) VALUES(%f, '%s');",current_energy, date_time_year);
		for(i=0;i<3;i++)
		{
			if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		print2msg("zErrMsg",zErrMsg);
		printfloatmsg("Insert yearly energy", current_energy);
	}

	sqlite3_close(db);
}

int update_monthly_energy(float current_energy,char *date_time)
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char date_time_month[7]={'\0'};
	int i=0;
	char **azResult;
	int nrow,ncolumn;
	int result;
	sqlite3 *db=NULL;


	sqlite3_open("/home/historical_data.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS monthly_energy(date VARCHAR(6), monthly_energy REAL, PRIMARY KEY(date))");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK ==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
			break;
		sleep(1);
	}

	strncpy(date_time_month,date_time,6);
	sprintf(sql, "SELECT monthly_energy FROM monthly_energy WHERE date='%s';",date_time_month);

	for(i=0;i<3;i++)
	{
		result=sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		sqlite3_free_table( azResult );
		if(result==SQLITE_OK)
			break;
		sleep(1);
	}

	if(1==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"UPDATE monthly_energy SET monthly_energy=%f+monthly_energy WHERE date='%s';",current_energy,date_time_month);

		for(i=0;i<3;i++)
		{
			if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		print2msg("zErrMsg",zErrMsg);
		printfloatmsg("Update monthly energy", current_energy);
	}


	if(0==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"INSERT INTO monthly_energy (monthly_energy, date) VALUES(%f, '%s');",current_energy, date_time_month);
		for(i=0;i<3;i++)
		{
			if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		print2msg("zErrMsg",zErrMsg);
		printfloatmsg("Insert monthly energy", current_energy);
	}

	sqlite3_close(db);
}

int update_daily_energy(float current_energy,char *date_time)
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char date_time_day[9]={'\0'};
	int i=0;
	char **azResult;
	int nrow,ncolumn;
	int result;
	sqlite3 *db=NULL;

	sqlite3_open("/home/historical_data.db", &db);
	printmsg("debug3");
	strcpy(sql, "CREATE TABLE IF NOT EXISTS daily_energy(date VARCHAR(8), daily_energy REAL, PRIMARY KEY(date))");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK ==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
			break;
		sleep(1);
	}
	strncpy(date_time_day,date_time,8);
	sprintf(sql, "SELECT daily_energy FROM daily_energy WHERE date='%s';",date_time_day);

	for(i=0;i<3;i++)
	{
		result=sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		sqlite3_free_table( azResult );
		if(result==SQLITE_OK)
			break;
		sleep(1);
	}

	if(1==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"UPDATE daily_energy SET daily_energy=%f+daily_energy WHERE date='%s';",current_energy,date_time_day);
		for(i=0;i<3;i++)
		{
			if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		print2msg("zErrMsg",zErrMsg);
		printfloatmsg("Update daily energy", current_energy);
	}

	if(0==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"INSERT INTO daily_energy (daily_energy, date) VALUES(%f, '%s');",current_energy, date_time_day);
		for(i=0;i<3;i++)
		{
			if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
				break;
			sleep(1);
		}
		print2msg("zErrMsg",zErrMsg);
		printfloatmsg("Insert daily energy", current_energy);
	}

	sqlite3_close(db);

}

int save_system_power(float system_power,char *date_time)
{
	char sql[1024]={'\0'};
	char date[9]={'\0'};
	char time[7]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow,ncolumn;
	int i;
	sqlite3 *db=NULL;

	sqlite3_open("/home/historical_data.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS each_system_power(date VARCHAR(8), time VARCHAR(6), each_system_power REAL, PRIMARY KEY(date, time))");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK ==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
			break;
		sleep(1);
	}

	print2msg("date_time",date_time);

	strncpy(date,date_time,8);
	strncpy(time,&date_time[8],6);

	memset(sql, '\0', sizeof(sql));
	sprintf(sql,"INSERT INTO each_system_power (each_system_power, date, time) VALUES(%f, '%s', '%s');",system_power, date, time);
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK==sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
			break;
		sleep(1);
	}
	print2msg("zErrMsg",zErrMsg);
	printfloatmsg("Insert each_system_power", system_power);

	sqlite3_close(db);

}

int save_system_power_single(float system_power,char *date_time)
{
	char sql[1024]={'\0'};
	char date[9]={'\0'};
	char time[7]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow,ncolumn;
	int i;
	sqlite3 *db=NULL;

	sqlite3_open("/home/historical_data.db", &db);

	print2msg("date_time",date_time);

	strncpy(date,date_time,8);
	strncpy(time,&date_time[8],6);

	memset(sql, '\0', sizeof(sql));

	sprintf(sql,"UPDATE each_system_power SET each_system_power=%f+each_system_power WHERE date='%s' AND time='%s';",system_power,date,time);

	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);

}

