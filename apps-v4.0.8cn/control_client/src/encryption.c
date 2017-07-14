#include <stdio.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "mydebug.h"
#include "mydatabase.h"
#include "myfile.h"
#include "sqlite3.h"

/*【A140】读取逆变器防盗状态*/
int response_inverter_encryption(const char *recvbuffer, char *sendbuffer)
{
	char **azResult;
	int nrow, ncolumn, item;
	sqlite3 *db;
	char sql[512]={'\0'};
	char *zErrMsg = 0;
	int ack_flag = SUCCESS;
	char a;
	char timestamp[15] = {'\0'};
	strncpy(timestamp, &recvbuffer[34], 14);
	FILE *fp;
	fp = fopen("/etc/yuneng/encryption_type.conf", "w");
	if(fp){
		fputs("2",fp);
		fclose(fp);
	}
	fp = fopen("/etc/yuneng/encryption_type.conf", "r");
	if(fp){
		a = fgetc(fp);
		fclose(fp);
	}
	if(a!='2')ack_flag=FILE_ERROR;
	if(!sqlite3_open("/home/encryption.db",&db))
	{

		sprintf(sql,"SELECT * FROM key where item=1");
		sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		memset(sql,'\0',512);
		if(nrow==0)
			sprintf(sql,"insert into key (item,read_flag) values(1,1)");
		else
			sprintf(sql,"UPDATE key SET read_flag=1 WHERE item=1");
		sqlite3_exec(db, sql, 0, 0, &zErrMsg);
		perror(zErrMsg);
		sqlite3_close(db);
	}
	msg_ACK(sendbuffer, "A140", timestamp,ack_flag);
	return 0;

}

/*【A141】重置防盗状态*/
int set_encryption(const char *recvbuffer, char *sendbuffer)
{
	sqlite3 *db;
	int nrow, ncolumn,num,i;
	char **azResult = NULL;
	char *zErrMsg = 0;
	char sql[1024] = {'\0'};
	char yuid[13];
	char nuid[13];
	char password[128]={'\0'};
	int ack_flag = SUCCESS;
	char a;
	char timestamp[15] = {'\0'};
	int cmd,passwordlth,tmout;
	cmd = msg_get_int(&recvbuffer[30], 1);
	passwordlth = msg_get_int(&recvbuffer[31],2);
	strncpy(timestamp, &recvbuffer[33+passwordlth], 14);
	if(passwordlth>0)
		strncpy(password, &recvbuffer[33], passwordlth);

	////判断operator,后由主程序判断
//	FILE *fp;
//	fp = fopen("/etc/yuneng/encryption_type.conf", "w");
//	if(fp){
//		fputs("2",fp);
//		fclose(fp);
//	}
//	fp = fopen("/etc/yuneng/encryption_type.conf", "r");
//	if(fp){
//		a = fgetc(fp);
//		fclose(fp);
//	}
//	if(a!='2')ack_flag=FILE_ERROR;

	if(!sqlite3_open("/home/encryption.db",&db))
	{
//		if(cmd==1)
//		{
		if(passwordlth>0)	//需要设置密码
		{
			sprintf(sql,"SELECT * FROM key where item=1");
			sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
			memset(sql,'\0',1024);
			if(nrow==0)
				sprintf(sql,"INSERT INTO key (item,key,operator,cmd,set_flag,read_flag,set_time_flag) VALUES(1,'%s',1,1,1,0,0)",password);
			else
				sprintf(sql,"UPDATE key SET key='%s',operator=1,cmd=1,set_flag=1 WHERE item=1",password);
			sqlite3_exec(db, sql, 0, 0, &zErrMsg);
			perror(zErrMsg);
			sqlite3_close(db);
		}
//		}
//		else if(cmd==2)
//		{
//			sprintf(sql,"REPLACE INTO key (item,operator,cmd,set_flag,read_flag) VALUES(1,1,2,1,0)");
//			sqlite3_exec(db, sql , 0, 0, &zErrMsg);
//
//		}
//		else
//			ack_flag=CMD_ERROR;
//		num = msg_get_int(&recvbuffer[30], 4);
//		for(i=0;i<num;i++)
//		{
//			memset(sql,'\0',1024);
//			strncpy(yuid,&recvbuffer[51+28*i],12);
//			strncpy(nuid,&recvbuffer[64+28*i],12);
//			sprintf(sql,"DELETE FROM need_id WHERE wrongid='%s'",nuid);
//			sqlite3_exec(db, sql , 0, 0, &zErrMsg);
//			sprintf(sql,"INSERT INTO need_id (correct_id,wrongid,set_flag) VALUES('%s','%s',1)",yuid,nuid);
//			if(-1==insert_data(db, sql))
//				ack_flag=DB_ERROR;
//		}
		sqlite3_close(db);
	}
	else ack_flag=DB_ERROR;
	msg_ACK(sendbuffer, "A141", timestamp, ack_flag);
	return 0;
}

/*【A142】擦除防盗状态*/
int clear_encryption(const char *recvbuffer, char *sendbuffer)
{
	sqlite3 *db;
	int nrow, ncolumn,num,i;
	char **azResult = NULL;
	char *zErrMsg = 0;
	char sql[1024] = {'\0'};
	char yuid[13];
	char nuid[13];
	char password[128]={'\0'};
	int ack_flag = SUCCESS;
	char a;
	char timestamp[15] = {'\0'};
	int cmd,passwordlth;
	cmd = msg_get_int(&recvbuffer[30], 1);
	passwordlth = msg_get_int(&recvbuffer[31],2);
	strncpy(timestamp, &recvbuffer[33+passwordlth], 14);
	strncpy(password, &recvbuffer[33], passwordlth);

	//判断operator,后由主程序判断
//	FILE *fp;
//	fp = fopen("/etc/yuneng/encryption_type.conf", "w");
//	if(fp){
//		fputs("2",fp);
//		fclose(fp);
//	}
//	fp = fopen("/etc/yuneng/encryption_type.conf", "r");
//	if(fp){
//		a = fgetc(fp);
//		fclose(fp);
//	}
//	if(a!='2')ack_flag=FILE_ERROR;

	if(!sqlite3_open("/home/encryption.db",&db))
	{
//		if(cmd==1)
//		{
//			sprintf(sql,"REPLACE INTO key (item,key,operator,cmd,set_flag,read_flag) VALUES(1,'%s',1,1,1,0)",password);
//			sqlite3_exec(db, sql , 0, 0, &zErrMsg);
//		}
//		else if(cmd==2)
//		{
			sprintf(sql,"REPLACE INTO key (item,operator,cmd,set_flag,read_flag) VALUES(1,1,2,1,0)");
			sqlite3_exec(db, sql , 0, 0, &zErrMsg);
//
//		}
//		else
//			ack_flag=CMD_ERROR;
//		num = msg_get_int(&recvbuffer[30], 4);
//		for(i=0;i<num;i++)
//		{
//			memset(sql,'\0',1024);
//			strncpy(yuid,&recvbuffer[51+28*i],12);
//			strncpy(nuid,&recvbuffer[64+28*i],12);
//			sprintf(sql,"DELETE FROM need_id WHERE wrongid='%s'",nuid);
//			sqlite3_exec(db, sql , 0, 0, &zErrMsg);
//			sprintf(sql,"INSERT INTO need_id (correct_id,wrongid,set_flag) VALUES('%s','%s',1)",yuid,nuid);
//			if(-1==insert_data(db, sql))
//				ack_flag=DB_ERROR;
//		}
		sqlite3_close(db);
	}
	else ack_flag=DB_ERROR;
	msg_ACK(sendbuffer, "A142", timestamp, ack_flag);
	return 0;
}

/*【A143】读取逆变器防盗时间*/
int response_inverter_encryption_time(const char *recvbuffer, char *sendbuffer)
{
	char **azResult;
	int nrow, ncolumn, item;
	sqlite3 *db;
	char sql[512]={'\0'};
	char *zErrMsg = 0;
	int ack_flag = SUCCESS;
	char a;
	char timestamp[15] = {'\0'};
	strncpy(timestamp, &recvbuffer[34], 14);
	FILE *fp;
	fp = fopen("/etc/yuneng/encryption_type.conf", "w");
	if(fp){
		fputs("2",fp);
		fclose(fp);
	}
	fp = fopen("/etc/yuneng/encryption_type.conf", "r");
	if(fp){
		a = fgetc(fp);
		fclose(fp);
	}
	if(a!='2')ack_flag=FILE_ERROR;
	if(!sqlite3_open("/home/encryption.db",&db))
	{

		sprintf(sql,"SELECT * FROM key where item=1");
		sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		memset(sql,'\0',512);
		if(nrow==1)
			sprintf(sql,"UPDATE key SET read_time_flag=1 WHERE item=1");
		sqlite3_exec(db, sql, 0, 0, &zErrMsg);
		perror(zErrMsg);
		sqlite3_close(db);
	}
	msg_ACK(sendbuffer, "A140", timestamp,ack_flag);
	return 0;

}


/*【A144】设置防盗时间*/
int set_encryption_time(const char *recvbuffer, char *sendbuffer)
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
	int cmd,time_encryption,tmout;
	time_encryption = msg_get_int(&recvbuffer[30],4);
	strncpy(timestamp, &recvbuffer[34], 14);
	if(!sqlite3_open("/home/encryption.db",&db))
	{
		sprintf(sql,"SELECT * FROM key where item=1");
		sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		memset(sql,'\0',1024);
		if(nrow==1)
			sprintf(sql,"UPDATE key SET timeout=%d,operator=1,cmd=1,set_time_flag=1 WHERE item=1",time_encryption);
		sqlite3_exec(db, sql, 0, 0, &zErrMsg);
		perror(zErrMsg);
		sqlite3_close(db);
	}
	else ack_flag=DB_ERROR;
	msg_ACK(sendbuffer, "A144", timestamp, ack_flag);
	return 0;


}


