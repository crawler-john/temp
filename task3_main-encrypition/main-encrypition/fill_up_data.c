/*
 * 本文件用于PLC补发数据功能
 * 使用方法：在主程序while循环前调用init_lost_data_info(),while循坏中正常轮询数据后调用fill_up_data()
 * 1.main.c文件中，main函数最后添加 durabletime = fill_up_data(inverter,(reportinterval+300+thistime),thistime)
 * 2.main.c文件中，main函数最前面添加 init_lost_data_info()
 * 3.在variation.h中增加inverter结构体变量model,在初始化逆变器函数中赋值0
 * 4.在plc.c文件中更改了getalldata函数，增加save_time_to_database函数
 * 5.在plc.c的getalldata函数前面，调用calibration_time_broadcast函数
 * 6.main.c文件中，main函数改动 time_linux = get_time(sendcommanddatetime, sendcommandtime);
 * 7.main.c文件中，main函数改动 curcount = getalldata(inverter, sendcommandtime,time_linux);
 * 8.替换datetime.c文件
 * 9.main.c文件中，float ltgeneration系统历史总发电量改为全局变量，方便fill_up_data.c文件中调用
 * 10.save_historical_data.c文件中增加save_system_power_single函数，并在fill_up_data.c文件中调用
 * 11.date_time.c文件获取时间同时返回给linux时间戳
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"
#include <time.h>
#include "debug.h"
#include "save_historical_data.h"

struct lost_data_t{
	int current_inverter;		//所有逆变器全部丢失数据的时间点中，记录当前补到的第几个逆变器
	char sendbuff[65535];		//所有逆变器全部丢失数据的时间点数据拼接结果
	char sendbuff_all[65535];		//所有逆变器全部丢失数据的时间点数据拼接结果，包括ECU的头信息
	int syspower;
	float curgeneration;
	float ltgeneration;
	int inverter_count;			//获取到补数据的逆变器个数
	int num; 					//逆变器结构体序号
};

int first_boot_mark=0;			// ECU主程序第一次启动标志位
struct lost_data_t lost_data_info;
extern char ecuid[13];			//ECU的ID
extern unsigned char ccuid[7];		//ECU3501的ID
extern int plcmodem;			//PLC的文件描述符
extern int caltype;			//计算方式，NA版和非NA版的区别
extern int displaymaxcount;
extern int zoneflag;			//修改时区的标志，改动为1， 没改动为0
extern float ltgeneration;			//系统历史总发电量
int turn_on_flag=0;			//补数据开关标志，1开始补数据功能，0关闭补数据功能


int init_lost_data_info()		//初始化补数据结构体信息，用于main.exe启动后和所有逆变器数据结束后
{
	FILE *fp;
	char buff[256] = {'\0'};

	fp = fopen("/etc/yuneng/fill_up_data.conf", "r");
	if(fp)
	{
		fgets(buff, sizeof(buff), fp);
		if(strlen(buff)>0)
			turn_on_flag = atoi(buff);
		else
			turn_on_flag = 0;
		fclose(fp);
	}
	else
	{
		turn_on_flag = 0;
		return 0;
	}

	lost_data_info.current_inverter=0;
	lost_data_info.inverter_count=0;
	memset(lost_data_info.sendbuff, '\0', sizeof(lost_data_info.sendbuff));
	memset(lost_data_info.sendbuff_all, '\0', sizeof(lost_data_info.sendbuff_all));
	lost_data_info.syspower=0;
	lost_data_info.curgeneration=0;
	lost_data_info.ltgeneration=0;
	lost_data_info.num=0;
}
/*
int plc_get_reply(char *data,struct inverter_info_t *inverter)			//读取逆变器的返回帧
{
	int ret, size;
	fd_set rd;
	struct timeval timeout;
	char inverterid[13] = {'\0'};

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if(select(plcmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
		return -1;
	}
	else
	{
		size = read(plcmodem, data, 255);
		printhexmsg("Reply", data, size);

		sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data[11],data[12],data[13],data[14],data[15],data[16]);
		if((size>0)&&(0xFB==data[0])&&(0xFB==data[1])&&(0==strcmp(inverter->inverterid,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}

	}
}
*/

int plc_get_reply_fromID(char *data,char *id)			//读取逆变器的返回帧2
{
	int ret, size;
	fd_set rd;
	struct timeval timeout;
	char inverterid[13] = {'\0'};

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if(select(plcmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
		return -1;
	}
	else
	{
		size = read(plcmodem, data, 255);
		printhexmsg("Reply", data, size);

		sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data[11],data[12],data[13],data[14],data[15],data[16]);
		if((size>0)&&(0xFB==data[0])&&(0xFB==data[1])&&(0==strcmp(id,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}

	}
}

int calibration_time_broadcast(struct inverter_info_t *firstinverter, int time_linux)	//发送时间校准广播帧
{
	time_t tm;
	unsigned char sendbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int i=0;
	struct inverter_info_t *curinverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
	{
		if(1 != curinverter->fill_up_data_flag)
			return 0;
	}
//	struct inverter_info_t *firstinverter = inverter;

//	time_linux=time(&tm);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x03;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = 0x00;		//TNID
	sendbuff[12] = 0x00;		//TNID
	sendbuff[13] = 0x00;		//TNID
	sendbuff[14] = 0x00;		//TNID
	sendbuff[15] = 0x00;		//TNID
	sendbuff[16] = 0x00;		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x0D;			//CMD
	sendbuff[21] = time_linux/16777216;
	sendbuff[22] = (time_linux%16777216)/65536;
	sendbuff[23] = ((time_linux%16777216)%65536)/256;
	sendbuff[24] = ((time_linux%16777216)%65536)%256;
	sendbuff[25] = 0x00;
	sendbuff[26] = 0x00;
	sendbuff[27] = 0x00;
	sendbuff[28] = 0x00;
	sendbuff[29] = 0x00;
	sendbuff[30] = 0x00;
	sendbuff[31] = 0x00;
	sendbuff[32] = 0x00;

	for(i=2; i<33; i++)
		check = check + sendbuff[i];

	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printmsg("Calibration time broadcast");
	write(plcmodem, sendbuff, 37);
	printhexmsg("sendbuff",sendbuff,37);

	return 1;

}

int get_inverter_model(struct inverter_info_t *inverter)		//读取逆变器的新旧版本，以确定需不需要做补数据功能
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, j, ret,flag=0;
	fd_set rd;
	sqlite3 *db;
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	char sql[1024];


	sqlite3_open("/tmp/replacement.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS get_inverter_model(inverterid VARCHAR, model INTEGER,primary key(inverterid))");
	sqlite3_exec_3times(db, sql);
	memset(sql, '\0', sizeof(sql));

	sprintf(sql, "SELECT model FROM get_inverter_model WHERE inverterid=%s",inverter->inverterid);
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	memset(sql, '\0', sizeof(sql));

	if(nrow>0)
	{
		if(NULL!=azResult[1])
		{
			if(1==atoi(azResult[1]))
			{
				inverter->fill_up_data_flag=1;					//1：有补数据功能
			}
			else if(2==atoi(azResult[1]))
			{
				inverter->fill_up_data_flag=2;					//2：无补数据功能
			}
			else if(3==atoi(azResult[1]))
			{
				inverter->fill_up_data_flag=0;					//没有响应，不确定是否有补数据功能
				flag=1;
			}
		}
		else
		{
			inverter->fill_up_data_flag=0;						//从未问询过机器型号，为第一次
			flag=1;
		}
	}
	else
	{
		flag=1;										//第一次创建表，表里没有任何东西
		inverter->fill_up_data_flag=0;						//从未问询过机器型号，为第一次
	}

	sqlite3_free_table( azResult );
	sqlite3_close(db);

	printmsg("debug44444");
	if((1==flag)&&('1' != inverter->inverterid[0]))
	{
		sendbuff[0] = 0xFB;			//HEAD
		sendbuff[1] = 0xFB;			//HEAD
		sendbuff[2] = 0x01;			//CMD
		sendbuff[3] = 0x00;			//LENGTH
		sendbuff[4] = 0x11;			//LENGTH
		sendbuff[5] = ccuid[0];		//CCID
		sendbuff[6] = ccuid[1];		//CCID
		sendbuff[7] = ccuid[2];		//CCID
		sendbuff[8] = ccuid[3];		//CCID
		sendbuff[9] = ccuid[4];		//CCID
		sendbuff[10] = ccuid[5];		//CCID
		sendbuff[11] = inverter->tnuid[0];		//TNID
		sendbuff[12] = inverter->tnuid[1];		//TNID
		sendbuff[13] = inverter->tnuid[2];		//TNID
		sendbuff[14] = inverter->tnuid[3];		//TNID
		sendbuff[15] = inverter->tnuid[4];		//TNID
		sendbuff[16] = inverter->tnuid[5];		//TNID
		sendbuff[17] = 0x4F;
		sendbuff[18] = 0x00;
		sendbuff[19] = 0x00;
		sendbuff[20] = 0xDC;
		sendbuff[21] = 0x11;

		for(i=2; i<22; i++)
		{
			check = check + sendbuff[i];
		}

		sendbuff[22] = check >> 8;		//CHK
		sendbuff[23] = check;		//CHK
		sendbuff[24] = 0xFE;		//TAIL
		sendbuff[25] = 0xFE;		//TAIL

		printhexmsg("Get inverter model", sendbuff, 26);
		write(plcmodem, sendbuff, 26);

		ret = plc_get_reply_fromID(readbuff,inverter->inverterid);

		if((37==ret)&&(0xFB == readbuff[0])&&(0xFB == readbuff[1])&&
			(sendbuff[11] == readbuff[11])&&
			(sendbuff[12] == readbuff[12])&&
			(sendbuff[13] == readbuff[13])&&
			(sendbuff[14] == readbuff[14])&&
			(sendbuff[15] == readbuff[15])&&
			(sendbuff[16] == readbuff[16])&&
			(0xDC == readbuff[20])&&(0xFE == readbuff[35])&&(0xFE == readbuff[36]))
		{
			if(0x01 == readbuff[23])
			{
				save_inverter_replacement_model(inverter->inverterid,1);		//1:有补数据功能
				inverter->fill_up_data_flag=1;
			}
			else if(0x00 == readbuff[23])
			{
				save_inverter_replacement_model(inverter->inverterid,2);		//2:无补数据功能
			//	inverter->last_replacement_data_flag=1;
				inverter->fill_up_data_flag=2;
			}
		}
		else
		{
			save_inverter_replacement_model(inverter->inverterid,3);			//3:没有回应，不确定是否有补数据功能
			inverter->fill_up_data_flag=0;
		}
	}
	printmsg("debug55555");
	return 1;
}

int save_inverter_replacement_model(char* inverterid , int flag)	//保存逆变器是否有补数据功能标志位到数据库
{
	char sql[1024]={'\0'};
	sqlite3 *db;
	char *zErrMsg=0;

	sqlite3_open("/tmp/replacement.db", &db);
	sprintf(sql, "REPLACE INTO get_inverter_model (inverterid, model) VALUES('%s', %d)", inverterid, flag);	//1:有补数据功能 2:无补数据功能 3:没有回应，不确定是否有补数据功能
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);
}

int save_replacement_time(char* inverterid , int time_linux)	//保存掉点时间和逆变器ID到数据库
{
	char sql[1024]={'\0'};
	sqlite3 *db;
	char *zErrMsg=0;
	int i;


	sqlite3_open("/tmp/replacement.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS save_replacement_time(inverterid VARCHAR, time INTEGER)");
	sqlite3_exec_3times(db, sql);

	memset(sql, '\0', sizeof(sql));
	sprintf(sql, "INSERT INTO save_replacement_time (inverterid, time) VALUES('%s', %d)", inverterid, time_linux);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);
}

int save_lost_all_time(int time_linux)	//保存全部掉包的时间点到数据库
{
	char sql[1024]={'\0'};
	sqlite3 *db;
	char *zErrMsg=0;
	int i;

	sqlite3_open("/tmp/replacement.db", &db);

	strcpy(sql, "CREATE TABLE IF NOT EXISTS lost_all_time(time INTEGER)");
	sqlite3_exec_3times(db, sql);

	memset(sql, '\0', sizeof(sql));
	sprintf(sql, "INSERT INTO lost_all_time (time) VALUES(%d)", time_linux);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);
}


int clear_single_less_than_time(int time_linux,char* inverterid)		//清除单个逆变器小于某个时间点的所有时间
{
	sqlite3 *db;
	char *zErrMsg=0;
	char sql[1024]={'\0'};

	sqlite3_open("/tmp/replacement.db", &db);
	sprintf(sql, "DELETE FROM save_replacement_time WHERE inverterid = '%s' AND time < %d", inverterid,time_linux);
	sqlite3_exec_3times(db, sql);
	sqlite3_close(db);
	return 1;
}

int clear_single_alltime(char* inverterid)		//清除单个逆变器的所有时间
{
	sqlite3 *db;
	char *zErrMsg=0;
	char sql[1024]={'\0'};

	sqlite3_open("/tmp/replacement.db", &db);
	sprintf(sql, "DELETE FROM save_replacement_time WHERE inverterid = '%s'", inverterid);
	sqlite3_exec_3times(db, sql);
	sqlite3_close(db);
	return 1;
}

int clear_single_time(int time_linux,char* inverterid)		//清除单个逆变器的单个时间
{
	sqlite3 *db;
	char *zErrMsg=0;
	char sql[1024]={'\0'};

	sqlite3_open("/tmp/replacement.db", &db);
	sprintf(sql, "DELETE FROM save_replacement_time WHERE inverterid = '%s' AND time = %d", inverterid ,time_linux);
	sqlite3_exec_3times(db, sql);
	sqlite3_close(db);
	return 1;
}

int clear_all_time(int time_linux)		//清楚所有逆变器的单个时间
{
	sqlite3 *db;
	char *zErrMsg=0;
	char sql[1024]={'\0'};

	sqlite3_open("/tmp/replacement.db", &db);
	sprintf(sql, "DELETE FROM lost_all_time WHERE time = %d", time_linux);
	sqlite3_exec_3times(db, sql);
	sqlite3_close(db);
	return 1;
}

int get_all_time()			//从数据库中读取所有逆变器丢失数据的单个时间,并返回
{
	sqlite3 *db;
	char sql[1024]={'\0'};
    char *zErrMsg=0;
    int nrow=0;
    int ncolumn;
	char **azResult;
    char first_time_all[20]={'\0'};
    int all_time_linux=0;


    sqlite3_open("/tmp/replacement.db", &db);
	strcpy(sql,"SELECT * FROM lost_all_time");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );


	if(nrow>0)
	{
		strcpy(first_time_all, azResult[1]);
		print2msg("first_time_all",first_time_all);
		all_time_linux = atoi(first_time_all);
		sqlite3_free_table( azResult );
		sqlite3_close(db);
	//	clear_all_time(all_time_linux);			//读到一个时间点就在表中删除这个时间点，ZK
		return all_time_linux;
	}
	else
	{
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return -1;
	}

}

int get_single_time(char* inverterid)		//从数据库中读取一台逆变器丢失数据数据的单个时间,查表中的第一个
{
	sqlite3 *db;
	char sql[1024]={'\0'};
    char *zErrMsg=0;
    char **azResult;
    int nrow=0,ncolumn;
    int time_linux=0;

    sqlite3_open("/tmp/replacement.db", &db);
//	sprintf(sql,"SELECT inverterid , time FROM save_replacement_time LIMIT 1");
	sprintf(sql, "SELECT time FROM save_replacement_time WHERE inverterid='%s' LIMIT 1;", inverterid);
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );


	if(nrow>0)
	{
		print2msg("inverterid",inverterid);
		print2msg("first_time",azResult[1]);
		time_linux = atoi(azResult[1]);
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return time_linux;
	}
	else
	{
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return -1;
	}
}

int get_nearest_time(char* inverterid, int time)		//从数据库中读取一台逆变器丢失数据数据的单个时间最接近的时间，逆变器和ECU的时间有误差
{
	sqlite3 *db;
	char sql[1024]={'\0'};
    char *zErrMsg=0;
    char **azResult;
    int nrow=0,ncolumn;
    int time_linux=0;

    sqlite3_open("/tmp/replacement.db", &db);
//	sprintf(sql,"SELECT inverterid , time FROM save_replacement_time LIMIT 1");
	sprintf(sql, "SELECT time FROM save_replacement_time WHERE inverterid='%s' AND time<%d AND time>%d LIMIT 1;", inverterid, time+100, time-100);
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );


	if(nrow>0)
	{
		print2msg("inverterid",inverterid);
		print2msg("nearlist time",azResult[1]);
		time_linux = atoi(azResult[1]);
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return time_linux;
	}
	else
	{
		print2msg("inverterid", "No nearest time");
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return -1;
	}
}

int check_exist(char *id, char *time)
{
	sqlite3 *db;
	char sql[1024]={'\0'};
    char *zErrMsg=0;
    char **azResult;
    int nrow=0,ncolumn;
    int ret=0;

    sqlite3_open("/home/record.db", &db);
	sprintf(sql, "SELECT record FROM Data WHERE date_time='%s';", time);
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );

	if(nrow>0)
	{
		print2msg("record",azResult[1]);
		if(NULL == strstr(azResult[1], id))
			ret = 0;
		else
			ret = 1;
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return ret;
	}
	else
	{
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return 0;
	}
}

int check_single_time()				//检查save_replacement_time表是否为空
{
	sqlite3 *db;
	char sql[1024]={'\0'};
    char *zErrMsg=0;
    char **azResult;
    int nrow=0,ncolumn;
    int time_linux=0;

    sqlite3_open("/tmp/replacement.db", &db);
	sprintf(sql,"SELECT count(*) FROM save_replacement_time ");
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		if(atoi(azResult[1])>0)
		{
			sqlite3_free_table( azResult );
			sqlite3_close(db);
			return 1;
		}
		else
		{
			sqlite3_free_table( azResult );
			sqlite3_close(db);
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

int save_time_to_database(struct inverter_info_t *firstinverter,int time_linux)
{
	int k=0;
	int m=0;
	int i, j, currentcount = 0;
	struct inverter_info_t *curinverter = firstinverter;

	if(0 == turn_on_flag)
		return 0;

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++)
	{
		get_inverter_model(curinverter);
		curinverter++;
	}

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++)
	{
		if((curinverter->flag=='0')&&(1 == curinverter->fill_up_data_flag))
			save_replacement_time(curinverter->inverterid , time_linux);		//把有数据补包功能的逆变器ID和时间保存到表格里
		curinverter++;
	}

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++)
	{
		if(curinverter->flag=='1')
		{
			k++;			// k为计数标志，如果轮询完所有逆变器，k仍然为0，则说明在此时间点，所有逆变器都没有获取到数据，就保存到所有掉点表格里
		}
		printdecmsg("k",k);
		curinverter++;
	}

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++)
	{
		if(1==curinverter->fill_up_data_flag)
		{
			m++;
		}
		printdecmsg("m",m);
		curinverter++;
	}

	if((0==k)&&(m>0))
		save_lost_all_time(time_linux);

	return 1;
}

int query_lost_data(char* inverterid , int time, char *data, char* receivetime)		//向逆变器读取丢失的数据
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
//	char receivetime[20]={'\0'};
    int temp_time;
    int i, j=0;
    int ret, size;
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = ((inverterid[0]-0x30)<<4) + (inverterid[1]-0x30);		//TNID
	sendbuff[12] = ((inverterid[2]-0x30)<<4) + (inverterid[3]-0x30);		//TNID
	sendbuff[13] = ((inverterid[4]-0x30)<<4) + (inverterid[5]-0x30);		//TNID
	sendbuff[14] = ((inverterid[6]-0x30)<<4) + (inverterid[7]-0x30);		//TNID
	sendbuff[15] = ((inverterid[8]-0x30)<<4) + (inverterid[9]-0x30);		//TNID
	sendbuff[16] = ((inverterid[10]-0x30)<<4) + (inverterid[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x0F;			//CMD
	sendbuff[21] = 0x00;
	sendbuff[22] = time/16777216;
	sendbuff[23] = (time%16777216)/65536;
	sendbuff[24] = ((time%16777216)%65536)/256;
	sendbuff[25] = ((time%16777216)%65536)%256;
	sendbuff[26] = 0x00;
	sendbuff[27] = 0x00;
	sendbuff[28] = 0x00;
	sendbuff[29] = 0x00;
	sendbuff[30] = 0x00;
	sendbuff[31] = 0x00;
	sendbuff[32] = 0x00;


	for(i=2; i<33; i++)
		check = check + sendbuff[i];

	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printmsg("Replacement_data");
	write(plcmodem, sendbuff, 37);

	printhexmsg("sendbuff",sendbuff,37);
	ret = plc_get_reply_fromID(readbuff,inverterid);

	if((64==ret)&&(0xFB == readbuff[0])&&(0xFB == readbuff[1])&&(0x01 == readbuff[2])&&
						(sendbuff[11] == readbuff[11])&&(sendbuff[12] == readbuff[12])&&
						(sendbuff[13] == readbuff[13])&&(sendbuff[14] == readbuff[14])&&
						(sendbuff[15] == readbuff[15])&&(sendbuff[16] == readbuff[16])&&
						(0xFE == readbuff[62])&&(0xFE == readbuff[63]))
	{
		memset(receivetime, '\0', sizeof(receivetime));
//		tm = readbuff[30]*16777216+readbuff[31]*65536+readbuff[32]*256+readbuff[33];
		tm = time;	//逆变器自己累计时间，返回的时间和ECU记录的时间可能有1-2秒误差，以ECU的时间为准。
		memcpy(&record_time,localtime(&tm), sizeof(record_time));

		sprintf(receivetime, "%04d%02d%02d%02d%02d%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
				record_time.tm_hour, record_time.tm_min, record_time.tm_sec);
		print2msg("Receive time", receivetime);

		if(1==readbuff[22])
		{
			for(i=0, j=34; i<26; i++, j++)
			{
				data[i] = readbuff[j];
			}
			clear_single_time(time,inverterid);
			return ret;
		}
		else
		{
			if(1==readbuff[25])
			{
				return -1;
			}
			else if(0==readbuff[25])
			{
				if(1==readbuff[24])
				{
					temp_time=readbuff[30]*16777216+readbuff[31]*65536+readbuff[32]*256+readbuff[33];
					printdecmsg("temp_time",temp_time);
					clear_single_less_than_time(temp_time,inverterid);
					return 2;
				}
				else if(0==readbuff[24])
				{
					clear_single_alltime(inverterid);
					return -1;
				}
			}
		}
	}
	else if((60==ret)&&(0xFB == readbuff[0])&&(0xFB == readbuff[1])&&(0x01 == readbuff[2])&&
						(sendbuff[11] == readbuff[11])&&(sendbuff[12] == readbuff[12])&&
						(sendbuff[13] == readbuff[13])&&(sendbuff[14] == readbuff[14])&&
						(sendbuff[15] == readbuff[15])&&(sendbuff[16] == readbuff[16])&&
						(0xFE == readbuff[58])&&(0xFE == readbuff[59]))
	{		//YC250
		memset(receivetime, '\0', sizeof(receivetime));
//		tm = readbuff[30]*16777216+readbuff[31]*65536+readbuff[32]*256+readbuff[33];
		tm = time;	//逆变器自己累计时间，返回的时间和ECU记录的时间可能有1-2秒误差，以ECU的时间为准。
		memcpy(&record_time,localtime(&tm), sizeof(record_time));

		sprintf(receivetime, "%04d%02d%02d%02d%02d%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
				record_time.tm_hour, record_time.tm_min, record_time.tm_sec);
		print2msg("Receive time", receivetime);

		if(1==readbuff[22])
		{
			for(i=0, j=34; i<22; i++, j++)
			{
				data[i] = readbuff[j];
			}
			clear_single_time(time,inverterid);
			return ret;
		}
		else
		{
			if(1==readbuff[25])
			{
				return -1;
			}
			else if(0==readbuff[25])
			{
				if(1==readbuff[24])
				{
					temp_time=readbuff[30]*16777216+readbuff[31]*65536+readbuff[32]*256+readbuff[33];
					printdecmsg("temp_time",temp_time);
					clear_single_less_than_time(temp_time,inverterid);
					return 2;
				}
				else if(0==readbuff[24])
				{
					clear_single_alltime(inverterid);
					return -1;
				}
			}
		}

	}
	else
	{
		return -1;
	}
}


int query_last_lost_data(char* inverterid , int time, char *data, char* receivetime)		//向逆变器读取丢失的昨天最后一包数据
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
//	char receivetime[20]={'\0'};
    int temp_time;
    int i, j=0;
    int ret, size;
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};
	int nearlist_time=-1;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x1C;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = ((inverterid[0]-0x30)<<4) + (inverterid[1]-0x30);		//TNID
	sendbuff[12] = ((inverterid[2]-0x30)<<4) + (inverterid[3]-0x30);		//TNID
	sendbuff[13] = ((inverterid[4]-0x30)<<4) + (inverterid[5]-0x30);		//TNID
	sendbuff[14] = ((inverterid[6]-0x30)<<4) + (inverterid[7]-0x30);		//TNID
	sendbuff[15] = ((inverterid[8]-0x30)<<4) + (inverterid[9]-0x30);		//TNID
	sendbuff[16] = ((inverterid[10]-0x30)<<4) + (inverterid[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0x0F;			//CMD
	sendbuff[21] = 0x01;
	sendbuff[22] = time/16777216;
	sendbuff[23] = (time%16777216)/65536;
	sendbuff[24] = ((time%16777216)%65536)/256;
	sendbuff[25] = ((time%16777216)%65536)%256;
	sendbuff[26] = 0x00;
	sendbuff[27] = 0x00;
	sendbuff[28] = 0x00;
	sendbuff[29] = 0x00;
	sendbuff[30] = 0x00;
	sendbuff[31] = 0x00;
	sendbuff[32] = 0x00;


	for(i=2; i<33; i++)
		check = check + sendbuff[i];

	sendbuff[33] = check >> 8;		//CHK
	sendbuff[34] = check;		//CHK
	sendbuff[35] = 0xFE;		//TAIL
	sendbuff[36] = 0xFE;		//TAIL

	printmsg("Replacement_last_data");
	write(plcmodem, sendbuff, 37);

	printhexmsg("sendbuff",sendbuff,37);
	ret = plc_get_reply_fromID(readbuff,inverterid);

	if((64==ret)&&(0xFB == readbuff[0])&&(0xFB == readbuff[1])&&(0x01 == readbuff[2])&&
						(sendbuff[11] == readbuff[11])&&(sendbuff[12] == readbuff[12])&&
						(sendbuff[13] == readbuff[13])&&(sendbuff[14] == readbuff[14])&&
						(sendbuff[15] == readbuff[15])&&(sendbuff[16] == readbuff[16])&&
						(0xFE == readbuff[62])&&(0xFE == readbuff[63]))
	{		//YC500
		memset(receivetime, '\0', sizeof(receivetime));
		tm = readbuff[30]*16777216+readbuff[31]*65536+readbuff[32]*256+readbuff[33];

		nearlist_time = get_nearest_time(inverterid, tm);
		if(-1 != nearlist_time)
			tm = nearlist_time;
		clear_single_time(tm,inverterid);	//补最后一包这里，只要读取到逆变器返回的最后一包时间，就立刻删除在表格中的这个时间点，以免单台逆变器补包的时候重复

		memcpy(&record_time,localtime(&tm), sizeof(record_time));

		sprintf(receivetime, "%04d%02d%02d%02d%02d%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
				record_time.tm_hour, record_time.tm_min, record_time.tm_sec);
		print2msg("Receive time", receivetime);



		if(readbuff[22]>0)
		{
			for(i=0, j=34; i<26; i++, j++)
			{
				data[i] = readbuff[j];
			}
			return ret;
		}
		else
		{
			return -1;
		}

	}
	else if((60==ret)&&(0xFB == readbuff[0])&&(0xFB == readbuff[1])&&(0x01 == readbuff[2])&&
						(sendbuff[11] == readbuff[11])&&(sendbuff[12] == readbuff[12])&&
						(sendbuff[13] == readbuff[13])&&(sendbuff[14] == readbuff[14])&&
						(sendbuff[15] == readbuff[15])&&(sendbuff[16] == readbuff[16])&&
						(0xFE == readbuff[58])&&(0xFE == readbuff[59]))
	{		//YC250
		memset(receivetime, '\0', sizeof(receivetime));
		tm = readbuff[30]*16777216+readbuff[31]*65536+readbuff[32]*256+readbuff[33];

		nearlist_time = get_nearest_time(inverterid, tm);
		if(-1 != nearlist_time)
			tm = nearlist_time;

		clear_single_time(tm,inverterid);	//补最后一包这里，只要读取到逆变器返回的最后一包时间，就立刻删除在表格中的这个时间点，以免单台逆变器补包的时候重复
		memcpy(&record_time,localtime(&tm), sizeof(record_time));

		sprintf(receivetime, "%04d%02d%02d%02d%02d%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
				record_time.tm_hour, record_time.tm_min, record_time.tm_sec);
		print2msg("Receive time", receivetime);

		if(readbuff[22]>0)
		{
			for(i=0, j=34; i<22; i++, j++)
			{
				data[i] = readbuff[j];
			}
			return ret;
		}
		else
		{
			return -1;
		}

	}
	else
	{
		return -1;
	}
}

//int resolve_lost_data_single_yc250(char *inverter_data,char *inverterid, char *receivetime,int last_data_mark)		//解析补发的数据
//{
//	char sql[1024]={'\0'};
//	unsigned char temp[2] = {'\0'};
//	int i, seconds;
//	char data_EMA_a[256]= {'\0'};
//	char data_EMA_b[256]= {'\0'};
//	char *buff;
//
//	float dv;			//直流电压
//	float di;			//直流电流
//	int op;			//输出功率
//	float np;			//电网频率
//	int nv;			//电网电压
//	int it;			//机内温度
//
//
//	float curgeneration;		//逆变器当前一轮的电量
//
//	char status_web[24]={'\0'};		//存入ECU本地数据库的状态，用于本地页面显示
//	char status[12]={'\0'};		//逆变器状态
//
//
//	sqlite3 *db=NULL;
//
//	inverter->dv=(inverter_data[0]*256+inverter_data[1])*825/4096;
//	inverter->di=(inverter_data[2]*256+inverter_data[3])*275/4096;
//	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;
//
//	if(2 == caltype)
//		inverter->nv=(inverter_data[6]*256+inverter_data[7]) / 2.93;
//	else if(1 == caltype)
//		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
//	else
//		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
//	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;
//
//
//
//	{
//		for(i=0; i<6; i++){
//			data[i] = inverter_data[12+i];
//		}
//
//
//		inverter->curacctime = inverter_data[18]*256 + inverter_data[19];		//最近两次上报的累计时间的差值，即两次上报的时间间隔
//		inverter->curaccgen = (float)(((inverter_data[13]*256 + inverter_data[14])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[15]*256.0*256.0 + inverter_data[16]*256.0 + inverter_data[17])/1661900.0*220.0/1000.0)/3600.0);
//
//		if((inverter->curaccgen > inverter->preaccgen)&&(inverter->curacctime > inverter->preacctime)){
//			seconds = inverter->curacctime - inverter->preacctime;
//			inverter->duration = seconds;
//			inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
//		}
//		else{
//			seconds = inverter->curacctime;
//			inverter->curgeneration = inverter->curaccgen;
//		}
//
//		inverter->preacctime = inverter->curacctime;
//		inverter->preaccgen = inverter->curaccgen;
//
//		if(inverter->curacctime > 600){
//			inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
//		}
//		else{
//			inverter->op = (int)(inverter->dv*inverter->di/100.0);
//		}
//
//
//		if(inverter->op>260)
//			inverter->op = (int)(inverter->dv*inverter->di/100.0);
//		/*if(1 == caltype)
//			inverter->op = (int)(inverter->dv*inverter->di*0.95/100.0);
//		else
//			inverter->op = (int)(inverter->dv*inverter->di*0.99/100.0);*/
//	}
//
//
//
//
//	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
//	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
//	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
//	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
//	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
//	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
//	inverter->status_web[6]='0';
//	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
//	inverter->status_web[8]='0';
//	inverter->status_web[9]='0';
//	inverter->status_web[10]='0';
//	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
//	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
//	inverter->status_web[13]=((inverter_data[10]>>1)&0x01)+0x30;		//Active anti-island protection
//	inverter->status_web[14]=((inverter_data[11]>>7)&0x01)+0x30;		//CP protection
//	inverter->status_web[15]=((inverter_data[11]>>1)&0x01)+0x30;		//HV protection
//	inverter->status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection
//	inverter->status_web[17]='0';										//GFDI-B
//	inverter->status_web[18]='0';//((inverter_data[11]>>2)&0x01)+0x30;		//Epprom_done
//
//	inverter->status[0]=inverter->status_web[0];
//	inverter->status[1]=inverter->status_web[1];
//	inverter->status[2]=inverter->status_web[2];
//	inverter->status[3]=inverter->status_web[3];
//	inverter->status[4]=inverter->status_web[4];
//	inverter->status[5]=inverter->status_web[5];
//	inverter->status[6]=inverter->status_web[7];
//	inverter->status[7]=inverter->status_web[11];
//	inverter->status[8]=inverter->status_web[12];
//	inverter->status[9]='0';
//	inverter->status[10]='0';
//
//	yc200_status(inverter);
//
//	inverter->lastreporttime[0] = inverter_data[20];
//	inverter->lastreporttime[1] = inverter_data[21];
//
//	if(inverter->last_gfdi_flag != inverter->status_web[11])
//		inverter->gfdi_changed_flag = 1;
//	else
//		inverter->gfdi_changed_flag = 0;
//	inverter->last_gfdi_flag = inverter->status_web[11];
//	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
//		inverter->turn_on_off_changed_flag = 1;
//	else
//		inverter->turn_on_off_changed_flag = 0;
//	inverter->last_turn_on_off_flag = inverter->status_web[12];
//
//	check_yc200_yc250(inverter);
//
//	return 1;
//}

int resolve_lost_data_single_yc250(char *inverter_data,char *inverterid, char *receivetime,int last_data_mark)		//解析补发的数据
{
	char sql[1024]={'\0'};
	unsigned char temp[2] = {'\0'};
	int i, seconds;
	char data_EMA[256]= {'\0'};
	char *buff;
	char status_ema[64] = {'\0'};

	float dv;			//直流电压
	float di;			//直流电流
	int op;			//输出功率
	float np;			//电网频率
	int nv;			//电网电压
	int it;			//机内温度

	float curgeneration;		//逆变器当前一轮的电量

	char status_web[24]={'\0'};		//存入ECU本地数据库的状态，用于本地页面显示
	char status[12]={'\0'};		//逆变器状态

	sqlite3 *db=NULL;

	if(1 == check_exist(inverterid, receivetime))
		return 0;

	memset(data_EMA, '\0', sizeof(data_EMA));

	dv = (inverter_data[0]*256 + inverter_data[1])*825/4096;
	di = (inverter_data[2]*256 + inverter_data[3])*275/4096;
	np = (inverter_data[4]*256 + inverter_data[5])/10.0;
	if(1 == caltype)
		nv = (inverter_data[6]*256 + inverter_data[7])*352/1024;
	else
		nv = (inverter_data[6]*256 + inverter_data[7])*687/1024;
	it = ((inverter_data[8]*256 + inverter_data[9])*330-245760)/4096;

	seconds = inverter_data[22]*256 + inverter_data[23];

	if(1==last_data_mark)     //当为补最后一包数据的时候，电量需要解析，否则则置为0
	{
		curgeneration = (float)(((inverter_data[13]*256 + inverter_data[14])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[15]*256.0*256.0 + inverter_data[16]*256.0 + inverter_data[17])/1661900.0*220.0/1000.0)/3600.0);
	}
	else
	{
		curgeneration = 0;
	}
	printf("curgeneration=%f\n",curgeneration);

	op = (int)(dv*di/100.0);

	status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	status_web[6]='0';
	status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	status_web[8]='0';
	status_web[9]='0';
	status_web[10]='0';
	status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	status_web[13]=((inverter_data[10]>>1)&0x01)+0x30;		//Active anti-island protection
	status_web[14]=((inverter_data[11]>>7)&0x01)+0x30;		//CP protection
	status_web[15]=((inverter_data[11]>>1)&0x01)+0x30;		//HV protection
	status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection
	status_web[17]='0';										//GFDI-B
	status_web[18]='0';										//Epprom_done

	status[0]=status_web[0];
	status[1]=status_web[1];
	status[2]=status_web[2];
	status[3]=status_web[3];
	status[4]=status_web[4];
	status[5]=status_web[5];
	status[6]=status_web[7];
	status[7]=status_web[11];
	status[8]=status_web[12];
	status[9]='0';
	status[10]='0';

	if(('1' == status_web[1]) || ('1' == status_web[0]) || ('1' == status_web[2]) ||
				('1' == status_web[3]) || ('1' == status_web[15]) || ('1' == status_web[14]) ||
				('1' == status_web[13]) || ('1' == status_web[16]) || ('1' == status_web[7]) ||
				('1' == status_web[11]) || ('1' == status_web[12])){
		strcat(status_ema, inverterid);
		strcat(status_ema, "01");
		if(('1' == status_web[1]) || ('1' == status_web[0]) || ('1' == status_web[2]) ||
				('1' == status_web[3]) || ('1' == status_web[15]) || ('1' == status_web[14]) ||
				('1' == status_web[13]) || ('1' == status_web[16]))
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if('1' == status_web[7])
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if('1' == status_web[11])
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if('1' == status_web[12])
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");

		status_ema[18] = status_web[1];
		status_ema[19] = status_web[0];
		status_ema[20] = status_web[2];
		status_ema[21] = status_web[3];
		status_ema[22] = status_web[16];
		status_ema[23] = status_web[15];
		status_ema[24] = status_web[14];
		status_ema[25] = status_web[13];
		status_ema[26] = status_web[7];
		status_ema[27] = status_web[11];
		status_ema[28] = status_web[12];
		status_ema[29] = '0';//inverter->status_web[18];
		strcat(status_ema, "END");

		save_status_single(inverterid, status_ema, receivetime);
	}

	sprintf(data_EMA, "%s0%05d%03d%05d%05d%03d%03d%010d", inverterid, (int)(dv), (int)(di), op*100, (int)(np*10), it+100, nv, (int)(curgeneration*1000000));
	print2msg("data_EMA_a",data_EMA);

	save_single_lost_data(data_EMA,receivetime);

	if(curgeneration>0)
	{
		ltgeneration = ltgeneration + curgeneration;
		update_lifetime_energy(ltgeneration);
		print2msg("receivetime",receivetime);
		printmsg("debug1");
		update_daily_energy(curgeneration,receivetime);
		printmsg("debug2");
		update_monthly_energy(curgeneration,receivetime);
		printmsg("debug3");
		update_yearly_energy(curgeneration,receivetime);
		printmsg("debug4");
	}

	if(op>0)
	{
		save_system_power_single(op,receivetime);
		printmsg("debug5");
	}

	return 1;
}

int resolve_lost_data_single_yc500(char *inverter_data,char *inverterid, char *receivetime,int last_data_mark)		//解析补发的数据
{

	char sql[1024]={'\0'};
	unsigned char temp[2] = {'\0'};
	int i, seconds;
	char data_EMA_a[256]= {'\0'};
	char data_EMA_b[256]= {'\0'};
	char *buff;
	char status_ema[64] = {'\0'};

	float dv;			//直流电压
	float di;			//直流电流
	int op;			//输出功率
	float np;			//电网频率
	int nv;			//电网电压
	int it;			//机内温度

//	int flagyc500;			//1，为YC500

	/********B路数据***********/
	float dvb;			//直流电压
	float dib;			//直流电流
	int opb;			//输出功率
	//float npb;			//电网频率
	//int nvb;			//电网电压
	//int itb;			//机内温度

	float curgeneration;		//逆变器当前一轮的电量
	float curgenerationb;		//B路当前一轮的电量

	char status_web[24]={'\0'};		//存入ECU本地数据库的状态，用于本地页面显示
	char status[12]={'\0'};		//逆变器状态
	char statusb[12]={'\0'};		//B路状态

	sqlite3 *db=NULL;

	if(1 == check_exist(inverterid, receivetime))
		return 0;

	memset(data_EMA_a, '\0', sizeof(data_EMA_a));
	memset(data_EMA_b, '\0', sizeof(data_EMA_b));

	dv = (((inverter_data[0] >> 4) & 0x0F) * 256 + ((inverter_data[0] << 4) & 0xF0) | ((inverter_data[1] >> 4) & 0x0F)) * 825 / 4096;
	di = ((inverter_data[1] & 0x0F) * 256 + inverter_data[2]) * 275 / 4096;
	dvb = (((inverter_data[3] >> 4) & 0x0F) * 256 + ((inverter_data[3] << 4) & 0xF0) | ((inverter_data[4] >> 4) & 0x0F)) * 825 / 4096;
	dib = ((inverter_data[4] & 0x0F) * 256 + inverter_data[5]) * 275 / 4096;
	np = (((inverter_data[6] >> 6) & 0x03) * 256 + (((inverter_data[6] << 2) & 0xFC) | ((inverter_data[7] >> 6) & 0x03))) / 10.0;
	if(1 == caltype)
		nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 352 / 1024;
	else
		nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 687 / 1024;
	it = (((inverter_data[8] & 0x0F) * 256 + inverter_data[9]) * 330 - 245760) / 4096;


	seconds = inverter_data[22]*256 + inverter_data[23];

//	op = (int)(((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/seconds);
//	opb = (int)(((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/seconds);

	if(1==last_data_mark)     //当为补最后一包数据的时候，电量需要解析，否则则置为0
	{
		curgeneration = (float)((((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/1000.0)/3600.0);
		curgenerationb = (float)((((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/1000.0)/3600.0);
	}
	else
	{
		curgeneration = 0;
		curgenerationb = 0;
	}
	printf("curgeneration=%f\n",curgeneration);
	printf("curgenerationb=%f\n",curgenerationb);

//	curgeneration = inverter->curgeneration * 0.91;
//	curgenerationb = inverter->curgenerationb * 0.91;



//	if(op>260)
		op = (int)(dv*di/100.0);
//	if(opb>260)
		opb = (int)(dvb*dib/100.0);


	status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	status_web[4]='0';
	status_web[5]='0';
	status_web[6]=((inverter_data[10]>>1)&0x01)+0x30;		//DC-A Voltage Too High 1bit
	status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	status_web[8]=((inverter_data[11]>>7)&0x01)+0x30;		//DC-A Voltage Too Low 1bit
	status_web[9]=((inverter_data[11]>>6)&0x01)+0x30;		//DC-B Voltage Too High 1bit
	status_web[10]=((inverter_data[11]>>5)&0x01)+0x30;		//DC-B Voltage Too Low 1bit
	status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	status_web[13]='0';
	status_web[14]='0';
	status_web[15]='0';

	status[0]=status_web[0];
	status[1]=status_web[1];
	status[2]=status_web[2];
	status[3]=status_web[3];
	status[4]=status_web[6];
	status[5]=status_web[8];
	status[6]=status_web[7];
	status[7]=status_web[11];
	status[8]=status_web[12];
	status[9]='0';
	status[10]='0';

	statusb[0]=status_web[0];
	statusb[1]=status_web[1];
	statusb[2]=status_web[2];
	statusb[3]=status_web[3];
	statusb[4]=status_web[9];
	statusb[5]=status_web[10];
	statusb[6]=status_web[7];
	statusb[7]=status_web[11];
	statusb[8]=status_web[12];
	statusb[9]='0';
	statusb[10]='0';

	if(('1' == status_web[1]) || ('1' == status_web[0]) || ('1' == status_web[2]) ||
			('1' == status_web[3]) || ('1' == status_web[15]) || ('1' == status_web[14]) ||
			('1' == status_web[13]) || ('1' == status_web[7]) || ('1' == status_web[12]) ||
			('1' == status_web[11]) || ('1' == status_web[17])){
		strcat(status_ema, inverterid);
		strcat(status_ema, "02");
		if(('1' == status_web[1]) || ('1' == status_web[0]) || ('1' == status_web[2]) ||
				('1' == status_web[3]) || ('1' == status_web[15]) || ('1' == status_web[14]) ||
				('1' == status_web[13]))
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if('1' == status_web[7])
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if(('1' == status_web[11]) || ('1' == status_web[17]))
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if('1' == status_web[12])
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");

		status_ema[18] = status_web[1];
		status_ema[19] = status_web[0];
		status_ema[20] = status_web[2];
		status_ema[21] = status_web[3];
		status_ema[22] = status_web[16];
		status_ema[23] = status_web[14];
		status_ema[24] = status_web[13];
		status_ema[25] = status_web[7];
		status_ema[26] = status_web[12];
		status_ema[27] = '0';//;inverter->status_web[18];
		status_ema[28] = 'A';
		status_ema[29] = status_web[11];
		status_ema[30] = 'B';
		status_ema[31] = status_web[17];
		strcat(status_ema, "END");

		save_status_single(inverterid, status_ema, receivetime);
	}


	sprintf(data_EMA_a, "%sA%05d%03d%05d%05d%03d%03d%010d", inverterid, (int)(dv), (int)(di), op*100, (int)(np*10), it+100, nv, (int)(curgeneration*1000000));
	print2msg("data_EMA_a",data_EMA_a);
	sprintf(data_EMA_b, "%sB%05d%03d%05d%05d%03d%03d%010d", inverterid, (int)(dvb), (int)(dib), opb*100, (int)(np*10), it+100, nv, (int)(curgenerationb*1000000));
	print2msg("data_EMA_b",data_EMA_b);

	save_single_lost_data(data_EMA_a,receivetime);
	save_single_lost_data(data_EMA_b,receivetime);


	if((curgeneration>0)||(curgenerationb>0))
	{
		ltgeneration = ltgeneration + curgeneration + curgenerationb;
		update_lifetime_energy(ltgeneration);
		print2msg("receivetime",receivetime);
		printmsg("debug1");
		update_daily_energy(curgeneration + curgenerationb,receivetime);
		printmsg("debug2");
		update_monthly_energy(curgeneration + curgenerationb,receivetime);
		printmsg("debug3");
		update_yearly_energy(curgeneration + curgenerationb,receivetime);
		printmsg("debug4");
	}

	if((op>0)||(opb>0))
	{
		save_system_power_single(op+opb,receivetime);
		printmsg("debug5");
	}
}

int save_single_lost_data(char *data_EMA , char *receivetime)	//保存单个时间单台逆变器丢失的数据到数据库
{
	char sql[1024]={'\0'};
	sqlite3 *db;
	char *zErrMsg=0;

	sqlite3_open("/home/record.db", &db);

	strcpy(sql,"CREATE TABLE IF NOT EXISTS fill_up_data(item INTEGER PRIMARY KEY AUTOINCREMENT, data VARCHAR, lost_date_time VARCHAR, send_date_time VARCHAR, send_flag INTEGER)");
	sqlite3_exec_3times(db, sql);


	memset(sql,'\0',1024);
	sprintf(sql,"INSERT INTO fill_up_data (data, lost_date_time, send_flag) VALUES('%s', '%s',1);",data_EMA , receivetime);
	sqlite3_exec_3times(db, sql);
	sqlite3_close(db);
}

int reslove_save_all_lost_data_250(char *inverter_data,char *inverterid, char *receivetime)	//解析并保存单个时间所有逆变器丢失的数据到数据库
{
	unsigned char temp[10] = {'\0'};
	int i, seconds;
	char data_EMA[256]= {'\0'};
	char *buff;
	char status_ema[64] = {'\0'};

	float dv;			//直流电压
	float di;			//直流电流
	int op;			//输出功率
	float np;			//电网频率
	int nv;			//电网电压
	int it;			//机内温度

	float curgeneration;		//逆变器当前一轮的电量

	char status_web[24]={'\0'};		//存入ECU本地数据库的状态，用于本地页面显示
	char status[12]={'\0'};		//逆变器状态

	dv = (inverter_data[0]*256 + inverter_data[1])*825/4096;
	di = (inverter_data[2]*256 + inverter_data[3])*275/4096;
	np = (inverter_data[4]*256 + inverter_data[5])/10.0;
	if(1 == caltype)
		nv = (inverter_data[6]*256 + inverter_data[7])*352/1024;
	else
		nv = (inverter_data[6]*256 + inverter_data[7])*687/1024;
	it = ((inverter_data[8]*256 + inverter_data[9])*330-245760)/4096;

	seconds = inverter_data[22]*256 + inverter_data[23];

	curgeneration = curgeneration = (float)(((inverter_data[13]*256 + inverter_data[14])/1661900.0*220.0/1000.0*256.0*256.0*256.0 +
			(inverter_data[15]*256.0*256.0 + inverter_data[16]*256.0 + inverter_data[17])/1661900.0*220.0/1000.0)/3600.0);

	op = (int)(dv*di/100.0);

	status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	status_web[6]='0';
	status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	status_web[8]='0';
	status_web[9]='0';
	status_web[10]='0';
	status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	status_web[13]=((inverter_data[10]>>1)&0x01)+0x30;		//Active anti-island protection
	status_web[14]=((inverter_data[11]>>7)&0x01)+0x30;		//CP protection
	status_web[15]=((inverter_data[11]>>1)&0x01)+0x30;		//HV protection
	status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection
	status_web[17]='0';										//GFDI-B
	status_web[18]='0';										//Epprom_done

	status[0]=status_web[0];
	status[1]=status_web[1];
	status[2]=status_web[2];
	status[3]=status_web[3];
	status[4]=status_web[4];
	status[5]=status_web[5];
	status[6]=status_web[7];
	status[7]=status_web[11];
	status[8]=status_web[12];
	status[9]='0';
	status[10]='0';

	if(('1' == status_web[1]) || ('1' == status_web[0]) || ('1' == status_web[2]) ||
				('1' == status_web[3]) || ('1' == status_web[15]) || ('1' == status_web[14]) ||
				('1' == status_web[13]) || ('1' == status_web[16]) || ('1' == status_web[7]) ||
				('1' == status_web[11]) || ('1' == status_web[12])){
		strcat(status_ema, inverterid);
		strcat(status_ema, "01");
		if(('1' == status_web[1]) || ('1' == status_web[0]) || ('1' == status_web[2]) ||
				('1' == status_web[3]) || ('1' == status_web[15]) || ('1' == status_web[14]) ||
				('1' == status_web[13]) || ('1' == status_web[16]))
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if('1' == status_web[7])
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if('1' == status_web[11])
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");
		if('1' == status_web[12])
			strcat(status_ema, "1");
		else
			strcat(status_ema, "0");

		status_ema[18] = status_web[1];
		status_ema[19] = status_web[0];
		status_ema[20] = status_web[2];
		status_ema[21] = status_web[3];
		status_ema[22] = status_web[16];
		status_ema[23] = status_web[15];
		status_ema[24] = status_web[14];
		status_ema[25] = status_web[13];
		status_ema[26] = status_web[7];
		status_ema[27] = status_web[11];
		status_ema[28] = status_web[12];
		status_ema[29] = '0';//inverter->status_web[18];
		strcat(status_ema, "END");

		save_status_single(inverterid, status_ema, receivetime);
	}

	sprintf(data_EMA, "%s0%05d%03d%05d%05d%03d%03d%010d", inverterid, (int)(dv), (int)(di), op*100, (int)(np*10), it+100, nv, (int)(curgeneration*1000000));
	print2msg("data_EMA_a",data_EMA);

	strcat(lost_data_info.sendbuff, data_EMA);

	print2msg("lost_data_info.sendbuff", lost_data_info.sendbuff);
	lost_data_info.syspower = lost_data_info.syspower + op;
	lost_data_info.curgeneration = lost_data_info.curgeneration + curgeneration;
	lost_data_info.inverter_count++;			//获取到数据的逆变器个数加1

	return 1;
}

int reslove_save_all_lost_data_500(char *inverter_data,char *inverterid, char *receivetime)	//解析并保存单个时间所有逆变器丢失的数据到数据库
{
	sqlite3 *db;
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	char sql[1024]={'\0'};

	unsigned char temp[10] = {'\0'};
	int i, seconds;
	char data_EMA_a[256]= {'\0'};
	char data_EMA_b[256]= {'\0'};
	char *buff;

	float dv;			//直流电压
	float di;			//直流电流
	int op;			//输出功率
	float np;			//电网频率
	int nv;			//电网电压
	int it;			//机内温度

//	int flagyc500;			//1，为YC500

	/********B路数据***********/
	float dvb;			//直流电压
	float dib;			//直流电流
	int opb;			//输出功率
	//float npb;			//电网频率
	//int nvb;			//电网电压
	//int itb;			//机内温度

	float curgeneration;		//逆变器当前一轮的电量
	float curgenerationb;		//B路当前一轮的电量

	char status_web[24]={'\0'};		//存入ECU本地数据库的状态，用于本地页面显示
	char status[12]={'\0'};		//逆变器状态
	char statusb[12]={'\0'};		//B路状态

//	struct replacement_sendEMA_data_info *sendEMA_data;


	dv = (((inverter_data[0] >> 4) & 0x0F) * 256 + ((inverter_data[0] << 4) & 0xF0) | ((inverter_data[1] >> 4) & 0x0F)) * 825 / 4096;
	di = ((inverter_data[1] & 0x0F) * 256 + inverter_data[2]) * 275 / 4096;
	dvb = (((inverter_data[3] >> 4) & 0x0F) * 256 + ((inverter_data[3] << 4) & 0xF0) | ((inverter_data[4] >> 4) & 0x0F)) * 825 / 4096;
	dib = ((inverter_data[4] & 0x0F) * 256 + inverter_data[5]) * 275 / 4096;
	np = (((inverter_data[6] >> 6) & 0x03) * 256 + (((inverter_data[6] << 2) & 0xFC) | ((inverter_data[7] >> 6) & 0x03))) / 10.0;
	if(1 == caltype)
		nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 352 / 1024;
	else
		nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 687 / 1024;
	it = (((inverter_data[8] & 0x0F) * 256 + inverter_data[9]) * 330 - 245760) / 4096;


	seconds = inverter_data[22]*256 + inverter_data[23];

//	op = (int)(((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/seconds);
//	opb = (int)(((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/seconds);


	curgeneration = (float)((((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/1000.0)/3600.0);
	curgenerationb = (float)((((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/1000.0)/3600.0);




//	if(op>260)
		op = (int)(dv*di/100.0);
//	if(opb>260)
		opb = (int)(dvb*dib/100.0);


	status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	status_web[4]='0';
	status_web[5]='0';
	status_web[6]=((inverter_data[10]>>1)&0x01)+0x30;		//DC-A Voltage Too High 1bit
	status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	status_web[8]=((inverter_data[11]>>7)&0x01)+0x30;		//DC-A Voltage Too Low 1bit
	status_web[9]=((inverter_data[11]>>6)&0x01)+0x30;		//DC-B Voltage Too High 1bit
	status_web[10]=((inverter_data[11]>>5)&0x01)+0x30;		//DC-B Voltage Too Low 1bit
	status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	status_web[13]='0';
	status_web[14]='0';
	status_web[15]='0';

	status[0]=status_web[0];
	status[1]=status_web[1];
	status[2]=status_web[2];
	status[3]=status_web[3];
	status[4]=status_web[6];
	status[5]=status_web[8];
	status[6]=status_web[7];
	status[7]=status_web[11];
	status[8]=status_web[12];
	status[9]='0';
	status[10]='0';

	statusb[0]=status_web[0];
	statusb[1]=status_web[1];
	statusb[2]=status_web[2];
	statusb[3]=status_web[3];
	statusb[4]=status_web[9];
	statusb[5]=status_web[10];
	statusb[6]=status_web[7];
	statusb[7]=status_web[11];
	statusb[8]=status_web[12];
	statusb[9]='0';
	statusb[10]='0';


	sprintf(data_EMA_a, "%sA%05d%03d%05d%05d%03d%03d%010dEND", inverterid, (int)(dv), (int)(di), op*100, (int)(np*10), it+100, nv, (int)(curgeneration*1000000));
	print2msg("data_EMA_a",data_EMA_a);
	sprintf(data_EMA_b, "%sB%05d%03d%05d%05d%03d%03d%010dEND", inverterid, (int)(dvb), (int)(dib), opb*100, (int)(np*10), it+100, nv, (int)(curgenerationb*1000000));
	print2msg("data_EMA_b",data_EMA_b);

	strcat(lost_data_info.sendbuff, data_EMA_a);
	strcat(lost_data_info.sendbuff, data_EMA_b);

	print2msg("lost_data_info.sendbuff",lost_data_info.sendbuff);
	lost_data_info.syspower=lost_data_info.syspower+op+opb;
	lost_data_info.curgeneration=lost_data_info.curgeneration+curgeneration+curgenerationb;
	lost_data_info.inverter_count++;			//获取到数据的逆变器个数加1

	/*
	if(1==mark_lastinverter)				//mark_lastinverter等于1时，说明已经是结构体最后一台逆变器，ZK
	{
		sprintf(lost_data_info.sendbuff_all, "APS150000000010001%s%010d%010d%010d%s%03d000000END", ecuid, lost_data_info.syspower, (int)(lost_data_info.curgeneration*1000000), (int)(lost_data_info.ltgeneration*10), receivetime, lost_data_info.inverter_count);
		strcat(lost_data_info.sendbuff_all, lost_data_info.sendbuff);
		sprintf(temp,"%05d",strlen(lost_data_info.sendbuff_all));
		for(i=5; i<10; i++)
			lost_data_info.sendbuff_all[i] = temp[i-5];
		strcat(lost_data_info.sendbuff_all, "\n");

		save_record(lost_data_info.sendbuff_all, receivetime);
		printmsg(lost_data_info.sendbuff_all);
//		memset(replacement_sendEMA_buff,'\0',sizeof(replacement_sendEMA_buff));
//		memset(replacement_sendEMA_buff2,'\0',sizeof(replacement_sendEMA_buff2));
//		replacement_sendEMA_syspower=0;
//		replacemlost_data_infoent_sendEMA_curgeneration=0;
//		replacement_sendEMA_count_2=0;
		init_lost_data_info();

	}
*/
	return 1;


}

int protocol_single_lost_data()	//拼接单个时间单台逆变器丢失的数据
{
	;
}

int protocol_all_lost_data(char *receivetime, int all_time_linux)	//拼接单个时间所有逆变器丢失的数据
{
	unsigned char temp[10] = {'\0'};
	int i;



	if(lost_data_info.inverter_count>0)				//当至少有一台逆变器获取到数据时候，才进行头拼接并发送到record表,并且清除lost_all_time表中这个时间点
	{

		sprintf(lost_data_info.sendbuff_all, "APS150000000010001%s%010d%010d%010d%s%03d", ecuid, lost_data_info.syspower, (int)(lost_data_info.curgeneration*1000000.0), (int)(lost_data_info.ltgeneration*10.0), receivetime, lost_data_info.inverter_count);

		if(!zoneflag)
			strcat(lost_data_info.sendbuff_all, "000000END");
		else
			strcat(lost_data_info.sendbuff_all, "100000END");

		strcat(lost_data_info.sendbuff_all, lost_data_info.sendbuff);
		sprintf(temp,"%05d",strlen(lost_data_info.sendbuff_all));
		for(i=5; i<10; i++)
			lost_data_info.sendbuff_all[i] = temp[i-5];
		strcat(lost_data_info.sendbuff_all, "\n");

		save_record(lost_data_info.sendbuff_all, receivetime);
		printmsg(lost_data_info.sendbuff_all);
	//	clear_all_time(all_time_linux);

	}

}



int fill_up_all_data(struct inverter_info_t *firstinverter,int rest_time)		//补单个时间所有逆变器丢失的数据
{
	struct inverter_info_t *curinverter = firstinverter;
	int i,all_time_linux=0;
	char data[512] = {'\0'};
	char receivetime[20]={'\0'};					//发送给EMA时的日期和时间，格式：年月日时分秒
	int flag=0;

	while(1)
	{
		all_time_linux = get_all_time();
		if(-1==all_time_linux)
			return-1;
		printdecmsg("all_time_linux",all_time_linux);
		printdecmsg("lost_data_info.current_inverter",lost_data_info.current_inverter);
		printdecmsg("displaymaxcount",displaymaxcount);
		for(i=lost_data_info.current_inverter; (i<displaymaxcount); i++, lost_data_info.current_inverter++,curinverter++)
		{
			memset(data, '\0', sizeof(data));
			curinverter = firstinverter+lost_data_info.current_inverter;
		//	if((rest_time-time(NULL)<25)||(fill_up_times>4))
			if(rest_time-time(NULL)<25)
				return-1;

			if(1 == curinverter->fill_up_data_flag)				//判断此台逆变器有补数据功能才进行补数据操作
			{
				flag = query_lost_data(curinverter->inverterid , all_time_linux, data,receivetime);
				if(64 == flag)
					reslove_save_all_lost_data_500(data,curinverter->inverterid, receivetime);
				if(60 == flag)
					reslove_save_all_lost_data_250(data,curinverter->inverterid, receivetime);
			}

			if(i==displaymaxcount-1)			//当全部逆变器都轮询一次并且至少有一台逆变器获取到数据才进入拼接
			{
				clear_all_time(all_time_linux);
				if(lost_data_info.inverter_count>0)
				{
					save_system_power(lost_data_info.syspower,receivetime);
				}
				protocol_all_lost_data(receivetime, all_time_linux);
				init_lost_data_info();
			//	fill_up_times++;
				break;
			}
		}
	}
	return 1;
}

int fill_up_single_data(struct inverter_info_t *firstinverter,int rest_time)		//补单个时间单个逆变器丢失的数据
{
	char inverterid[13]={'\0'};
	int time_linux=0;
	int i,result=0;
	char data[512] = {'\0'};
	char receivetime[20]={'\0'};					//发送给EMA时的日期和时间，格式：年月日时分秒
	struct inverter_info_t *curinverter = firstinverter+lost_data_info.num;
	int fill_up_times=0;				//补包总轮询次数,超过4次，则在本轮询不再补包

	if(-1==check_single_time())
	{
		return-1;
	}

	while(1)
	{
		for(i=lost_data_info.num; (i<displaymaxcount); i++, lost_data_info.num++)
		{
			if(-1==check_single_time())
				return-1;

			curinverter = firstinverter+lost_data_info.num;
			print2msg("curinverter->inverterid",curinverter->inverterid);
			printdecmsg("lost_data_info.num",lost_data_info.num);
			printdecmsg("displaymaxcount",displaymaxcount);
			while((time_linux=(get_single_time(curinverter->inverterid)))>0)
			{
				printdecmsg("rest_time",rest_time);
				memset(data, '\0', sizeof(data));
				printdecmsg("TIME_LINUX",time_linux);
				if((rest_time-time(NULL)<25)||(fill_up_times>4))
					return-1;
				result = query_lost_data(curinverter->inverterid , time_linux, data,receivetime);

				if(-1==result)				//-1时直接跳下一台逆变器
					break;
				if(64==result)				//1时解析获取到的数据 YC500
					resolve_lost_data_single_yc500(data,curinverter->inverterid, receivetime,0);
				if(60==result)				//2时解析获取到的数据 YC250
					resolve_lost_data_single_yc250(data,curinverter->inverterid, receivetime,0);

			}
			sleep(1);
			if(i==(displaymaxcount-1))
			{
				init_lost_data_info();
				fill_up_times++;
				if(fill_up_times>4)
					return -1;
				break;
			}
		}
	}

}


int transform_first_time(int thistime)
{
	time_t tm;
	struct tm record_time;    //记录时间
	int first_boot_time;

	tm=thistime;
	memcpy(&record_time,localtime(&tm), sizeof(record_time));
	first_boot_time = thistime-(record_time.tm_hour*3600 + record_time.tm_min*60 + record_time.tm_sec);
	printdecmsg("first_boot_time",first_boot_time);
	return first_boot_time;
}

int fill_up_last_data(struct inverter_info_t *firstinverter,int rest_time,int thistime)		//补单个时间所有逆变器丢失的数据
{
	int i,all_time_linux=0;
	char data[512] = {'\0'};
	char receivetime[20]={'\0'};					//发送给EMA时的日期和时间，格式：年月日时分秒
	int first_boot_time;
	struct inverter_info_t *curinverter = firstinverter+lost_data_info.current_inverter;
	int mark=0;

	if(0==first_boot_mark)
	{
		first_boot_time = transform_first_time(thistime);
		for(i=lost_data_info.current_inverter; (i<displaymaxcount); i++, lost_data_info.current_inverter++,curinverter++)
		{
			memset(data, '\0', sizeof(data));
			if(rest_time-time(NULL)<25)
				return-1;

			mark = query_last_lost_data(curinverter->inverterid , first_boot_time, data,receivetime);
			if(64 == mark)
				resolve_lost_data_single_yc500(data,curinverter->inverterid, receivetime,1);
			if(60 == mark)
				resolve_lost_data_single_yc250(data,curinverter->inverterid, receivetime,1);

			if(i==displaymaxcount-1)
			{
				init_lost_data_info();
				first_boot_mark=1;			//当最后一台逆变器补最后一包结束后，标志位置为1
				break;
			}
		}
	}
	return 1;
}

//逆变器的ID突变，或者更换逆变器后，数据库中存在以前的数据，需要清除
int find_changed_ids(struct inverter_info_t *firstinverter)
{
	sqlite3 *db;
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0,ncolumn;
	int i,j, k, exist;
	struct inverter_info_t *curinverter;

    sqlite3_open("/tmp/replacement.db", &db);
	strcpy(sql, "SELECT DISTINCT inverterid FROM save_replacement_time ");

	for(i=0; i<3; i++)
	{
		if(SQLITE_OK == sqlite3_get_table(db , sql , &azResult , &nrow , &ncolumn , &zErrMsg))
		{
			for(k=1; k<=nrow; k++)
			{
				for(j=0, exist=0, curinverter=firstinverter; (j<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); j++, curinverter++)
				{
					if(!strcmp(curinverter->inverterid, azResult[k]))
						exist = 1;
				}
				if(0 == exist)
					clear_single_alltime(azResult[k]);
			}

			sqlite3_free_table(azResult);

			break;
		}
		else
			sqlite3_free_table(azResult);
	}

	sqlite3_close(db);

	return 0;
}

int fill_up_data(struct inverter_info_t *firstinverter,int rest_time,int thistime)				//进入不数据流程,rest_time为当前轮询空闲时间，大于15秒才进行补数据功能，否则就直接退出，ZK
{
	struct inverter_info_t *curinverter = firstinverter;
	int temp_time, i;

	if(0 == turn_on_flag)
		return time(NULL);

	find_changed_ids(firstinverter);

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
	{
		if(1 != curinverter->fill_up_data_flag)
			return time(NULL);
	}

	curinverter = firstinverter;
	if(rest_time-time(NULL)>25)
	{
//		fill_up_last_data(curinverter,rest_time,thistime);
		fill_up_all_data(curinverter,rest_time);
		fill_up_single_data(curinverter,rest_time);
	}
	temp_time=rest_time-time(NULL);
	printdecmsg("temp_time",temp_time);
	if(temp_time>300)
	{
		printdecmsg("temp_time-300",temp_time-300);
		sleep(temp_time-300);
	}
	else if(temp_time>0)
	{
		printdecmsg("temp_time",temp_time);
		sleep(temp_time);
	}
	else
		;

	return time(NULL);
}
