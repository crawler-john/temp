#include <stdio.h>
#include <string.h>
#include <remote_control_protocol.h>
#include "mydebug.h"
#include "mydatabase.h"

/* 清除所有逆变器的GFDI */
int clear_all(sqlite3* db)
{
	char **azResult = NULL;
	int nrow, ncolumn;
	int i, err_count = 0;
	char sql[1024] = {'\0'};

	//查询所有逆变器ID号
	snprintf(sql, sizeof(sql), "SELECT id FROM id");
	if(get_data(db, sql, &azResult, &nrow, &ncolumn))return 1;

	for(i=1; i<=nrow; i++)
	{
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "REPLACE INTO clear_gfdi "
				"(id, set_flag) VALUES ('%s', 1)", azResult[i]);
		if(replace_data(db, sql) < 0)
			err_count++;
	}
	sqlite3_free_table(azResult);
	return err_count;
}

/* 清除指定台数逆变器的GFDI */
int clear_num(sqlite3* db, const char *msg, int num)
{
	char **azResult = NULL;
	int nrow, ncolumn;
	int i, err_count = 0;
	char inverter_id[13] = {'\0'};
	char sql[1024] = {'\0'};

	for(i=0; i<num; i++)
	{
		//获取一台逆变器的ID号
		strncpy(inverter_id, &msg[i*16], 12);
		//获取清除GFDI标志(注意:协议中0是维持原样,1是清除标志;但在ECU数据库中1是清除标志，并没有0)
		if(msg_get_int(&msg[i*16 + 12], 1)){
			//插入一条逆变器开关机指令
			snprintf(sql, sizeof(sql), "REPLACE INTO clear_gfdi "
					"(id, set_flag) VALUES ('%s', 1)", inverter_id);
			if(replace_data(db, sql) < 0)
				err_count++;
		}
	}
	sqlite3_free_table(azResult);
	return err_count;
}

/* 【A112】EMA设置逆变器GFDI */
int clear_inverter_gfdi(const char *recvbuffer, char *sendbuffer)
{
	sqlite3 *db;
	int ack_flag = SUCCESS;
	int type, num;
	char timestamp[15] = {'\0'};
	char sql[1024] = {'\0'};

	//获取设置类型标志位: 0全部清除, 1指定逆变器清除
	type = msg_get_int(&recvbuffer[30], 1);
	//获取逆变器数量
	num = msg_get_int(&recvbuffer[31], 4);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[35], 14);

	if(!open_db("/home/database.db", &db))
	{
		snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS clear_gfdi(id VARCHAR(256), set_flag INTEGER, primary key(id))");
		create_table(db, sql);
		switch(type)
		{
			case 0:
				if(clear_all(db) > 0)
					ack_flag = DB_ERROR;
				break;
			case 1:
				//检查格式
				if(!msg_num_check(&recvbuffer[52], num, 13, 1)){
					ack_flag = FORMAT_ERROR;
				}
				else{
					if(clear_num(db, &recvbuffer[52], num) > 0)
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
	msg_ACK(sendbuffer, "A112", timestamp, ack_flag);
	return 0;
}
