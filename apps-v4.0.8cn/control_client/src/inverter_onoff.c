#include <stdio.h>
#include <string.h>
#include <remote_control_protocol.h>
#include "mydebug.h"
#include "mydatabase.h"

/* 设置所有逆变器的开关机 */
int turn_onoff_all(sqlite3* db, int type)
{
	char **azResult = NULL;
	int nrow, ncolumn;
	int i, count = 0;
	char sql[1024] = {'\0'};

	//查询所有逆变器ID号
	snprintf(sql, sizeof(sql), "SELECT id FROM id");
	if(get_data(db, sql, &azResult, &nrow, &ncolumn))return count;

	for(i=1; i<=nrow; i++)
	{
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "REPLACE INTO turn_on_off "
				"(id, set_flag) VALUES ('%s', %d) ", azResult[i], type);
		if(!replace_data(db, sql))
		count++;
	}
	sqlite3_free_table(azResult);
	return count;
}

/* 设置指定台数逆变器的开关机 */
int turn_onoff_num(sqlite3* db, const char *msg, int num)
{
	char **azResult = NULL;
	int nrow, ncolumn;
	int i, onoff, err_count = 0;
	char inverter_id[13] = {'\0'};
	char sql[1024] = {'\0'};

	for(i=0; i<num; i++)
	{
		//获取一台逆变器的ID号
		strncpy(inverter_id, &msg[i*16], 12);
		//获取开机或关机(注意:协议中0是开,1是关;但在ECU数据库中1是开,2是关...所以这里加上1)
		onoff = msg_get_int(&msg[i*16 + 12], 1) + 1;
		//插入一条逆变器开关机指令
		snprintf(sql, sizeof(sql), "REPLACE INTO turn_on_off "
				"(id, set_flag) VALUES ('%s', %d)", inverter_id, onoff);
		if(replace_data(db, sql) < 0)
			err_count++;
	}
	sqlite3_free_table(azResult);
	return err_count;
}

/* 【A111】EMA设置逆变器开关机 */
int set_inverter_onoff(const char *recvbuffer, char *sendbuffer)
{
	sqlite3 *db;
	int ack_flag = SUCCESS;
	int type, num;
	char timestamp[15] = {'\0'};
	char sql[1024] = {'\0'};

	//获取设置类型标志位: 0全开; 1全关; 2指定逆变器开关
	type = msg_get_int(&recvbuffer[30], 1);
	//获取逆变器数量
	num = msg_get_int(&recvbuffer[31], 4);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[35], 14);

	if(!open_db("/home/database.db", &db))
	{
		snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS turn_on_off(id VARCHAR(256), set_flag INTEGER, primary key(id))");
		create_table(db, sql);
		switch(type)
		{
			case 0:
				turn_onoff_all(db, 1);
				break;
			case 1:
				turn_onoff_all(db, 2);
				break;
			case 2:
				//检查格式
				if(!msg_num_check(&recvbuffer[52], num, 13, 1)){
					ack_flag = FORMAT_ERROR;
				}
				else{
					if(turn_onoff_num(db, &recvbuffer[52], num) > 0)
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
	msg_ACK(sendbuffer, "A111", timestamp, ack_flag);
	return 0;
}
