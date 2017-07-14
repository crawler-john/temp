#include <stdio.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "mydebug.h"
#include "mydatabase.h"
#include "myfile.h"
#include "sqlite3.h"

/*【A145】上报逆变器的功率因数*/ //会同时触发A131；所以在A118中未加入A145，因为A131也会触发A145
int response_inverter_power_factor(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	
	//读取逆变器交流保护参数+功率因数
	if(file_set_one("2", "/tmp/presetdata.conf")){
		ack_flag = FILE_ERROR;
	}
	
	//获取时间戳
	strncpy(timestamp, &recvbuffer[34], 14);
	
	//拼接应答消息
	msg_ACK(sendbuffer, "A145", timestamp, ack_flag);
	return 0;

}

int set_all_inverter_power_factor(char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	sqlite3 *db;
	char sql[1024]={'\0'};
	char *zErrMsg = 0;
	int temp = 0;
	if(recvbuffer[30]=='A')
	{
		temp = msg_get_int(&recvbuffer[31],3);
		if(temp == 100)
			temp = 31;
		else
			temp = temp/10+11;
	}
	else if(recvbuffer[30]=='B')
	{
		temp = temp/10+1;
	}
	else
	{
		ack_flag = FORMAT_ERROR;
	}
	strncpy(timestamp, &recvbuffer[34], 14);
	if((recvbuffer[30]=='A')||(recvbuffer[30]=='B'))
	{
		
		if(!open_db("/home/database.db", &db))
		{
			sprintf(sql,"REPLACE INTO set_protection_parameters (parameter_name,parameter_value,set_flag) VALUES('power_factor',%d,1)",temp);
			sqlite3_exec(db, sql, 0, 0, &zErrMsg);
			close_db(db);
		}
		else ack_flag=DB_ERROR;
	}
	msg_ACK(sendbuffer, "A146", timestamp, ack_flag);
	return 0;


}
