#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "mydebug.h"
#include "mydatabase.h"
#include "myfile.h"

#define MAXPOWER_RANGE "020300"

/* 设置所有逆变器最大功率 */
int set_maxpower_all(sqlite3* db, int maxpower)
{
	char **azResult = NULL;
	int nrow, ncolumn;
	int i, j;
	char sql[1024] = {'\0'};
	char msg[18000] = {'\0'};

	//查询所有逆变器ID号
	snprintf(sql, sizeof(sql), "SELECT id FROM id");
	if(get_data(db, sql, &azResult, &nrow, &ncolumn))return 0;

	//将所有逆变器拼接成设置单台的形式
	for(i=1;i<=nrow;i++)
		for(j=0;j<ncolumn;j++){
			msgcat_s(msg, 12, azResult[i*ncolumn + j]);
			msgcat_d(msg, 3, maxpower);
			msgcat_s(msg, 3, "END");
		}
	sqlite3_free_table(azResult);

	return set_maxpower_num(db, msg, strlen(msg)/18);
}

/* 设置指定台数逆变器最大功率 */
int set_maxpower_num(sqlite3* db, const char *msg, int num)
{
	char **azResult = NULL;
	int nrow, ncolumn;
	int i, item, maxpower, err_count = 0;
	char inverter_id[13] = {'\0'};
	char sql[1024] = {'\0'};

	for(i=0; i<num; i++)
	{
		//获取一台逆变器的ID号
		strncpy(inverter_id, &msg[i*18], 12);
		//获取最大功率
		maxpower = msg_get_int(&msg[i*18 + 12], 3);
		if(maxpower < 0)
			continue;

		//查询该逆变器ID在表中是否存在(建议建表的时候以‘id’为主键，方便使用REPLASE插入)
		snprintf(sql, sizeof(sql), "SELECT COUNT(id) FROM power WHERE id='%s' ", inverter_id);
		if(get_data(db, sql, &azResult, &nrow, &ncolumn))break;
		//已经存在则用UPDATE更新
		if(atoi(azResult[1]) > 0){
			snprintf(sql, sizeof(sql),
					"UPDATE power SET limitedpower=%d,flag=1 WHERE id='%s' ", maxpower, inverter_id);
			if(update_data(db, sql) < 0)
				err_count++;
		}
		//不存在则用INSERT插入
		else{
			//获取逆变器ID的最大编号(item列貌似没什么用)
			snprintf(sql, sizeof(sql), "SELECT MAX(item) FROM power");
			if(get_data(db, sql, &azResult, &nrow, &ncolumn))break;
			if(azResult[1] == NULL)
				item = 0;
			else
				item = atoi(azResult[1]) + 1;
			//插入该逆变器的最大功率
			snprintf(sql, sizeof(sql), "INSERT INTO power "
					"(item,id,limitedpower,flag) VALUES (%d, '%s', %d, 1)",
					item, inverter_id, maxpower);
			if(insert_data(db, sql) < 0)
				err_count++;
		}
	}
	sqlite3_free_table(azResult);
	return err_count;
}

/* 【A110】EMA设置逆变器最大功率 */
int set_inverter_maxpower(const char *recvbuffer, char *sendbuffer)
{
	sqlite3 *db;
	int ack_flag = SUCCESS;
	int type, maxpower, num;
	char timestamp[15] = {'\0'};

	//获取设置类型标志位: 0设置全部, 1设置指定逆变器
	type = msg_get_int(&recvbuffer[30], 1);
	//获取逆变器数量
	num = msg_get_int(&recvbuffer[31], 4);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[35], 14);

	if(!open_db("/home/database.db", &db))
	{
		switch(type)
		{
			case 0:
				maxpower = msg_get_int(&recvbuffer[52], 3);
				if(maxpower >= 0){
					if(set_maxpower_all(db, maxpower) > 0)
						ack_flag = DB_ERROR;
				}
				break;
			case 1:
				//检查格式
				if(!msg_num_check(&recvbuffer[52], num, 15, 1)){
					ack_flag = FORMAT_ERROR;
				}
				else{
					if(set_maxpower_num(db, &recvbuffer[52], num) > 0)
						ack_flag = DB_ERROR;
				}
				break;
			default:
				ack_flag = FORMAT_ERROR;
				break;
		}
		close_db(db);
	}
	//拼接应答消息
	msg_ACK(sendbuffer, "A110", timestamp, ack_flag);
	return 0;
}

/* 【A117】读取逆变器最大功率及范围 */
int response_inverter_maxpower(const char *recvbuffer, char *sendbuffer)
{
	sqlite3 *db;
	char **azResult = NULL;
	int nrow = 0, ncolumn;
	char sql[1024] = {'\0'};
	int i, ack_flag = SUCCESS;
//	char ecuid[13] = {'\0'};
	char timestamp[15] = {'\0'};

	//设置读取所有逆变器最大功率的指令
	if(file_set_one("ALL", "/tmp/getmaxpower.conf") < 0){
		ack_flag = FILE_ERROR;
	}

//	//获取参数
//	file_get_one(ecuid, sizeof(ecuid), "/etc/yuneng/ecuid.conf");
	strncpy(timestamp, &recvbuffer[34], 14);
//	//从数据库中读取当前逆变器最大功率
//	if(!open_db("/home/database.db", &db))
//	{
//		memset(sql, 0, sizeof(sql));
//		snprintf(sql, sizeof(sql), "SELECT id.id,power.limitedresult FROM id "
//				"LEFT JOIN power ON id.id=power.id");
//		get_data(db, sql, &azResult, &nrow, &ncolumn);
//		//拼接信息
//		msg_Header(sendbuffer, "A117");
//		msgcat_s(sendbuffer, 12, ecuid);
//		msgcat_d(sendbuffer, 4, nrow);
//		msgcat_s(sendbuffer, 14, timestamp);
//		msgcat_s(sendbuffer, 3, "END");
//		for(i=1; i<=nrow; i++){
//			msgcat_s(sendbuffer, 12, azResult[i*ncolumn]);
//			if(azResult[i*ncolumn + 1] == NULL || (!strncmp(azResult[i*ncolumn + 1], "-", 1)))
//				msgcat_d(sendbuffer, 3, -1);
//			else
//				msgcat_d(sendbuffer, 3, atoi(azResult[i*ncolumn + 1]));
//			msgcat_s(sendbuffer, 6, MAXPOWER_RANGE);
//			msgcat_s(sendbuffer, 3, "END");
//		}
//		sqlite3_free_table(azResult);
//		close_db(db);
//	}
//	else{
//		//拼接应答消息
//		msg_ACK(sendbuffer, "A117", timestamp, 2);
//	}

	//拼接应答消息
	msg_ACK(sendbuffer, "A117", timestamp, ack_flag);
	return 0;
}
