#include <stdio.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "mydebug.h"
#include "mydatabase.h"
#include "myfile.h"
#include "sqlite3.h"

/*【A148】读取异常3501uid*/
int read_wrong_id(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char a;
	char timestamp[15] = {'\0'};
	strncpy(timestamp, &recvbuffer[34], 14);
	FILE *fp;
	fp = fopen("/etc/yuneng/get_bad_id.conf", "w");
	if(fp){
		fputs("1",fp);
		fclose(fp);
	}
	fp = fopen("/etc/yuneng/get_bad_id.conf", "r");
	if(fp){
		a = fgetc(fp);
		fclose(fp);
	}
	if(a!='1')ack_flag=FILE_ERROR;
	msg_ACK(sendbuffer, "A148", timestamp,ack_flag);
	return 0;
}


int set_unnormal_id(const char *recvbuffer, char *sendbuffer)
{
	sqlite3 *db;
	int nrow, ncolumn,num,i;
	char **azResult = NULL;
	char *zErrMsg = 0;
	char sql[1024] = {'\0'};
	char yuid[13];
	char nuid[13];
	int ack_flag = SUCCESS;
	char a;
	char timestamp[15] = {'\0'};
	strncpy(timestamp, &recvbuffer[34], 14);
	if(!sqlite3_open("/home/database.db",&db))
	{
		num = msg_get_int(&recvbuffer[30], 4);
		for(i=0;i<num;i++)
		{
			memset(sql,'\0',1024);
			strncpy(yuid,&recvbuffer[51+28*i],12);
			strncpy(nuid,&recvbuffer[64+28*i],12);
			sprintf(sql,"DELETE FROM need_id WHERE wrongid='%s'",nuid);
			sqlite3_exec(db, sql , 0, 0, &zErrMsg);
			sprintf(sql,"INSERT INTO need_id (correct_id,wrongid,set_flag) VALUES('%s','%s',1)",yuid,nuid);
			if(-1==insert_data(db, sql))
				ack_flag=DB_ERROR;
		}
		sqlite3_close(db);
	}
	else ack_flag=DB_ERROR;
	msg_ACK(sendbuffer, "A150", timestamp, ack_flag);
	return 0;
}
