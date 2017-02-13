#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "mydebug.h"
#include "mydatabase.h"
#include "myfile.h"

int read_signal_strength_num(const char *msg, int num)
{
	sqlite3 *db;
	int i, err_count = 0;
	char inverter_id[13] = {'\0'};
	char sql[1024] = {'\0'};

	if(!open_db("/home/database.db", &db))
	{
		snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS signal_strength(id VARCHAR(256), signal_strength INTEGER, set_flag INTEGER, primary key(id))");
		create_table(db, sql);
		for(i=0; i<num; i++)
		{
			//获取一台逆变器的ID号
			strncpy(inverter_id, &msg[i*12], 12);
			memset(sql, 0, sizeof(sql));
			snprintf(sql, sizeof(sql), "REPLACE INTO signal_strength "
					"(id, set_flag) VALUES('%s', 1) ", inverter_id);
			if(replace_data(db, sql) < 0)
				err_count++;
		}
		close_db(db);
	}
	return err_count;
}

/*【A128】EMA读取逆变器的信号强度*/
int read_inverter_signal_strength(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int type, num;
	char timestamp[15] = {'\0'};

	//获取设置类型标志位: 0读取所有逆变器，1读取指定逆变器
	type = msg_get_int(&recvbuffer[30], 1);

	//获取时间戳
	strncpy(timestamp, &recvbuffer[31], 14);

	switch(type)
	{
		case 0:
			//读取所有逆变器，存入配置文件
			file_set_one("ALL", "/tmp/read_all_signal_strength.conf");
			break;
		case 1:
			//获取逆变器数量
			num = msg_get_int(&recvbuffer[48], 4);

			//检查格式（逆变器数量）
			if(!msg_num_check(&recvbuffer[52], num, 12, 0)){
				ack_flag = FORMAT_ERROR;
			}
			else{
				//读取指定逆变器，存入数据库
				if(read_signal_strength_num(&recvbuffer[52], num) > 0)
					ack_flag = DB_ERROR;
			}
			break;
		default:
			ack_flag = FORMAT_ERROR;
			break;
	}
	file_set_one("1", "/tmp/upload_signal_strength.conf");

	//拼接应答消息
	msg_ACK(sendbuffer, "A128", timestamp, ack_flag);
	return 0;
}
