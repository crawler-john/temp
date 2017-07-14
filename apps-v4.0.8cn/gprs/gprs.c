/*
 * client.c
 * V1.2.2
 * modified on: 2013-08-13
 * 与EMA异步通信
 * update操作数据库出现上锁时，延时再update
 */

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include "sqlite3.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyO1"
#define _POSIX_SOURCE 1
#define MAXSIZE 56000
#define sendsize 800

//#define DEBUG
//#define DEBUGLOG

void printmsg(char *msg)		//打印字符串
{
#ifdef DEBUG
	printf("Client: %s!\n", msg);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/client.log", "a");
	if(fp)
	{
		fprintf(fp, "Client: %s!\n", msg);
		fclose(fp);
	}
#endif
}

void print2msg(char *msg1, char *msg2)		//打印字符串
{
#ifdef DEBUG
	printf("Client: %s, %s!\n", msg1, msg2);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/client.log", "a");
	if(fp)
	{
		fprintf(fp, "Client: %s, %s!\n", msg1, msg2);
		fclose(fp);
	}
#endif
}

void printdecmsg(char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	printf("Client: %s: %d!\n", msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/client.log", "a");
	if(fp)
	{
		fprintf(fp, "Client: %s: %d!\n", msg, data);
		fclose(fp);
	}
#endif
}

void printhexmsg(char *msg, char *data, int size)		//打印十六进制数据
{
#ifdef DEBUG
	int i;

	printf("Client: %s: ", msg);
	for(i=0; i<size; i++)
		printf("%X, ", data[i]);
	printf("\n");
#endif
#ifdef DEBUGLOG
	FILE *fp;
	int j;

	fp = fopen("/home/client.log", "a");
	if(fp)
	{
		fprintf(fp, "Client: %s: ", msg);
		for(j=0; j<size; j++)
			fprintf(fp, "%X, ", data[j]);
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif
}

int opengprs(void)		//打开串口
{
	int fd, res, i=0;
	char buff[MAXSIZE]={'\0'};
	struct termios newtio;

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);		//打开串口
	if(fd<0){
		perror("MODEMDEVICE");
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 140;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	return fd;
}

int closegprs(int fd)		//关闭串口
{
	close(fd);

	return 0;
}

int gprssendrecord(int fd, char *sendbuff)		//使用GPRS发送数据，即往串口中写数据，模块会自己发送
{
	int res, i, j, sendtimes=0, recvbytes;
	char readbuff[255]={'\0'};
	fd_set rd;
	struct timeval timeout;

	printdecmsg(sendbuff, strlen(sendbuff));

	sendtimes=strlen(sendbuff)/sendsize;
	if((strlen(sendbuff)%sendsize)>0){
		sendtimes++;
	}

	printdecmsg("sendtimes", sendtimes);

	for(j=0; j<3; j++){
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		timeout.tv_sec = 20;
		timeout.tv_usec = 0;

		for(i=1; i<=sendtimes; i++){
			if(i == sendtimes)
				write(fd, sendbuff+(i-1)*800, strlen(sendbuff+(i-1)*800));
			else{
				write(fd, sendbuff+(i-1)*800, sendsize);
				sleep(5);
			}
		}

		res = select(fd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printdecmsg("res", res);
			continue;
		}
		else{
			if(FD_ISSET(fd, &rd)){
				sleep(1);
				recvbytes = read(fd, readbuff, sizeof(readbuff));
				print2msg("readbuff", readbuff);

				if(!strncmp(readbuff, &sendbuff[60], 14))
					return 1;
				else
					continue;
			}
		}
	}

	return 0;
}

int  main(int argc, char *argv[])
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	int i, fd;
	char flag[2];
	FILE *fp;
	sqlite3 *db;

	while(1){
		fp = fopen("/etc/yuneng/gprs.conf", "r");
		if(fp){
			fgets(flag, sizeof(flag), fp);
			close(fp);
			if('1' == flag[0])
				break;
		}
		sleep(86400);
	}

	while(1){
		nrow = 0;
		if(SQLITE_OK == sqlite3_open("/home/record.db", &db)){
			strcpy(sql,"SELECT record,date_time FROM Data WHERE resendflag='1' ORDER BY date_time DESC LIMIT 0,15");
			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
				if(nrow > 0){
					fd = opengprs();
					printdecmsg("nrow", nrow);
					for(i=1; i<=nrow; i++){
						if(1 != gprssendrecord(fd, azResult[i*ncolumn]))
							break;
						else{
							memset(sql, '\0', sizeof(sql));
							sprintf(sql,"UPDATE Data SET resendflag='0' WHERE date_time='%s'", azResult[i*ncolumn+1]);
							sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
						}
					}
					closegprs(fd);
				}
			}
			else
				print2msg("Failed to query record", zErrMsg);

			sqlite3_free_table( azResult );
			sqlite3_close(db);
		}
		else
			print2msg("Failed to open record.db", zErrMsg);

		//if(nrow < 15)
		sleep(300);
	}

	return 0;
}
