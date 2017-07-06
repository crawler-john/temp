/*
 * ZigBee补发数据功能
 * 其他文件改动：
 * 一、main.c文件中：
 * 1.包含头文件：fill_up_data.h
 * 2.init_all()中加入初始化补数据结构体函数：init_lost_data_info()
 * 3.主函数中定义时间戳int变量：time_linux，保存每次轮询的时间戳
 * 4.将get_time()函数返回值赋值给time_linux变量
 * 5.在getalldata()函数之前调用calibration_time_broadcast(time_linux)，用于广播校准逆变器的时间
 * 6.在getalldata()函数之后调用save_time_to_database(inverter, time_linux)，用于将补报时间戳保存到数据库
 * 7.在轮询结束后的空闲期调用fill_up_data()执行补数据操作
 * 二、datetime.c文件中：
 * 1.get_time()函数返回当前轮询的时间戳
 * 三、protocol.c文件中：
 * 1.protocol()函数中的消息头从"APS1400000AAAAAAA1"改为"APS15000000002AAA1"
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"
#include <time.h>
#include "debug.h"
#include "save_historical_data.h"
#include "protocol.h"

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
extern ecu_info ecu;
int turn_on_flag=0;			//补数据开关标志，1开始补数据功能，0关闭补数据功能

/*
 * 初始化补数据结构体信息
 * 用于main.exe启动后和读取所有逆变器数据结束后
 */
int init_lost_data_info()
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
	printf("init_lost_data_info: %d--->%d\n",__LINE__,turn_on_flag);
	lost_data_info.current_inverter=0;
	lost_data_info.inverter_count=0;
	memset(lost_data_info.sendbuff, '\0', sizeof(lost_data_info.sendbuff));
	memset(lost_data_info.sendbuff_all, '\0', sizeof(lost_data_info.sendbuff_all));
	lost_data_info.syspower=0;
	lost_data_info.curgeneration=0;
	lost_data_info.ltgeneration=0;
	lost_data_info.num=0;
	return 0;
}

/*
 * 发送时间校准指令(广播帧)
 */
int calibration_time_broadcast(struct inverter_info_t *firstinverter,int time_linux)
{

	unsigned char sendbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int i=0;
	struct inverter_info_t *curinverter = firstinverter;
//	printf("%d\n",__LINE__);
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)
	{
		if(1 != curinverter->fill_up_data_flag)
			return 0;
	}
	printf("%d\n",__LINE__);
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x11;			//LENGTH
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0x0D;
	sendbuff[6] = time_linux/16777216;
	sendbuff[7] = (time_linux%16777216)/65536;
	sendbuff[8] = ((time_linux%16777216)%65536)/256;
	sendbuff[9] = ((time_linux%16777216)%65536)%256;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0x00;
	sendbuff[14] = 0x00;
	sendbuff[15] = 0x00;
	sendbuff[16] = 0x00;
	sendbuff[17] = 0x00;
	sendbuff[18] = 0x04;
	sendbuff[19] = 0x01;

	for(i=2; i<20; i++)
		check = check + sendbuff[i];

	sendbuff[20] = check >> 8;	//CHK
	sendbuff[21] = check;		//CHK
	sendbuff[22] = 0xFE;		//TAIL
	sendbuff[23] = 0xFE;		//TAIL

	printmsg("Calibration time broadcast");
	zb_broadcast_cmd(sendbuff, 24);
	printhexmsg("sendbuff",sendbuff, 24);
	sleep(10);

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

	sprintf(sql, "SELECT model FROM get_inverter_model WHERE inverterid=%s",inverter->id);
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
				inverter->fill_up_data_flag=1;					//2：无补数据功能
			}
			else if(3==atoi(azResult[1]))
			{
				inverter->fill_up_data_flag=1;					//没有响应，不确定是否有补数据功能
				flag=1;
			}
		}
		else
		{
			inverter->fill_up_data_flag=1;						//从未问询过机器型号，为第一次
			flag=1;
		}
	}
	else
	{
		flag=1;										//第一次创建表，表里没有任何东西
		inverter->fill_up_data_flag=1;						//从未问询过机器型号，为第一次
	}

	sqlite3_free_table( azResult );
	sqlite3_close(db);

	printmsg("debug44444");
//	if((1==flag)&&('1' != inverter->inverterid[0]))
//	{
//		sendbuff[0] = 0xFB;			//HEAD
//		sendbuff[1] = 0xFB;			//HEAD
//		sendbuff[2] = 0x01;			//CMD
//		sendbuff[3] = 0x00;			//LENGTH
//		sendbuff[4] = 0x11;			//LENGTH
//		sendbuff[5] = ccuid[0];		//CCID
//		sendbuff[6] = ccuid[1];		//CCID
//		sendbuff[7] = ccuid[2];		//CCID
//		sendbuff[8] = ccuid[3];		//CCID
//		sendbuff[9] = ccuid[4];		//CCID
//		sendbuff[10] = ccuid[5];		//CCID
//		sendbuff[11] = inverter->tnuid[0];		//TNID
//		sendbuff[12] = inverter->tnuid[1];		//TNID
//		sendbuff[13] = inverter->tnuid[2];		//TNID
//		sendbuff[14] = inverter->tnuid[3];		//TNID
//		sendbuff[15] = inverter->tnuid[4];		//TNID
//		sendbuff[16] = inverter->tnuid[5];		//TNID
//		sendbuff[17] = 0x4F;
//		sendbuff[18] = 0x00;
//		sendbuff[19] = 0x00;
//		sendbuff[20] = 0xDC;
//		sendbuff[21] = 0x11;
//
//		for(i=2; i<22; i++)
//		{
//			check = check + sendbuff[i];
//		}
//
//		sendbuff[22] = check >> 8;		//CHK
//		sendbuff[23] = check;		//CHK
//		sendbuff[24] = 0xFE;		//TAIL
//		sendbuff[25] = 0xFE;		//TAIL
//
//		printhexmsg("Get inverter model", sendbuff, 26);
//		write(zbmodem, sendbuff, 26);
//
//		ret = plc_get_reply_fromID(readbuff,inverter->inverterid);
//
//		if((37==ret)&&(0xFB == readbuff[0])&&(0xFB == readbuff[1])&&
//			(sendbuff[11] == readbuff[11])&&
//			(sendbuff[12] == readbuff[12])&&
//			(sendbuff[13] == readbuff[13])&&
//			(sendbuff[14] == readbuff[14])&&
//			(sendbuff[15] == readbuff[15])&&
//			(sendbuff[16] == readbuff[16])&&
//			(0xDC == readbuff[20])&&(0xFE == readbuff[35])&&(0xFE == readbuff[36]))
//		{
//			if(0x01 == readbuff[23])
//			{
//				save_inverter_replacement_model(inverter->inverterid,1);		//1:有补数据功能
//				inverter->fill_up_data_flag=1;
//			}
//			else if(0x00 == readbuff[23])
//			{
//				save_inverter_replacement_model(inverter->inverterid,2);		//2:无补数据功能
//			//	inverter->last_replacement_data_flag=1;
//				inverter->fill_up_data_flag=2;
//			}
//		}
//		else
//		{
//			save_inverter_replacement_model(inverter->inverterid,3);			//3:没有回应，不确定是否有补数据功能
//			inverter->fill_up_data_flag=0;
//		}
//	}
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

/*
 * 清除之前的时间戳
 * 输入：逆变器ID，时间戳
 * 功能：删除该逆变器在此补报时间戳之前的所有补报时间戳
 */
int clear_single_less_than_time(int time_linux,char* inverterid)
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

/*
 * 清除所有时间戳
 * 输入：逆变器ID
 * 功能：删除该逆变器的所有补报时间戳
 */
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

/*
 * 从lost_all_time表中删除已经补报完毕的时间戳
 */
int clear_all_time(int time_linux)
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

/*
 * 读取所有逆变器丢失数据的单个时间戳
 */
int get_all_time()
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
		return all_time_linux;
	}
	else
	{
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return -1;
	}

}

/*
 * 读取该台逆变器的一个补报时间戳
 */
int get_single_time(char* inverterid)
{
	sqlite3 *db;
	char sql[1024]={'\0'};
    char *zErrMsg=0;
    char **azResult;
    int nrow=0,ncolumn;
    int time_linux=0;

    sqlite3_open("/tmp/replacement.db", &db);
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

/*
 * 检查save_replacement_time表是否为空
 */
int check_single_time()
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

/*
 * 将需要补报的逆变器和时间戳存入数据库
 */
int save_time_to_database(struct inverter_info_t *firstinverter,int time_linux)
{
	int k=0;
	int m=0;
	int i, j, currentcount = 0;
	struct inverter_info_t *curinverter = firstinverter;

	if(0 == turn_on_flag)
		return 0;

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
	{
		get_inverter_model(curinverter);
		curinverter++;
	}

	curinverter = firstinverter;printf("%d ddd%d \n",curinverter->dataflag,curinverter->fill_up_data_flag);
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
	{
		if((curinverter->dataflag==0)&&(1 == curinverter->fill_up_data_flag))
			save_replacement_time(curinverter->id , time_linux);		//把有数据补包功能的逆变器ID和时间保存到表格里
		curinverter++;
	}

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
	{
		if(curinverter->dataflag==1)
		{
			k++;			// k为计数标志，如果轮询完所有逆变器，k仍然为0，则说明在此时间点，所有逆变器都没有获取到数据，就保存到所有掉点表格里
		}
		printdecmsg("k",k);
		curinverter++;
	}

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
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

/*
 * 发送补报数据指令
 */
int query_lost_data(inverter_info *inverter, int time, char *data, char* receivetime)
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int temp_time;
	int i, j=0;
	int ret, size;
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};


	//模拟逆变器数据 K1-K80
	char test_data_BB[80] = {0x05, 0x16, 0x13, 0xFC, 0x00, 0x00, 0x00, 0x47, 0x4C, 0x6E,
							 0xF0, 0x00, 0x29, 0x32, 0x80, 0x25, 0x82, 0x81, 0x27, 0xA1,
							 0xBD, 0x28, 0x47, 0x60, 0x27, 0xFF, 0x9D, 0x04, 0xF2, 0xD4,
							 0xEF, 0xD5, 0x04, 0xCF, 0x21, 0x97, 0xC5, 0x04, 0x84, 0x4F,
							 0xB8, 0x7F, 0x04, 0xD6, 0xD4, 0x78, 0xFC, 0x00, 0x2E, 0x57,
							 0x00, 0x19, 0xC1, 0x00, 0x17, 0xF7, 0x02, 0xD9, 0xD6, 0x02,
							 0xCD, 0xE1, 0x02, 0xCC, 0x5F, 0x01, 0x66, 0x74, 0x01, 0x5F,
							 0xB9, 0x01, 0x5F, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x11;			//LENGTH
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0x0F;
	sendbuff[6] = 0x00;
	sendbuff[7] = time/16777216;
	sendbuff[8] = (time%16777216)/65536;
	sendbuff[9] = ((time%16777216)%65536)/256;
	sendbuff[10] = ((time%16777216)%65536)%256;
	sendbuff[11] = 0x00;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0x00;
	sendbuff[14] = 0x00;
	sendbuff[15] = 0x00;
	sendbuff[16] = 0x00;
	sendbuff[17] = 0x00;
	sendbuff[18] = 0x04;
	sendbuff[19] = 0x01;
	for(i=2; i<20; i++)
		check = check + sendbuff[i];
	sendbuff[20] = check >> 8;	//CHK
	sendbuff[21] = check;		//CHK
	sendbuff[22] = 0xFE;		//TAIL
	sendbuff[23] = 0xFE;		//TAIL

	printmsg("Replacement_data");
	zb_send_cmd(inverter, sendbuff, 24);
	printhexmsg("sendbuff",sendbuff,24);
	ret = zb_get_reply(readbuff, inverter);


	//替换成模拟数据
//	for (i=0; i<80; i++) {
//		readbuff[19+i] = test_data_BB[i];
//	}



	if ((105 == ret) &&
		(0xFB == readbuff[0]) && (0xFB == readbuff[1]) &&
		(0x62 == readbuff[2]) && (0x0F == readbuff[5]) &&
		(0xFE == readbuff[103]) && (0xFE == readbuff[104])) {

		/* 解析逆变器返回消息 */

		//解析接收时间
		memset(receivetime, '\0', sizeof(receivetime));
		tm = readbuff[15]*16777216+readbuff[16]*65536+readbuff[17]*256+readbuff[18];
		memcpy(&record_time,localtime(&tm), sizeof(record_time));
		sprintf(receivetime, "%04d%02d%02d%02d%02d%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
				record_time.tm_hour, record_time.tm_min, record_time.tm_sec);
		print2msg("Receive time", receivetime);

		//获取数据并删除时间戳
		if (0x01 == readbuff[7]) {
			//补报成功，截取80位补报数据到data
			for (i=0, j=19; i<80; i++, j++) {
				data[i] = readbuff[j];
			}
			clear_single_time(time,inverter->id);
			return ret;
		}
		else {
			//补报失败
			if (0==readbuff[10]) {
				if (1==readbuff[9]) {
					//清除此时间点以前该逆变器的补报时间点
					temp_time = readbuff[15]*16777216+readbuff[16]*65536+readbuff[17]*256+readbuff[18];
					printdecmsg("temp_time",temp_time);
					clear_single_less_than_time(temp_time,inverter->id);
					return 2;
				}
				else if (0==readbuff[9]) {
					//清除该逆变器的所有补报时间点
					clear_single_alltime(inverter->id);
					return -1;
				}
			}
		}
	}
	else {
		return -1;
	}
}


int query_last_lost_data(inverter_info *inverter, int time, char *data, char* receivetime)		//向逆变器读取丢失的昨天最后一包数据
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int temp_time;
	int i, j=0;
	int ret, size;
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x11;			//LENGTH
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0x0F;
	sendbuff[6] = 0x01;
	sendbuff[7] = time/16777216;
	sendbuff[8] = (time%16777216)/65536;
	sendbuff[9] = ((time%16777216)%65536)/256;
	sendbuff[10] = ((time%16777216)%65536)%256;
	sendbuff[11] = 0x00;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0x00;
	sendbuff[14] = 0x00;
	sendbuff[15] = 0x00;
	sendbuff[16] = 0x00;
	sendbuff[17] = 0x00;
	sendbuff[18] = 0x04;
	sendbuff[19] = 0x01;
	for(i=2; i<20; i++)
		check = check + sendbuff[i];
	sendbuff[20] = check >> 8;	//CHK
	sendbuff[21] = check;		//CHK
	sendbuff[22] = 0xFE;		//TAIL
	sendbuff[23] = 0xFE;		//TAIL

	printmsg("Replacement_data");
	zb_send_cmd(inverter, sendbuff, 24);
	printhexmsg("sendbuff",sendbuff,24);
	ret = zb_get_reply(readbuff, inverter);

	if ((105 == ret) &&
		(0xFB == readbuff[0]) && (0xFB == readbuff[1]) &&
		(0x62 == readbuff[2]) && (0x0F == readbuff[5]) &&
		(0xFE == readbuff[103]) && (0xFE == readbuff[104])) {
		/* 解析逆变器返回消息 */

		//解析接收时间
		memset(receivetime, '\0', sizeof(receivetime));
		tm = readbuff[15]*16777216+readbuff[16]*65536+readbuff[17]*256+readbuff[18];
		clear_single_time(tm,inverter->id);	//补最后一包这里，只要读取到逆变器返回的最后一包时间，就立刻删除在表格中的这个时间点，以免单台逆变器补包的时候重复
		memcpy(&record_time,localtime(&tm), sizeof(record_time));
		sprintf(receivetime, "%04d%02d%02d%02d%02d%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
				record_time.tm_hour, record_time.tm_min, record_time.tm_sec);
		print2msg("Receive time", receivetime);

		//获取数据并删除时间戳
		if (0x01 == readbuff[7]) {
			//补报成功，截取80位补报数据到data
			for (i=0, j=19; i<80; i++, j++) {
				data[i] = readbuff[j];
			}
			clear_single_time(time,inverter->id);
			return ret;
		}
		else {
			return -1;
		}
	}
}

/*
 * 保存补报数据到数据库(YC1000)
 * 输入：补报信息，逆变器ID，接收时间，最后一帧标志位(为1时计算电量)
 * 保存在fill_up_data表中
 */
int resolve_fill_up_data_to_database_for_yc1000(char *inverter_data,char *inverterid, char *receivetime,int last_data_mark)
{
	inverter_info fill_up_inverter[1];
	char buff[65535]={'\0'};
	float curgeneration_total = 0.0;	//逆变器当前一轮的总电量
	int op_total = 0;							//输出总功率
	char wendu[4] = {'\0'};

	memset(fill_up_inverter, 0, sizeof(inverter_info));
	//解析YC1000数据
	resolvedata_1000(inverter_data, fill_up_inverter);

	//电量计算,若不是最后一帧，则将电量置0
	if (last_data_mark != 1) {
		fill_up_inverter->curgeneration = 0;
		fill_up_inverter->curgenerationb = 0;
		fill_up_inverter->curgenerationc = 0;
		fill_up_inverter->curgenerationd = 0;
	}

	//拼接数据
	strcat(buff, inverterid);
//	strcat(buff, fill_up_inverter->id); //?????????????????????????????
	strcat(buff, "04");
	transdv(buff, fill_up_inverter->dv*10.0);
	transfrequency(buff, fill_up_inverter->gf*10.0);
	sprintf(wendu,"%03d",fill_up_inverter->it+100);
	strcat(buff,wendu);
	strcat(buff, "1");
	transdi(buff, fill_up_inverter->di*10.0);
	transgridvolt(buff, fill_up_inverter->gv);
	transreactivepower(buff,(int) (fill_up_inverter->reactive_power));
	transactivepower(buff,(int)( fill_up_inverter->active_power));
	transcurgen(buff, fill_up_inverter->output_energy*1000000.0);
	transcurgen(buff, fill_up_inverter->curgeneration*1000000.0);
	transpower(buff, fill_up_inverter->op);
//	transstatus(buff, fill_up_inverter->status);
	strcat(buff, "2");
	transdi(buff, fill_up_inverter->dib*10.0);
	transgridvolt(buff, fill_up_inverter->gvb);
	transreactivepower(buff, (int)(fill_up_inverter->reactive_powerb));
	transactivepower(buff,(int)(fill_up_inverter->active_powerb));
	transcurgen(buff, fill_up_inverter->output_energyb*1000000.0);
	transcurgen(buff, fill_up_inverter->curgenerationb*1000000.0);
	transpower(buff, fill_up_inverter->opb);
//	transstatus(buff, fill_up_inverter->statusb);
	strcat(buff, "3");
	transdi(buff, fill_up_inverter->dic*10.0);
	transgridvolt(buff, fill_up_inverter->gvc);
	transreactivepower(buff, (int)(fill_up_inverter->reactive_powerc));
	transactivepower(buff, (int)(fill_up_inverter->active_powerc));
	transcurgen(buff, fill_up_inverter->output_energyc*1000000.0);
	transcurgen(buff, fill_up_inverter->curgenerationc*1000000.0);
	transpower(buff, fill_up_inverter->opc);
//	transstatus(buff, fill_up_inverter->statusc);
	strcat(buff, "4");
	transdi(buff, fill_up_inverter->did*10.0);
	transcurgen(buff, fill_up_inverter->curgenerationd*1000000.0);
	transpower(buff, fill_up_inverter->opd);

	print2msg("buff", buff);
	save_single_lost_data(buff,receivetime);

	if (last_data_mark == 1)
	{
		curgeneration_total = fill_up_inverter->curgeneration +
							  fill_up_inverter->curgenerationb +
							  fill_up_inverter->curgenerationc +
							  fill_up_inverter->curgenerationd;
		op_total = fill_up_inverter->op +
				   fill_up_inverter->opb +
				   fill_up_inverter->opc +
				   fill_up_inverter->opd;
		ecu.life_energy = ecu.life_energy + curgeneration_total;
		update_lifetime_energy(ecu.life_energy);
		update_daily_energy(curgeneration_total,receivetime);
		update_monthly_energy(curgeneration_total,receivetime);
		update_yearly_energy(curgeneration_total,receivetime);
		save_system_power_single(op_total,receivetime);
	}

	return 0;
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

/*
 * 保存补报数据到缓存(YC1000)
 * 输入：补报信息，逆变器ID
 * 缓存在lost_data_info.sendbuff中
 */
int resolve_fill_up_data_to_buffer_for_yc1000(char *inverter_data, char *inverterid)
{
	inverter_info fill_up_inverter[1];
	char buff[1024] = {'\0'};
	char wendu[4] = {'\0'};

	memset(fill_up_inverter, 0, sizeof(inverter_info));
	//解析YC1000数据
	resolvedata_1000(inverter_data, fill_up_inverter);

	//拼接数据
	strcat(buff, inverterid);
	strcat(buff, "04");
	transdv(buff, 10.0*fill_up_inverter->dv);
	transfrequency(buff, 10*fill_up_inverter->gf);
	sprintf(wendu,"%03d",fill_up_inverter->it);
	strcat(buff,wendu);
	strcat(buff, "1");
	transdi(buff, 10*fill_up_inverter->di);
	transgridvolt(buff, fill_up_inverter->gv);
	transreactivepower(buff, (int)(fill_up_inverter->reactive_power));
	transactivepower(buff, (int)(fill_up_inverter->active_power));
	transcurgen(buff, 1000000*fill_up_inverter->output_energy);
	transcurgen(buff, 1000000*fill_up_inverter->curgeneration);
	transpower(buff, fill_up_inverter->op);
//	transstatus(buff, fill_up_inverter->status);
	strcat(buff, "2");
	transdi(buff, 10*fill_up_inverter->dib);
	transgridvolt(buff, fill_up_inverter->gvb);
	transreactivepower(buff, (int)(fill_up_inverter->reactive_powerb));
	transactivepower(buff, (int)(fill_up_inverter->active_powerb));
	transcurgen(buff, 1000000*fill_up_inverter->output_energyb);
	transcurgen(buff, 1000000*fill_up_inverter->curgenerationb);
	transpower(buff, fill_up_inverter->opb);
//	transstatus(buff, fill_up_inverter->statusb);
	strcat(buff, "3");
	transdi(buff, 10*fill_up_inverter->dic);
	transgridvolt(buff, fill_up_inverter->gvc);
	transreactivepower(buff, (int)(fill_up_inverter->reactive_powerc));
	transactivepower(buff, (int)(fill_up_inverter->active_powerc));
	transcurgen(buff, 1000000*fill_up_inverter->output_energyc);
	transcurgen(buff, 1000000*fill_up_inverter->curgenerationc);
	transpower(buff, fill_up_inverter->opc);
//	transstatus(buff, fill_up_inverter->statusc);
	strcat(buff, "4");
	transdi(buff, 10*fill_up_inverter->did);
	transcurgen(buff, 1000000*fill_up_inverter->curgenerationd);
	transpower(buff, fill_up_inverter->opd);
	strcat(buff, "END");

	print2msg("buff", buff);
	strcat(lost_data_info.sendbuff, buff);
	lost_data_info.syspower = lost_data_info.syspower + fill_up_inverter->op + fill_up_inverter->opb + fill_up_inverter->opc + fill_up_inverter->opd;
	lost_data_info.curgeneration = lost_data_info.curgeneration + fill_up_inverter->curgeneration + fill_up_inverter->curgenerationb + fill_up_inverter->curgenerationc + fill_up_inverter->curgenerationd;
	lost_data_info.inverter_count++;			//获取到数据的逆变器个数加1
	return 0;
}

/*
 * 拼接单个时间全部丢失的补报协议
 * 结果存入：Data表(与普通轮询格式一样)
 */
int protocol_all_lost_data(char *receivetime, int all_time_linux)
{
	unsigned char temp[10] = {'\0'};
	int i;

	if(lost_data_info.inverter_count > 0) //当至少有一台逆变器获取到数据时候，才进行头拼接并发送到record表,并且清除lost_all_time表中这个时间点
	{
		sprintf(lost_data_info.sendbuff_all, "APS180000000000001%s%010d%010d%010d%s%03d",
				ecu.id,
				lost_data_info.syspower,
				(int)(lost_data_info.curgeneration*1000000.0),
				(int)(lost_data_info.ltgeneration*10.0),
				receivetime,
				lost_data_info.inverter_count);

		if(!ecu.zoneflag)
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
	}
	return 0;
}

/*
 * 补单个时间点全部逆变器的数据
 */
int fill_up_all_data(struct inverter_info_t *firstinverter,int rest_time)
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
			return -1;
		printdecmsg("Fill Up Time Stamp",all_time_linux);
		printdecmsg("Fill Up Current Inverter",lost_data_info.current_inverter);
		printdecmsg("Total",ecu.total);
		for(i=lost_data_info.current_inverter; (i < ecu.total); i++, lost_data_info.current_inverter++,curinverter++)
		{
			memset(data, '\0', sizeof(data));
			curinverter = firstinverter+lost_data_info.current_inverter;
			if (rest_time-time(NULL) < 25)
				return -1;
			//发送补报指令
			if(1 == curinverter->fill_up_data_flag)				//判断此台逆变器有补数据功能才进行补数据操作
			{
				flag = query_lost_data(curinverter, all_time_linux, data,receivetime);
				if (105 == flag)
					resolve_fill_up_data_to_buffer_for_yc1000(data,curinverter->id);
			}
		}
		clear_all_time(all_time_linux);
		if (lost_data_info.inverter_count>0) {
			save_system_power(lost_data_info.syspower,receivetime);
		}
		protocol_all_lost_data(receivetime, all_time_linux);
		init_lost_data_info();
	}
	return 1;
}

/*
 * 补单个时间点单个逆变器的数据
 */
int fill_up_single_data(struct inverter_info_t *firstinverter,int rest_time)
{
	char inverterid[13]={'\0'};
	int time_linux=0;
	int i,result=0;
	char data[512] = {'\0'};
	char receivetime[20]={'\0'};					//发送给EMA时的日期和时间，格式：年月日时分秒
	struct inverter_info_t *curinverter = firstinverter+lost_data_info.num;
	int fill_up_times = 4;				//补包总轮询次数,超过4次，则在本轮询不再补包

	if (-1 == check_single_time())
		return -1;

	while(fill_up_times--)
	{
		printmsg("Check Inverter Fill Up Data");
		printdecmsg("Total", ecu.total);
		for(i = lost_data_info.num; (i < ecu.total); i++, lost_data_info.num++)
		{
			if(-1 == check_single_time())
				return -1;
			curinverter = firstinverter + lost_data_info.num;
			while((time_linux = get_single_time(curinverter->id)) > 0)
			{
				printdecmsg("Rest Time", rest_time);
				memset(data, '\0', sizeof(data));
				printdecmsg("TIME_LINUX", time_linux);
				print2msg("Current Fill Up Inverter", curinverter->id);
				printdecmsg("lost_data_info.num",lost_data_info.num);
				if((rest_time - time(NULL)) < 25)
					return -1;
				result = query_lost_data(curinverter, time_linux, data,receivetime);
				if(-1 == result) //-1时直接跳下一台逆变器
					break;
				if(105 == result)
					resolve_fill_up_data_to_database_for_yc1000(data,curinverter->id, receivetime,0);
			}
			sleep(1);
		}
		init_lost_data_info();
	}
	return 0;
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

/*
 * 补单个时间所有逆变器丢失的数据
 */
int fill_up_last_data(struct inverter_info_t *firstinverter,int rest_time,int thistime)
{
	int i,all_time_linux=0;
	char data[512] = {'\0'};
	char receivetime[20]={'\0'}; //发送给EMA时的日期和时间，格式：年月日时分秒
	int first_boot_time;
	struct inverter_info_t *curinverter = firstinverter+lost_data_info.current_inverter;
	int mark=0;

	if(0==first_boot_mark)
	{
		first_boot_time = transform_first_time(thistime);
		for(i=lost_data_info.current_inverter; (i < ecu.total); i++, lost_data_info.current_inverter++,curinverter++)
		{
			memset(data, '\0', sizeof(data));
			if(rest_time-time(NULL)<25)
				return-1;

			mark = query_last_lost_data(curinverter, first_boot_time, data,receivetime);
			if(105==mark)
				resolve_fill_up_data_to_database_for_yc1000(data,curinverter->id, receivetime,1);

			if(i==ecu.total-1)
			{
				init_lost_data_info();
				first_boot_mark=1;			//当最后一台逆变器补最后一包结束后，标志位置为1
				break;
			}
		}
	}
	return 1;
}

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
				for(j=0, exist=0, curinverter=firstinverter; (j<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); j++, curinverter++)
				{
					if(!strcmp(curinverter->id, azResult[k]))
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

/*
 * 处理数据补报
 * 输入：逆变器结构体，当前轮询空闲时间，当前轮询时间戳
 * 1.fill_up_last_data		补前一天最后一轮数据
 * 2.fill_up_all_data		补全部逆变器，按正常轮询解析，解析结果存入Data表
 * 3.fill_up_single_data	补单个逆变器，按补报协议解析，解析结果存入fill_up_data表
 * 注：一但空闲时间少于25秒，就退出补报
 */
int fill_up_data(struct inverter_info_t *firstinverter,int rest_time,int thistime)
{
	struct inverter_info_t *curinverter = firstinverter;
	int temp_time;

	if(0 == turn_on_flag)
		return time(NULL);

	find_changed_ids(firstinverter);

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
	else ;

	return time(NULL);
}
