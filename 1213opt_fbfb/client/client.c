/*
 * client.c
 * V1.2.2
 * modified on: 2013-08-13
 * 与EMA异步通信
 * update操作数据库出现上锁时，延时再update
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <net/if.h>	//ifr
#include <netdb.h>
#include "sqlite3.h"

#define DAYS 100
sqlite3 *db;

void getcurrenttime(char db_time[])		//ECU上显示用的时间，与上面函数获取的时间只是格式区别，格式：年-月-日 时：分：秒，如2012-09-02 14：28：35
{
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};

	time(&tm);
	memcpy(&record_time,localtime(&tm), sizeof(record_time));
	sprintf(db_time, "%d-%02d-%02d %02d:%02d:%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday, record_time.tm_hour, record_time.tm_min, record_time.tm_sec);
}

int get_hour()			//获取当前是几点钟，只返回小时
{
	time_t tm;         //实例化time_t结构
	struct tm timenow;         //实例化tm结构指针

	time(&tm);
	memcpy(&timenow,localtime(&tm), sizeof(timenow));
	printdecmsg("Hour:", timenow.tm_hour);

	return timenow.tm_hour;
}

int writeconnecttime(void)			//保存最后一次连接上服务器的时间，在home页面上会显示
{
	char connecttime[20]={'\0'};
	FILE *fp;

	getcurrenttime(connecttime);
	fp = fopen("/etc/yuneng/connect_time.conf","w");
	if(fp)
	{
		fprintf(fp,"%s",connecttime);
		fclose(fp);
	}

	return 0;
}

void showconnected(void)		//已连接EMA，button程序从/tmp/connectemaflag.txt中间中读取，如果是connected，显示+W
{
	FILE *fp;

	fp = fopen("/tmp/connectemaflag.txt", "w");
	if(fp)
	{
		fputs("connected", fp);
		fclose(fp);
	}

}

void showdisconnected(void)		//无法连接EMA，button程序从/tmp/connectemaflag.txt中间中读取，如果是disconnected，显示-W
{
	FILE *fp;

	fp = fopen("/tmp/connectemaflag.txt", "w");
	if(fp)
	{
		fputs("disconnected", fp);
		fclose(fp);
	}

}

int randvalue(void)		//EMA有两个端口，ECU每次从两个端口中随即连接一个
{
	int i;

	srand((unsigned)time(NULL));
	i = rand()%2;
	printdecmsg("Randvalue", i);

	return i;
}

int createsocket(void)					//创建套接字
{
	int fd_sock;

	fd_sock=socket(AF_INET,SOCK_STREAM,0);
	if(-1==fd_sock)
		printmsg("Failed to create socket");
	else
		printmsg("Create socket successfully");

	return fd_sock;
}

int connect_socket(int fd_sock)				//连接到服务器，EMA的域名、IP、端口保存在/etc/yuneng/datacenter.conf文件中
{
	char domain[1024]={'\0'};		//EMA的域名
	char ip[20] = {'\0'};	//EMA的缺省IP
	int port[2]={8995, 8996}, i;
	char buff[1024] = {'\0'};
	struct sockaddr_in serv_addr;
	struct hostent * host;
	FILE *fp;

	fp = fopen("/etc/yuneng/datacenter.conf", "r");
	if(fp)
	{
		while(1)
		{
			memset(buff, '\0', sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if(!strlen(buff))
				break;
			if(!strncmp(buff, "Domain", 6))
			{
				strcpy(domain, &buff[7]);
				if('\n' == domain[strlen(domain)-1])
					domain[strlen(domain)-1] = '\0';
			}
			if(!strncmp(buff, "IP", 2))
			{
				strcpy(ip, &buff[3]);
				if('\n' == ip[strlen(ip)-1])
					ip[strlen(ip)-1] = '\0';
			}
			if(!strncmp(buff, "Port1", 5))
				port[0]=atoi(&buff[6]);
			if(!strncmp(buff, "Port2", 5))
				port[1]=atoi(&buff[6]);
		}
		fclose(fp);
	}

	if(!strlen(domain))
		strcpy(domain, "ecu.apsema.com");
	if(!strlen(ip))
		strcpy(ip, "60.190.131.190");


	host = gethostbyname(domain);
	if(NULL == host)
	{
		printmsg("Resolve domain failure");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		inet_ntop(AF_INET, *host->h_addr_list, ip, 32);
		printmsg(ip);
	}

	print2msg("IP", ip);
	printdecmsg("Port1", port[0]);
	printdecmsg("Port2", port[1]);

	bzero(&serv_addr,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port[randvalue()]);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	bzero(&(serv_addr.sin_zero),8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		showdisconnected();
		printmsg("Failed to connect to EMA");
		return -1;
	}
	else{
		showconnected();
		printmsg("Connect to EMA successfully");
		writeconnecttime();
		return 1;
	}
}

void close_socket(int fd_sock)					//关闭套接字
{
	close(fd_sock);
	printmsg("Close socket");
}

int sqlite3_exec_3times(sqlite3 *db, char *sql)		//数据库的封装，如果执行失败，最多执行3次，每次间隔100毫秒
{
	char *zErrMsg=0;
	int i;

	for(i=0; i<3; i++)
	{
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
			break;
		else
			print2msg(sql, zErrMsg);
		usleep(100000);
	}

	return 0;
}

int clear_date_time(char *date_time)		//把send_date_time设置为null，发送标志设置为1
{
	char sql[256]={'\0'};
	sqlite3 *db;

	sprintf(sql, "UPDATE fill_up_data SET send_flag=1,send_date_time=NULL WHERE send_date_time='%s'", date_time);
	sqlite3_exec_3times(db, sql);

	return 0;
}

int clear_send_flag(char *readbuff)		//已接收到EMA的答复，EMA答复中的时间对应的数据已经保存到服务器，ECU把这些数据标志置为0
{
	int i, j, count;		//EMA返回多少个时间（有几个时间，就说明EMA保存了多少条记录）
	char recv_date_time[16];
	char sql[1024]={'\0'};
	char *zErrMsg=0;

	if(strlen(readbuff) >= 3)
	{
		count = (readbuff[1] - 0x30) * 10 + (readbuff[2] - 0x30);
		if(count == (strlen(readbuff) - 3) / 14)
		{
			for(i=0; i<count; i++)
			{
				memset(sql,'\0',sizeof(sql));
				memset(recv_date_time, '\0', sizeof(recv_date_time));
				strncpy(recv_date_time, &readbuff[3+i*14], 14);
				sprintf(sql,"UPDATE Data SET resendflag='0' WHERE date_time='%s'", recv_date_time);
				sqlite3_exec_3times(db, sql);

				sprintf(sql, "UPDATE fill_up_data SET send_flag=0 WHERE send_date_time='%s'", recv_date_time);
				sqlite3_exec_3times(db, sql);
			}
		}
	}

	return 0;
}

int update_send_flag(char *send_date_time)		//当前一条数据发送至EMA，EMA返回结果，说明已收到，但还没有存入数据库，ECU把这条数据置为2
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	int i;

	memset(sql,'\0',sizeof(sql));
	sprintf(sql,"UPDATE Data SET resendflag='2' WHERE date_time='%s';", send_date_time);
	sqlite3_exec_3times(db, sql);

	memset(sql, '\0', sizeof(sql));
	sprintf(sql, "UPDATE fill_up_data SET send_flag=2 WHERE send_date_time='%s';", send_date_time);
	sqlite3_exec_3times(db, sql);

	return 0;
}

int recv_response(int fd_sock, char *readbuff)		//ECU接收EMA的应答，并且判断应答的长度是否正确
{
	fd_set rd;
	struct timeval timeout;
	int recvbytes, res, count=0, readbytes = 0;
	char recvbuff[65535];

	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(fd_sock, &rd);
		timeout.tv_sec = 120;
		timeout.tv_usec = 0;

		res = select(fd_sock+1, &rd, NULL, NULL, &timeout);
		if(res <= 0){
			printmsg("Receive data reply from EMA timeout");
			return -1;
		}
		else{
			sleep(1);
			memset(recvbuff, '\0', sizeof(recvbuff));
			recvbytes = recv(fd_sock, recvbuff, sizeof(recvbuff), 0);
			if(0 == recvbytes)
			{
				printmsg("Socket is closed by EMA");
				return -1;
			}
			strcat(readbuff, recvbuff);
			readbytes += recvbytes;
			if(readbytes >= 3)
			{
				print2msg("Reply", readbuff);
				count = (readbuff[1]-0x30)*10 + (readbuff[2]-0x30);
				if(count==((strlen(readbuff)-3)/14))
					return readbytes;
			}
		}
	}
}

int get_inverter_number()		//获取总的逆变器数量
{
	FILE *fp;
	char total[16]={'\0'};
	char sql[256]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn, count=0;
	sqlite3 *db;

	fp = fopen("/tmp/total_inverter.txt", "r");
	if(fp){
		fgets(total, sizeof(total), fp);
		fclose(fp);
		return atoi(total);
	}
	else{
		sprintf(sql,"SELECT COUNT(*) FROM id");
		sqlite3_open("/home/database.db", &db);
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
			if(nrow > 0){
				fp = fopen("/tmp/total_inverter.txt", "w");
				if(fp){
					count = atoi(azResult[1]);
					fprintf(fp, "%d", count);
					fclose(fp);
				}
			}
		}
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		return count;
	}
}

int add_lost_data(char *sendbuff, char *send_date_time)		//在正常读取到的数据后面，加上PLC补报上来的数据
{
	char sql[65535]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn, i, j, ret;
	char length[16]={'\0'};
	int powerin=0, powerout=0, energyin=0, energyout=0, total_powerin, total_powerout, total_energyin, total_energyout;
	char temp[256]={'\0'};
	int number, count;	//逆变器有number个，同一个时间点的帧数
	char date_time[16];
	int number_tmp=0;	//正常记录中逆变器的个数，需要加上同一个时间点的逆变器个数
	//char result[65535]={'\0'};

	if(strncmp(sendbuff, "APS16", 5) >= 0){
		memset(sql,'\0',sizeof(sql));
		sprintf(sql,"SELECT item,data FROM fill_up_data WHERE send_flag=1 AND lost_date_time='%s'", send_date_time);
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
			powerin=0;
			powerout=0;
			energyin=0;
			energyout=0;
			if(nrow > 0){
				sendbuff[strlen(sendbuff)-1] = '\0';
				for(i=1; i<=nrow; i++){
					strcat(sendbuff, azResult[i*ncolumn + 1]);
					strcat(sendbuff, "END");
					sprintf(sql, "UPDATE fill_up_data SET send_date_time='%s' WHERE item='%s'", send_date_time, azResult[i*ncolumn]);
					sqlite3_exec_3times(db, sql);

					memset(temp, '\0', sizeof(temp));
					strncpy(temp, azResult[i*ncolumn + 1]+24, 6);
					printdecmsg("atoi(temp)", atoi(temp));
					powerout += atoi(temp);

					memset(temp, '\0', sizeof(temp));
					strncpy(temp, azResult[i*ncolumn + 1]+56, 6);
					printdecmsg("atoi(temp)", atoi(temp));
					powerin += atoi(temp);
					memset(temp, '\0', sizeof(temp));
					strncpy(temp, azResult[i*ncolumn + 1]+85, 6);
					printdecmsg("atoi(temp)", atoi(temp));
					powerin += atoi(temp);

					memset(temp, '\0', sizeof(temp));
					strncpy(temp, azResult[i*ncolumn + 1]+30, 10);
					printdecmsg("atoi(temp)", atoi(temp));
					energyout += atoi(temp);

					memset(temp, '\0', sizeof(temp));
					strncpy(temp, azResult[i*ncolumn + 1]+62, 10);
					printdecmsg("atoi(temp)", atoi(temp));
					energyin += atoi(temp);
					memset(temp, '\0', sizeof(temp));
					strncpy(temp, azResult[i*ncolumn + 1]+91, 10);
					printdecmsg("atoi(temp)", atoi(temp));
					energyin += atoi(temp);

				}
				memset(temp, '\0', sizeof(temp));
				strncpy(temp, sendbuff+30, 10);
				total_powerin = atoi(temp) + powerin;
				memset(temp, '\0', sizeof(temp));
				sprintf(temp, "%010d", total_powerin);
				for(i=30; i<40; i++)
					sendbuff[i] = temp[i-30];

				memset(temp, '\0', sizeof(temp));
				strncpy(temp, sendbuff+40, 10);
				total_powerout = atoi(temp) + powerout;
				memset(temp, '\0', sizeof(temp));
				sprintf(temp, "%010d", total_powerout);
				for(i=40; i<50; i++)
					sendbuff[i] = temp[i-40];

				memset(temp, '\0', sizeof(temp));
				strncpy(temp, sendbuff+50, 10);
				total_energyin = atoi(temp) + energyin;
				memset(temp, '\0', sizeof(temp));
				sprintf(temp, "%010d", total_energyin);
				for(i=50; i<60; i++)
					sendbuff[i] = temp[i-50];

				memset(temp, '\0', sizeof(temp));
				strncpy(temp, sendbuff+60, 10);
				total_energyout = atoi(temp) + energyout;
				memset(temp, '\0', sizeof(temp));
				sprintf(temp, "%010d", total_energyout);
				for(i=60; i<70; i++)
					sendbuff[i] = temp[i-60];

				sprintf(length, "%05d", strlen(sendbuff));
				for(i=5; i<10; i++)
					sendbuff[i] = length[i-5];

				if('A' != sendbuff[84])
					number_tmp += (sendbuff[84] - 0x30) * 100;
				if('A' != sendbuff[85])
					number_tmp += (sendbuff[85] - 0x30) * 10;
				if('A' != sendbuff[86])
					number_tmp += (sendbuff[86] - 0x30);
				memset(temp, '\0', sizeof(temp));
				sprintf(temp, "%03d", number_tmp);
				for(i=84; i<87; i++)
					sendbuff[i] = temp[i-84];

				strcat(sendbuff, "\n");
			}
		}
		else
			print2msg(sql, zErrMsg);
		sqlite3_free_table( azResult );

		number = get_inverter_number();
printf("number: %d\n", number);

		if(number >0){
			memset(sql,'\0',sizeof(sql));
			if(number > 50){
				count = number*0.2;
				if(count > 200)	//补发200条上限
					count = 200;
			}
			else
				count = 20;
			sprintf(sql, "SELECT item,data,lost_date_time FROM fill_up_data WHERE send_flag=1 AND "
					"lost_date_time<>'%s' AND lost_date_time>'20100101' AND "
					"lost_date_time IN (SELECT DISTINCT date_time FROM Data WHERE resendflag='0' AND date_time>'20100101') "
					"ORDER BY lost_date_time DESC LIMIT 0,%d", send_date_time, count);

			if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
				if(nrow > 0){
					sendbuff[strlen(sendbuff)-1] = '\0';
					strcat(sendbuff, "ADD");

					for(i=1; i<=nrow;){
						count=1;
						if(i<nrow){
							for(j=i+1; j<=nrow; j++){
								if(!strcmp(azResult[i*ncolumn + 2], azResult[j*ncolumn + 2]))
									count++;
								else
									break;
							}
						}

						strcat(sendbuff, azResult[i*ncolumn + 2]);
						sprintf(temp, "%03d", count);
						strcat(sendbuff, temp);
						strcat(sendbuff, "UND");
						for(j=1; j<=count; j++, i++){
							strcat(sendbuff, azResult[i*ncolumn + 1]);
							strcat(sendbuff, "UND");
							memset(sql,'\0',sizeof(sql));
							sprintf(sql, "UPDATE fill_up_data SET send_date_time='%s' WHERE item=%d", send_date_time, atoi(azResult[i*ncolumn]));

							sqlite3_exec_3times(db, sql);
						}
						strcat(sendbuff, "END");
					}
					sprintf(length, "%05d", strlen(sendbuff));
					for(i=5; i<10; i++)
						sendbuff[i] = length[i-5];
					strcat(sendbuff, "\n");
				}
			}
			else
				print2msg(sql, zErrMsg);
			sqlite3_free_table( azResult );
		}
	}
}

int fill_up_data()		//PLC补发的数据（单路）超过逆变器总数的12倍时，PLC补发的数据单独发送给EMA，不跟在正常轮的数据中。
{
	char sql[65535]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn, number, i, j, count=0,sendMaxCount=0, res;
	char sendbuff[65535]={'\0'};
	char temp[256]={'\0'};
//	struct timeval send_time;
	time_t tm;
	struct tm record_time;    //记录时间
	FILE *fp;
	char ecu_id[16]={'\0'};
	char send_date_time[32]={'\0'};
	char length[16]={'\0'};
	int fd_sock;

	strcpy(sql, "SELECT COUNT(*) FROM fill_up_data WHERE send_flag=1");
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
		count = atoi(azResult[1]);
	sqlite3_free_table( azResult );

	number = get_inverter_number();
	printf("number: %d, count: %d\n", number, count);

	if((number >0)){
		fp = fopen("/etc/yuneng/ecuid.conf", "r");
		if(fp)
		{
			fgets(ecu_id, 13, fp);
			fclose(fp);
		}
		print2msg("ECU ID", ecu_id);

		fd_sock = createsocket();

		memset(sql, '\0', sizeof(sql));
		if (number > 100) {
			sendMaxCount = number * 0.2;
			if (sendMaxCount > 100)	//补发200条上限
				sendMaxCount = 100;
		} else
			sendMaxCount = 20;

		if(1 == connect_socket(fd_sock)){
			while(1){
				sprintf(sql, "SELECT item,data,lost_date_time FROM fill_up_data WHERE send_flag=1 AND "
						"lost_date_time>'20100101' AND "
						"lost_date_time IN (SELECT DISTINCT date_time FROM Data WHERE resendflag='0' AND date_time>'20100101') "
						"ORDER BY lost_date_time DESC LIMIT 0,%d", sendMaxCount);
				if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg )){
					if(nrow > 0){
						time(&tm);
						memcpy(&record_time, localtime(&tm), sizeof(record_time));
						//年份故意加了1000
						sprintf(send_date_time, "%04d%02d%02d%02d%02d%02d", record_time.tm_year+2900, record_time.tm_mon+1, record_time.tm_mday, record_time.tm_hour, record_time.tm_min, record_time.tm_sec);
						print2msg("ECU ID", ecu_id);
						sprintf(sendbuff, "APS160000000010001%12s%14sENDADD", ecu_id, send_date_time);
						for(i=1; i<=nrow;){
							count=1;
							if(i<nrow){
								for(j=i+1; j<=nrow; j++){
									if(!strcmp(azResult[i*ncolumn + 2], azResult[j*ncolumn + 2]))
										count++;
									else
										break;
								}
							}

							strcat(sendbuff, azResult[i*ncolumn + 2]);
							sprintf(temp, "%03d", count);
							strcat(sendbuff, temp);
							strcat(sendbuff, "UND");
							for(j=1; j<=count; j++, i++){
								strcat(sendbuff, azResult[i*ncolumn + 1]);
								strcat(sendbuff, "UND");
								sprintf(sql, "UPDATE fill_up_data SET send_date_time='%s' WHERE item=%d", send_date_time, atoi(azResult[i*ncolumn]));
								//strcpy(sql, "UPDATE fill_up_data SET send_date_time=NULL");
								sqlite3_exec_3times(db, sql);
							}
							strcat(sendbuff, "END");
						}
						sprintf(length, "%05d", strlen(sendbuff));
						for(i=5; i<10; i++)
							sendbuff[i] = length[i-5];
						strcat(sendbuff, "\n");

						print2msg("sendbuff", sendbuff);

						sqlite3_free_table( azResult );

						res = send_record_lost_only(fd_sock, sendbuff, send_date_time);
						if(-1 == res)
							break;

					}
					else{
						sqlite3_free_table( azResult );
						break;
					}
				}
				else{
					print2msg(sql, zErrMsg);
					sqlite3_free_table( azResult );
					break;
				}
			}
		}
		close_socket(fd_sock);
	}
}

int getdbsize(void){			//获取数据库的文件大小，如果数据库超过30M后，每插入一条新的数据，就删除数据库中最早的数据
	struct stat fileinfo;
	int filesize=0;

	stat("/home/record.db", &fileinfo);
	filesize=fileinfo.st_size;//filesize.st_size/1024;

	printdecmsg("Size of database", filesize);

	if(filesize >= 30000000)
		return 1;
	else
		return 0;
}

void save_record(char *data, char *date_time)			//ECU发送记录给EMA的同时，本地也保存一份
{
	char sql[65535]={'\0'};
	sqlite3 *db;

	sqlite3_open("/home/record.db", &db);
	if(strlen(data)>0) {
		if(1 == getdbsize()) {
			memset(sql, '\0', sizeof(sql));
			strcpy(sql, "DELETE FROM Data WHERE rowid=(SELECT MIN(rowid) FROM Data)");
			sqlite3_exec_3times(db , sql);
		}

		memset(sql, '\0', sizeof(sql));
		sprintf(sql, "INSERT INTO Data (item, record, resendflag, date_time) "
				"VALUES(%d , '%s', '1', '%s');", 1, data, date_time);
		sqlite3_exec_3times(db, sql);
	}
	sqlite3_close(db);
}

int delete_fill_up_data(char *time)
{
	char sql[1024]={'\0'};
	sqlite3 *db;

	sqlite3_open("/home/record.db", &db);
	sprintf(sql, "DELETE FROM fill_up_data WHERE lost_date_time='%s'", time);
	sqlite3_exec_3times(db, sql);
	sqlite3_close(db);

	return 0;
}

int change_head()	//实时数据的时间点不存在，EMA无法保存该时间点补上来的数据，需要把时间点转换成实时数据的协议头
{
	char sql[1024]={'\0'};
	char body[65535]={'\0'};
	char buff[65535]={'\0'};
	char temp[16]={'\0'};
	char ecu_id[16]={'\0'};
	char zone[50] = {'\0'};
	char *zErrMsg=0;
	char **azResult;
	int i, nrow, ncolumn;
	int power=0, energy=0, count=0; //暂用energy为优化器的输入功率
	int zoneflag = 0;
	sqlite3 *db;
	FILE *fp;

	fp = fopen("/etc/yuneng/ecuid.conf", "r");
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	fp = fopen("/etc/yuneng/timezone.conf", "r");
	if(fp)
	{
		fgets(zone, 50, fp);
		fclose(fp);

		if(!strncmp(zone, "Asia/Shanghai", 13))
			zoneflag = 0;
		else
			zoneflag = 1;
	}

	sqlite3_open("/home/record.db", &db);
	strcpy(sql, "SELECT data,lost_date_time FROM fill_up_data WHERE send_flag=1 AND "
			"lost_date_time NOT IN (SELECT DISTINCT date_time FROM Data)");
	if(SQLITE_OK != sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
		print2msg("Select", zErrMsg);
	else
	{
		for(i=1; i<=nrow; i++)
		{
			strcat(body, azResult[i*ncolumn]);
			strcat(body, "END");
			strncpy(temp, &azResult[i*ncolumn][24], 6);
			power += atoi(temp);
			strncpy(temp, &azResult[i*ncolumn][56], 6);
			energy += atoi(temp);
			strncpy(temp, &azResult[i*ncolumn][85], 6);
			energy += atoi(temp);
			count++;
			if((i==nrow) || (strcmp(azResult[i*ncolumn+1], azResult[(i+1)*ncolumn+1])))		//时间不同，保存数据，初始化变量
			{
				sprintf(buff, "APS16%05d00010001%s%010d%010d00000000000000000000%s%03d%1d00000END", 96+101*count, ecu_id, power, energy,
							azResult[i*ncolumn+1], count, zoneflag);
				strcat(buff, body);
				strcat(buff, "\n");
				save_record(buff, azResult[i*ncolumn+1]);
				delete_fill_up_data(azResult[i*ncolumn+1]);

				memset(body, '\0', sizeof(body));
				memset(buff, '\0', sizeof(buff));
				power = 0;
				energy = 0;
				count = 0;
			}
		}
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	return 0;
}

int send_record(int fd_sock, char *sendbuff, char *send_date_time)			//发送数据到EMA
{
	int sendbytes=0, res=0;
	char readbuff[65535] = {'\0'};
	char lost_data[65535]={'\0'};
	char same_time_data[65535]={'\0'};

	add_lost_data(sendbuff, send_date_time);
	print2msg("data", sendbuff);

	sendbytes = send(fd_sock, sendbuff, strlen(sendbuff), 0);

	if(-1 == sendbytes){
		//clear_date_time(send_date_time);
		printmsg("Failed to send data");
		return -1;
	}

	if(-1 == recv_response(fd_sock, readbuff))
		return -1;
	else
	{
		if('1' == readbuff[0])
			update_send_flag(send_date_time);
		clear_send_flag(readbuff);
		return 0;
	}

}

int send_record_lost_only(int fd_sock, char *sendbuff, char *send_date_time)			//发送PLC补发的数据到EMA
{
	int sendbytes=0, res=0;
	char readbuff[65535] = {'\0'};
	char lost_data[65535]={'\0'};
	char same_time_data[65535]={'\0'};

	sendbytes = send(fd_sock, sendbuff, strlen(sendbuff), 0);

	if(-1 == sendbytes){
		return -1;
	}

	if(-1 == recv_response(fd_sock, readbuff))
		return -1;
	else
	{
		if('1' == readbuff[0])
			update_send_flag(send_date_time);
		clear_send_flag(readbuff);
		return 0;
	}

}

int preprocess()			//晚上单独向EMA获取一次信息，检查是否存在EMA已经存入服务器但是没有应答ECU的数据。发送头信息到EMA，读取已经已经存入EMA的记录的时间
{
	int sendbytes=0, res=0, recvbytes = 0;
	char readbuff[65535] = {'\0'};
	fd_set rd;
	struct timeval timeout;
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char sendbuff[256] = {'\0'};
	FILE *fp;
	char **azResult;
	int nrow, ncolumn;
	int fd_sock;
	int count=0, count2=0;

	strcpy(sql,"SELECT date_time FROM Data WHERE resendflag='2'");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	print2msg("Select nrow of resendflag=2 from db", zErrMsg);
	sqlite3_free_table( azResult );
	count = nrow;

	strcpy(sql,"SELECT item FROM fill_up_data WHERE send_flag=2");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	print2msg("Select nrow of resendflag=2 from db", zErrMsg);
	sqlite3_free_table( azResult );
	count2 = nrow;

	if((count < 1) && (count2 < 1))
		return 0;

	strcpy(sendbuff, "APS13AAA22");
	fp = fopen("/etc/yuneng/ecuid.conf", "r");
	if(fp)
	{
		fgets(&sendbuff[10], 13, fp);
		fclose(fp);
	}
	strcat(sendbuff, "\n");
	print2msg("Sendbuff", sendbuff);

	fd_sock = createsocket();
	printdecmsg("Socket", fd_sock);
	if(1 == connect_socket(fd_sock))
	{
		while(1)
		{
			memset(readbuff, '\0', sizeof(readbuff));
			sendbytes = send(fd_sock, sendbuff, strlen(sendbuff), 0);
			if(-1 == sendbytes)
			{
				close_socket(fd_sock);
				return -1;
			}
			if(recv_response(fd_sock, readbuff) > 3)
				clear_send_flag(readbuff);
			else
				break;
		}
	}

	close_socket(fd_sock);
	return 0;
}

int resend_record()		//把没有接收到EMA返回的记录设置为1，让ECU接下去重新发送
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int i, res, nrow, ncolumn;
	int fd_sock;

	strcpy(sql, "UPDATE fill_up_data SET send_flag=1 WHERE send_flag=2");
	sqlite3_exec_3times(db, sql);

	strcpy(sql, "UPDATE Data SET resendflag='1' WHERE resendflag='2'");
	sqlite3_exec_3times(db, sql);

	return 0;
}

int main(int argc, char *argv[])
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow, ncolumn, res, i;
	int fd_sock;
	int thistime=0, lasttime=0;
	char data[65535];
	char date_time[32];
	char last_day[16];		//没有上传数据的最后一天的日期
	char earlist_day_last_time[16];		//没有上传数据的最后一天的日期
	char date[DAYS][10];
	int nday, nrecord, j;		//天数，发送的条数

	printmsg("Started");

	while(1)
	{
//		thistime = lasttime = time(NULL);
		for(i=0; i<DAYS; i++)
			memset(date[i], '\0', sizeof(date[i]));
		nrecord = 1;
		nrow = 0;
		ncolumn = 0;

		lasttime = time(NULL);
		sqlite3_open("/home/record.db", &db);

		if(1 == get_hour())
		{
			preprocess();		//预处理，先发送一个头信息给EMA，让EMA把标记为2的记录的时间返回给ECU，然后ECU再把标记为2的记录再次发给EMA，防止EMA收到记录返回收到标记而没有存入数据库的情况
			resend_record();
			fill_up_data();
			change_head();
		}

		strcpy(sql, "SELECT record,date_time FROM Data WHERE resendflag='1' AND date_time>'20100101' ORDER BY date_time ASC");
		if(SQLITE_OK != sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
			print2msg("Select result from db", zErrMsg);
		else
			printdecmsg("Record count", nrow);

		if(nrow >= 1)
		{
			fd_sock = createsocket();
			if(1 == connect_socket(fd_sock))
			{
				for(i=nrow, nday=0; (i>=1)&&(nday<DAYS); i--)
				{
					if(0 == nday)
					{
						strncpy(date[nday], azResult[i*ncolumn+1], 8);
						nday++;
					}
					else
					{
						if(strncmp(date[nday-1], azResult[i*ncolumn+1], 8))
						{
							strncpy(date[nday], azResult[i*ncolumn+1], 8);
							nday++;
						}
					}
				}

				for(i=0; i<nday; i++)
				{
					printmsg(date[i]);
				}
				printdecmsg("nday", nday);
				printdecmsg("nrow", nrow);
				printdecmsg("Socket", fd_sock);

				for(j=0; j<nday; j++)
				{
					for(i=1; i<=nrow; i++)
					{
						if(!strncmp(date[j], azResult[i*ncolumn+1], 8))
						{
							memset(data, '\0', sizeof(data));
							strcpy(data, azResult[i*ncolumn]);
							strcpy(date_time, azResult[i*ncolumn+1]);
							if(nrecord != nrow)
								data[88] = '1';
							//printmsg(azResult[i*ncolumn]);
							res = send_record(fd_sock, data, date_time);
							nrecord++;
							if(-1 == res)
								break;
						}
					}
					if(-1 == res)
						break;
				}
			}
			close_socket(fd_sock);
		}
		else{
			printmsg("There are none record");
		}
		sqlite3_free_table( azResult );
		sqlite3_close(db);
		thistime = time(NULL);
		if(thistime-lasttime < 300)
			sleep(300 + lasttime - thistime);
	}

	return 0;
}
