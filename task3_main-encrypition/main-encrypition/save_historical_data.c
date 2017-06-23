#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "sqlite3.h"
#include "variation.h"
#include "debug.h"

void historical_database_init()			//数据库初始化
{
	char sql[1024]={'\0'};
	sqlite3 *db;

	sqlite3_open("/home/historical_data.db", &db);	//create a database

	strcpy(sql, "CREATE TABLE IF NOT EXISTS lifetime_energy(item INTEGER, lifetime_energy REAL, PRIMARY KEY(item))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS yearly_energy(date VARCHAR(4), yearly_energy REAL, PRIMARY KEY(date))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS monthly_energy(date VARCHAR(6), monthly_energy REAL, PRIMARY KEY(date))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS daily_energy(date VARCHAR(8), daily_energy REAL, PRIMARY KEY(date))");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS each_system_power(date VARCHAR(8), time VARCHAR(6), each_system_power REAL, PRIMARY KEY(date, time))");
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);
}

int update_lifetime_energy(float current_energy)			//更新系统开通以来的历史发电量
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;
	int i=0;
	char **azResult;
	int nrow,ncolumn;
	int result;

	sqlite3_open("/home/historical_data.db", &db);

//	sprintf(sql, "SELECT lifetime_energy FROM lifetime_energy");
//	for(i=0;i<3;i++)
//	{
//		result=sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
//		sqlite3_free_table( azResult );
//		if(result==SQLITE_OK)
//			break;
//		sleep(1);
//	}
//
//	if(1==nrow)
//	{
//		sprintf(sql,"UPDATE lifetime_energy SET lifetime_energy=%f ",current_energy);
//		sqlite3_exec_3times(db, sql);
//	}
//
//	if(0==nrow)
//	{
	sprintf(sql,"REPLACE INTO lifetime_energy (item, lifetime_energy) VALUES(1, %f);", current_energy);
	sqlite3_exec_3times(db, sql);
//	}

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
		sqlite3_exec_3times(db, sql);
	}


	if(0==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"INSERT INTO yearly_energy (yearly_energy, date) VALUES(%f, '%s');",current_energy, date_time_year);
		sqlite3_exec_3times(db, sql);
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
		sqlite3_exec_3times(db, sql);
	}


	if(0==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"INSERT INTO monthly_energy (monthly_energy, date) VALUES(%f, '%s');",current_energy, date_time_month);
		sqlite3_exec_3times(db, sql);
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
		sqlite3_exec_3times(db, sql);
	}

	if(0==nrow)
	{
		memset(sql, '\0', sizeof(sql));
		sprintf(sql,"INSERT INTO daily_energy (daily_energy, date) VALUES(%f, '%s');",current_energy, date_time_day);
		sqlite3_exec_3times(db, sql);
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

	print2msg("date_time",date_time);

	strncpy(date,date_time,8);
	strncpy(time,&date_time[8],6);

	memset(sql, '\0', sizeof(sql));
	sprintf(sql,"INSERT INTO each_system_power (each_system_power, date, time) VALUES(%f, '%s', '%s');",system_power, date, time);
	sqlite3_exec_3times(db, sql);

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
