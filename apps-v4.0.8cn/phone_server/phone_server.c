#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <pthread.h>
#include "sqlite3.h"

#define SERVERPORT 4560		//服务器监听端口号
#define BACKLOG 5			//同时最大连接请求数

//#define DEBUG
//#define DEBUGLOG

int check_ecuid(char *buff)
{
	FILE *fp;int i;
	char buf[13]={'\0'};
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	for(i=0;i<12;i++)
	{
		if(buf[i]!=buff[i+15])
			return -1;
	}
	return 1;
}


void printmsg(char *msg)	//打印字符串
{
#ifdef DEBUG
	printf("phone_server: %s!\n", msg);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/phone_server.log", "a");
	if(fp)
	{
		fprintf(fp, "phone_server: %s!\n", msg);
		fclose(fp);
	}
#endif
}

void print2msg(char *msg1, char *msg2)		//打印两个字符串
{
#ifdef DEBUG
	printf("phone_server: %s: %s!\n", msg1, msg2);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/phone_server.log", "a");
	if(fp)
	{
		fprintf(fp, "phone_server: %s: %s!\n", msg1, msg2);
		fclose(fp);
	}
#endif
}

void printdecmsg(char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	printf("phone_server: %s: %d!\n", msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/phone_server.log", "a");
	if(fp)
	{
		fprintf(fp, "phone_server: %s: %d!\n", msg, data);
		fclose(fp);
	}
#endif
}

void printhexmsg(char *msg, char *data, int size)		//打印十六进制数据
{
#ifdef DEBUG
	int i;

	printf("phone_server: %s: ", msg);
	for(i=0; i<size; i++)
		printf("%X, ", data[i]);
	printf("\n");
#endif
#ifdef DEBUGLOG
	FILE *fp;
	int j;

	fp = fopen("/home/phone_server.log", "a");
	if(fp)
	{
		fprintf(fp, "phone_server: %s: ", msg);
		for(j=0; j<size; j++)
			fprintf(fp, "%X, ", data[j]);
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif
}

int response_nomatch_ecuid(int fd_sock, char *date, char *head)
{

	FILE *fp;
	char buf[50]={'\0'};
	char record[65535]={'\0'};
	int i, j, ret,num;

	printmsg("ecuid ERROR: start");
	//消息头
	strcat(record,head);
	//获取ID号
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	fp=NULL;
	strcat(record,buf);		//head+ecuid...
	strcat(record,date);	//head+12+14..(长度未改)
	strcat(record,"9END");
//	system("killall main.exe");
	memset(buf, '\0', sizeof(buf));
	sprintf(buf,"%05d", strlen(record));
	strncpy(&record[5],buf,5);
	print2msg("Send",record);
	strcat(record,"\n");
	for(i=0;i<3;i++){
		if(-1!=send(fd_sock,record,strlen(record),0)){
			printmsg("response_ecuid_error: end");
			return 0;
		}
	}
	printmsg("response_ecuid_error: send failed");
	return -1;
}

void printtime(void)		//打印时间
{
	time_t tm;
	struct tm record_time;
	char curtime[20]={'\0'};

	time(&tm);
	memcpy(&record_time,localtime(&tm), sizeof(record_time));
	sprintf(curtime,
		"%04d-%02d-%02d %02d:%02d:%02d",
		record_time.tm_year+1900,
		record_time.tm_mon+1,
		record_time.tm_mday,
		record_time.tm_hour,
		record_time.tm_min,
		record_time.tm_sec);
	print2msg("system time", curtime);
}

void printtime2(int kind, int *p, unsigned int pid)		//打印时间
{
	time_t tm;
	struct tm record_time;
	struct timeval utime;
	char curtime[20]={'\0'};

	gettimeofday(&utime,NULL);
	time(&tm);
	memcpy(&record_time,localtime(&tm), sizeof(record_time));
	sprintf(curtime,
		"%d->%04d-%02d-%02d %02d:%02d:%02d:%u %p pid= %u",
		kind,
		record_time.tm_year+1900,
		record_time.tm_mon+1,
		record_time.tm_mday,
		record_time.tm_hour,
		record_time.tm_min,
		record_time.tm_sec,
		utime.tv_usec,
		p,
		pid);
	print2msg("system time2", curtime);
}

int create_socket(void)
{
	int sockfd;
	if(-1==(sockfd=socket(AF_INET,SOCK_STREAM,0))){
		perror("socket");
		exit(1);
	}
	printdecmsg("Create socket successfully!  socket",sockfd);
	return sockfd;
}

void bind_socket(int sockfd)
{
	int on;
	struct sockaddr_in server_sockaddr;
	//允许端口立即重用
	on = 1;
	if(0!=setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))){
		perror("bind");
	}
	//填充sockaddr结构
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(SERVERPORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero),8);
	if(-1==bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))){
		perror("bind");
		exit(1);
	}
	printmsg("Bind socket successfully");
}

void listen_socket(int sockfd)
{
	if(-1==listen(sockfd,BACKLOG)){
		perror("listen");
		exit(1);
	}
	printmsg("Listen socket successfully");
}

int accept_socket(int sockfd)
{
	int sin_size;
	int clientfd;
	struct sockaddr_in client_sockaddr;
	char buf[50]={'\0'};

	sin_size=sizeof(struct sockaddr_in);
	if(-1==(clientfd=accept(sockfd,(struct sockaddr *)&client_sockaddr,&sin_size))){
		perror("accept");
		return -1;
	}
	sprintf(buf,"Got connection from  %s:%d  client:%d",inet_ntoa(client_sockaddr.sin_addr),ntohs(client_sockaddr.sin_port),clientfd);
	printmsg(buf);
	printtime();
	return clientfd;
}

//日期加减函数，返回一个8位整数
int change_date(char *date, int days)
{
	char year[5]={'\0'}; 
	char month[3]={'\0'};
	char day[3]={'\0'};
	int new_date;
	time_t times;
	struct tm time;
	struct tm *tm_p;

	strncpy(year,date,4);	
	strncpy(month,&date[4],2);
	strncpy(day,&date[6],2);	
	time.tm_year = atoi(year)-1900;
	time.tm_mon = atoi(month)-1;
	time.tm_mday = atoi(day);
	time.tm_hour = 12;
	time.tm_min = 0;
	time.tm_sec = 0;
	if(-1==(times = mktime(&time))){
		return 20380118;	
	}
	times += days*3600*24;
	tm_p = localtime(&times);
	new_date = (tm_p->tm_year+1900)*10000+(tm_p->tm_mon+1)*100+tm_p->tm_mday;
	return new_date;
}

int response_ecu_info(int fd_sock, char *head)
{
	FILE *fp;
	char sql[100]={'\0'};
	char buf[50]={'\0'};
	char record[65535]={'\0'};
	char length[4]={'\0'};
	sqlite3 *db;
	char **azResult;
	int nrow, ncolumn;
	char *zErrMsg = 0;
	int i, ret;
	
	printmsg("response_ecu_info: start");
	//消息头
	strcat(record,head);
	//获取ID号
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){	
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	fp=NULL;
	strcat(record,buf);
//	print2msg("ECU_ID",buf);
	//获取版本号
	if(NULL==(fp = fopen("/etc/yuneng/version.conf", "r"))){	
		perror("version.conf");
		return -1;
	}
	fgets(buf, 50, fp);
	if(10==buf[strlen(buf)-1]){
		//删除换行符号
		buf[strlen(buf)-1]='\0';
	}
	fclose(fp);
	fp=NULL;
	//计算版本号长度
	memset(length, '\0', sizeof(length));
	sprintf(length,"%03d", strlen(buf));
	strcat(record,length);
	strcat(record,buf);
//	print2msg("ECU_Version",buf);
	//获取时区信息
	if(NULL==(fp = fopen("/etc/yuneng/timezone.conf", "r"))){	
		perror("timezone.conf");
		return -1;
	}
	fgets(buf, 50, fp);
	if(10==buf[strlen(buf)-1]){
		//删除换行符号
		buf[strlen(buf)-1]='\0';
	}
	fclose(fp);
	fp=NULL;
	//计算时区信息长度
	memset(length, '\0', sizeof(length));
	sprintf(length,"%03d", strlen(buf));
	strcat(record,length);
	strcat(record,buf);
//	print2msg("ECU_TimeZone",buf);
	//获取逆变器数量	
	if(sqlite3_open("/home/database.db",&db)){					
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	memset(sql, '\0', sizeof(sql));	
	strcpy(sql, "SELECT COUNT(id) FROM id ");
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}	
	if(!nrow){
		strcat(record,"0000");
		memset(buf, '\0', sizeof(buf));	
	}
	else{
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%04d",atoi(azResult[1]));
		strcat(record,buf);		
	}
//	print2msg("Inverter_Num",buf);
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}
	sqlite3_close(db);
	strcat(record,"END");
	memset(buf, '\0', sizeof(buf));
	sprintf(buf,"%05d", strlen(record));
	strncpy(&record[5],buf,5);
	print2msg("Send",record);
	strcat(record,"\n");
	for(i=0;i<3;i++){
		if(-1!=send(fd_sock,record,strlen(record),0)){
			printmsg("response_ecu_info: end");
			return 0;
		}
	}
	printmsg("response_ecu_info: send failed");
	return -1;
}

int response_system_info(int fd_sock, char *date, char *head)
{
	FILE *fp;
	char sql[100]={'\0'};
	char buf[50]={'\0'};
	char record[65535]={'\0'};
	sqlite3 *db;
	char **azResult;
	int nrow, ncolumn;
	char *zErrMsg = 0;
	int i, ret;

	printmsg("response_system_info: start");
	//消息头
	strcat(record,head);
	//获取ID号
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){	
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	fp=NULL;
	strcat(record,buf);
//	print2msg("ECU_ID",buf);
	//获取系统总功率
	if(NULL==(fp = fopen("/tmp/system_p_display.conf", "r"))){	
		perror("system_p_display.conf");
		return -1;
	}
	fgets(buf, 50, fp);
	if(10==buf[strlen(buf)-1]){
		//删除换行符号
		buf[strlen(buf)-1]='\0';
	}
	fclose(fp);
	fp=NULL;
	char power[50]={'\0'};
	sprintf(power,"%010d",(int)(100*atof(buf)+0.5));
	strcat(record,power);
//	print2msg("ECU_Sys_Total_Power",power);
	//打开历史数据库
	if(sqlite3_open("/home/historical_data.db",&db)){					
		fprintf(stderr, "Can't open historical_data: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	//获取历史发电量
	memset(sql, '\0', sizeof(sql));	
	strcpy(sql, "SELECT lifetime_energy FROM lifetime_energy ");
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}	
	if(!nrow){
		strcat(record,"0000000000");
		memset(buf, '\0', sizeof(buf));	
	}
	else{
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%010d",(int)(100*atof(azResult[1])+0.5));
		strcat(record,buf);		
	}
//	print2msg("ECU_Sys_Lifetime_Energy",buf);
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}
	//最近一周发电量
	memset(sql, '\0', sizeof(sql));	
	sprintf(sql, "SELECT SUM(daily_energy) FROM daily_energy WHERE date BETWEEN '%d' AND '%s' ORDER BY date" ,change_date(date,-6),date);				
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}
	if(NULL==azResult[1]){
		strcat(record,"0000000000");
		memset(buf, '\0', sizeof(buf));	
	}
	else{
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%010d",(int)(100*atof(azResult[1])+0.5));
		strcat(record,buf);
	}
//	print2msg("ECU_Sys_LastWeek_Energy",buf);
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}
	//当月发电量
	memset(buf, '\0', sizeof(buf));	
	strncpy(buf,date,6);
	memset(sql, '\0', sizeof(sql));	
	sprintf(sql, "SELECT monthly_energy FROM monthly_energy WHERE date='%s' ",buf);						
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}
	if(!nrow){
		strcat(record,"0000000000");
		memset(buf, '\0', sizeof(buf));	
	}
	else{
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%010d",(int)(100*atof(azResult[1])+0.5));
		strcat(record,buf);
	}
//	print2msg("ECU_Sys_LastMonth_Energy",buf);
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}
	//获取最近一次轮询时间
	memset(sql, '\0', sizeof(sql));	
	strcpy(sql, "SELECT date,time FROM each_system_power ORDER BY date DESC,time DESC LIMIT 0,1");					
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}
	if(!nrow){
		strcat(record,"00000000000000");
		memset(buf, '\0', sizeof(buf));	
	}
	else{
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%s%s",azResult[2],azResult[3]);
		strcat(record,buf);
	}
//	print2msg("ECU_Send_Time",buf);
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}

	//ECU当天能量
	memset(sql, '\0', sizeof(sql));	
	sprintf(sql, "SELECT daily_energy FROM daily_energy WHERE date='%s' ", date);
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}	
	if(!nrow ){
		strcat(record,"0000000000");
		memset(buf, '\0', sizeof(buf));
	}
	else{
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%010d",(int)(100*atof(azResult[1])+0.5));
		strcat(record,buf);
	}
//	print2msg("Daily_Energy",buf);
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}
	sqlite3_close(db);

	strcat(record,"END");
	memset(buf, '\0', sizeof(buf));
	sprintf(buf,"%05d", strlen(record));
	strncpy(&record[5],buf,5);
	print2msg("Send",record);
	strcat(record,"\n");
	for(i=0;i<3;i++){
		if(-1!=send(fd_sock,record,strlen(record),0)){
			printmsg("response_system_info: end");
			return 0;
		}
	}
	printmsg("response_system_info: send failed");
	return -1;
}

int response_system_power(int fd_sock, char *date, char *head)
{
	FILE *fp;
	char sql[100]={'\0'};
	char buf[50]={'\0'};
	char record[65535]={'\0'};
	sqlite3 *db;
	char **azResult;
	int nrow, ncolumn;
	char *zErrMsg = 0;
	int i, ret;

	printmsg("response_system_power: start");
	//消息头
	strcat(record,head);
	//获取ID号
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){	
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	fp=NULL;
	strcat(record,buf);
//	print2msg("ECU_ID",buf);
	//获取APP请求日期
	strcat(record,date);								
	//打开数据库，按当天时间排序
	if(sqlite3_open("/home/historical_data.db",&db)){					
		fprintf(stderr, "Can't open historical_data: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	memset(sql, '\0', sizeof(sql));							
	sprintf(sql,"SELECT time,each_system_power FROM each_system_power WHERE date='%s' ORDER BY time", date);	
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}
	if(!nrow){
		strcat(record,"000000000000000");
	}
	else{
		//开始时间
		strcat(record,azResult[ncolumn]);
//		print2msg("ECU_Send_BeginTime",azResult[ncolumn]);
		//结束时间							
		strcat(record,azResult[nrow*ncolumn]);
//		print2msg("ECU_Send_EndTime",azResult[nrow*ncolumn]);
		//功率组数
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%03d",nrow);			
		strcat(record,buf);
//		print2msg("ECU_Power_Num",buf);
		//功率具体信息
		for(i=1;i<=nrow;i++){
			//时间
			strcat(record,azResult[i*ncolumn]);
			//功率
			memset(buf, '\0', sizeof(buf));	
			sprintf(buf,"%010d",(int)(100*atof(azResult[i*ncolumn+1])+0.5));
			strcat(record,buf);
//			print2msg(azResult[i*ncolumn],buf);
		}
	}
	//释放内存并关闭数据库
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}
	sqlite3_close(db);
	
	strcat(record,"END");
	memset(buf, '\0', sizeof(buf));
	sprintf(buf,"%05d", strlen(record));
	strncpy(&record[5],buf,5);
	print2msg("Send",record);
	strcat(record,"\n");
	for(i=0;i<3;i++){
		if(-1!=send(fd_sock,record,strlen(record),0)){
			printmsg("response_system_power: end");
			return 0;
		}
	}
	printmsg("response_system_power: send failed");
	return -1;	
}

int response_daily_energy(int fd_sock, char *date, char *head)
{
	FILE *fp;
	char sql[100]={'\0'};
	char buf[50]={'\0'};
	char record[65535]={'\0'};
	sqlite3 *db;
	char **azResult;
	int nrow, ncolumn;
	char *zErrMsg = 0;
	int i, ret;

	printmsg("response_daily_energy: start");
	//消息头
	strcat(record,head);
	//获取ID号
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){	
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	fp=NULL;
	strcat(record,buf);
//	print2msg("ECU_ID",buf);
	//打开数据库，取出当月数据按时间排序
	if(sqlite3_open("/home/historical_data.db",&db)){					
		fprintf(stderr, "Can't open historical_data: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	memset(buf, '\0', sizeof(buf));	
	strncpy(buf,date,6);	
	memset(sql, '\0', sizeof(sql));						
	sprintf(sql,"SELECT * FROM daily_energy WHERE date LIKE '%s__' ORDER BY date", buf);	
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}
	if(!nrow){
		strcat(record,"000000000000000000");
	}
	else{
		//起始日
		strcat(record,azResult[ncolumn]);
//		print2msg("ECU_Send_BeginDay",azResult[ncolumn]);
		//结束日							
		strcat(record,azResult[nrow*ncolumn]);
//		print2msg("ECU_Send_EndDay",azResult[nrow*ncolumn]);
		//天数
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%02d",nrow);			
		strcat(record,buf);
//		print2msg("ECU_MonthEnergy_Num",buf);
		//能量具体信息
		for(i=1;i<=nrow;i++){
			//日期
			memset(buf, '\0', sizeof(buf));
			strncpy(buf,&azResult[i*ncolumn][6],2);
			strcat(record,buf);
			//能量
			memset(buf, '\0', sizeof(buf));	
			sprintf(buf,"%010d",(int)(100*atof(azResult[i*ncolumn+1])+0.5));
			strcat(record,buf);
//			print2msg(&azResult[i*ncolumn][6],buf);
		}
	}
	//释放内存并关闭数据库
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}
	sqlite3_close(db);

	strcat(record,"END");
	memset(buf, '\0', sizeof(buf));
	sprintf(buf,"%05d", strlen(record));
	strncpy(&record[5],buf,5);
	print2msg("Send",record);
	strcat(record,"\n");
	for(i=0;i<3;i++){
		if(-1!=send(fd_sock,record,strlen(record),0)){
			printmsg("response_daily_energy: end");
			return 0;
		}
	}
	printmsg("response_daily_energy: send failed");
	return -1;	
}

int response_monthly_energy(int fd_sock, char *date, char *head)
{
	FILE *fp;
	char sql[100]={'\0'};
	char buf[50]={'\0'};
	char record[65535]={'\0'};
	sqlite3 *db;
	char **azResult;
	int nrow, ncolumn;
	char *zErrMsg = 0;
	int year, month;
	int i, ret;

	printmsg("response_monthly_energy: start");
	//消息头
	strcat(record,head);
	//获取ID号
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){	
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	fp=NULL;
	strcat(record,buf);
//	print2msg("ECU_ID",buf);
	//打开数据库，取出最近一年数据并按时间排序
	if(sqlite3_open("/home/historical_data.db",&db)){
		fprintf(stderr, "Can't open historical_data: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	memset(buf, '\0', sizeof(buf));	
	year = atoi(strncpy(buf,date,4));
	memset(buf, '\0', sizeof(buf));	
	month = atoi(strncpy(buf,&date[4],2));
	memset(sql, '\0', sizeof(sql));	
	if(12==month){
		sprintf(sql,"SELECT * FROM monthly_energy WHERE date BETWEEN '%04d01' AND '%04d12' ORDER BY date", year,year);
	}
	else{	
		sprintf(sql,"SELECT * FROM monthly_energy WHERE date BETWEEN '%04d%02d' AND '%04d%02d' ORDER BY date", year-1,month+1,year,month);
	}
	for(i=0;i<3;i++){
		ret = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == ret)break;//查询成功
		sqlite3_free_table( azResult );
		print2msg("Get data failed",zErrMsg);
		usleep(1000);
	}
	if(!nrow){
		strcat(record,"00000000000000");
	}
	else{
		//起始月
		strcat(record,azResult[ncolumn]);
//		print2msg("ECU_Send_BeginMonth",azResult[ncolumn]);
		//结束月							
		strcat(record,azResult[nrow*ncolumn]);
//		print2msg("ECU_Send_EndMonth",azResult[nrow*ncolumn]);
		//月数
		memset(buf, '\0', sizeof(buf));	
		sprintf(buf,"%02d",nrow);			
		strcat(record,buf);
//		print2msg("ECU_YearEnergy_Num",buf);
		//能量具体信息
		for(i=1;i<=nrow;i++){
			//日期
			memset(buf, '\0', sizeof(buf));
			strcpy(buf,azResult[ncolumn*i]);
			strcat(record,buf);
			//能量
			memset(buf, '\0', sizeof(buf));	
			sprintf(buf,"%010d",(int)(100*atof(azResult[i*ncolumn+1])+0.5));
			strcat(record,buf);
//			print2msg(azResult[ncolumn*i],buf);
		}
	}
	//释放内存并关闭数据库
	if(SQLITE_OK == ret){
		sqlite3_free_table( azResult );
	}
	sqlite3_close(db);

	strcat(record,"END");
	memset(buf, '\0', sizeof(buf));
	sprintf(buf,"%05d", strlen(record));
	strncpy(&record[5],buf,5);
	print2msg("Send",record);
	strcat(record,"\n");
	for(i=0;i<3;i++){
		if(-1!=send(fd_sock,record,strlen(record),0)){
			printmsg("response_monthly_energy: end");
			return 0;
		}
	}
	printmsg("response_monthly_energy: send failed");
	return -1;	
}

int response_inverter_id(int fd_sock, char *date, char *ids, char *head)
{
	FILE *fp;
	char sql[65536]={'\0'};
	char buf[50]={'\0'};
	char record[65535]={'\0'};
	sqlite3 *db;
	//char **azResult;
	//int nrow, ncolumn;
	char *zErrMsg = 0;
	//int year, month;
	int i, j, ret,num;
	char inverterid[256][13];

	printmsg("response_inverter_id: start");
	//消息头
	strcat(record,head);
	//获取ID号
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	fp=NULL;
	strcat(record,buf);		//head+ecuid...
	strcat(record,date);	//head+12+14..(长度未改)
	strcat(record,"2END");

	if(sqlite3_open("/home/database.db",&db)){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	sprintf(sql,"DELETE FROM id");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK == sqlite3_exec(db, sql , 0, 0, &zErrMsg))
			break;
	}
	num=strlen(ids)/13;
	for(i=0;i<num;i++)
	{
		strncpy(inverterid[i],&ids[i*13],12);printf("%d:%s\t",i,inverterid[i]);
	}

//	memset(sql,'\0',65536);sprintf(sql,"INSERT INTO id (id,flag) VALUES (111111111111,0),(222222222222,0);");sqlite3_exec(db, sql , 0, 0, &zErrMsg);
	memset(sql,'\0',65536);
	for(i=0;i<num;i++)
	{

		strcat(sql,"INSERT INTO id (id,flag) VALUES");
		strcat(sql,"('");
		strcat(sql,inverterid[i]);
		strcat(sql,"',0);");
	}
	print2msg("sql",sql);
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK == sqlite3_exec(db, sql , 0, 0, &zErrMsg))
		{
			record[41]='0';
			break;
		}
		else
			record[41]='1';
	}


	system("killall main.exe");
	memset(buf, '\0', sizeof(buf));
	sprintf(buf,"%05d", strlen(record));
	strncpy(&record[5],buf,5);
	print2msg("Send",record);
	strcat(record,"\n");
	for(i=0;i<3;i++){
		if(-1!=send(fd_sock,record,strlen(record),0)){
			printmsg("response_inverter_id: end");
			return 0;
		}
	}
	printmsg("response_inverter_id: send failed");
	return -1;

}

response_timezone(int fd_sock, char *date, char *tmzone, char *head)
{
	FILE *fp;
	char sql[65536]={'\0'};
	char buf[50]={'\0'};
	char record[65535]={'\0'};
	sqlite3 *db;
	//char **azResult;
	//int nrow, ncolumn;
	char *zErrMsg = 0;
	//int year, month;
	int i, j, ret,num;

	printmsg("response_timezone: start");
	//消息头
	strcat(record,head);
	//获取ID号
	if(NULL==(fp = fopen("/etc/yuneng/ecuid.conf", "r"))){
		perror("ecuid.conf");
		return -1;
	}
	fgets(buf, 12+1, fp);
	fclose(fp);
	fp=NULL;
	strcat(record,buf);		//head+ecuid...
	strcat(record,date);	//head+12+14..(长度未改)
	strcat(record,"2END");
	sprintf(sql,"cp /usr/share/zoneinfo/%s /etc/localtime",tmzone);
	system(sql);
	FILE *fpfp;
	memset(sql,'\0',65536);
	sprintf(sql,"/usr/share/zoneinfo/%s",tmzone);
	if(NULL==(fpfp = fopen(sql, "r")))
	{record[41]='1';printmsg("no such zone");}
	else{
		if(NULL==(fp = fopen("/etc/yuneng/timezone.conf", "w"))){
			perror("ecuid.conf");
			return -1;
		}

		fclose(fpfp);
		if(EOF==fputs(tmzone,fp))
			record[41]='1';			//1失败 0成功
		else record[41]='0';
		fclose(fp);
	}


	//system("killall main.exe");
	memset(buf, '\0', sizeof(buf));
	sprintf(buf,"%05d", strlen(record));
	strncpy(&record[5],buf,5);
	print2msg("Send",record);
	strcat(record,"\n");
	for(i=0;i<3;i++){
		if(-1!=send(fd_sock,record,strlen(record),0)){
			printmsg("response_timezone: end");
			return 0;
		}
	}
	printmsg("response_timezone: send failed");
	return -1;

}

void *msg_handle(void *fd)
{
	fd_set rd;					//select监视的读文件描述符集合
	struct timeval timeout, stime, etime;
	char buf[65535] = {'\0'};
	char date[9] = {'\0'};
	char head[20] = {'\0'};
	int recvbytes;
	double seconds = 0;
	int clientfd = *((int *)fd);

	int leng=0;
	char length[6]= {'\0'};
	char date_time[15]= {'\0'};
	char ids[65536];
	char tmzone[128];

	//客户端处理
	while(1)
	{
		printtime2(1, fd, (unsigned int)pthread_self());
		gettimeofday(&stime,NULL);	//开始时间
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		FD_ZERO(&rd);				//将select()监视的读的文件描述符集合清除
		FD_SET(clientfd, &rd);		//将新建的socket描述符加到select()监视的读的文件描述符集合中
		if(0>=select(clientfd+1, &rd, NULL, NULL, &timeout))
		{
			printmsg("Receive date from EMA timeout");
			break;
		}
		else if(FD_ISSET(clientfd, &rd))
		{
			usleep(1000);
			memset(buf, '\0', sizeof(buf));
			if(-1==(recvbytes=recv(clientfd,buf,sizeof(buf),0)))
			{
				printmsg("recv failed");
				break;
			}
			if(recvbytes==0)
			{
				printmsg("client closed");
				break;
			}
			print2msg("Recv",buf);
			//判断以APS开始以END结束的消息头
			if((!strncmp(buf,"APS",3))&&(!strncmp(&buf[12],"END",3)))
			{
				memset(head, '\0', sizeof(head));
				strncpy(head,buf,15);
				if((!strncmp(&buf[10],"01",2)))
				{
					response_ecu_info(clientfd,head);
				}
				else if((!strncmp(&buf[10],"02",2)))
				{
					strncpy(date,&buf[27],8);
					response_system_info(clientfd,date,head);
				}
				else if((!strncmp(&buf[10],"03",2)))
				{
					strncpy(date,&buf[27],8);
					response_system_power(clientfd,date,head);
				}
				else if((!strncmp(&buf[10],"04",2)))
				{
					strncpy(date,&buf[27],8);
					response_daily_energy(clientfd,date,head);
				}
				else if((!strncmp(&buf[10],"05",2)))
				{
					strncpy(date,&buf[27],8);
					response_monthly_energy(clientfd,date,head);
				}
				else if((!strncmp(&buf[10],"10",2)))
				{
					strncpy(length,&buf[5],5);strncpy(date_time,&buf[27],14);
					if(1==check_ecuid(buf)){
					leng=atoi(length);strncpy(ids,&buf[44],leng-47);printdecmsg("leng",leng);print2msg("ids",ids);
					response_inverter_id(clientfd,date_time,ids,head);}
					else response_nomatch_ecuid(clientfd,date_time,head);
				}
				else if((!strncmp(&buf[10],"11",2)))
				{
					strncpy(length,&buf[41],3);strncpy(date_time,&buf[27],14);
					if(1==check_ecuid(buf)){
					leng=atoi(length);strncpy(tmzone,&buf[44],leng);printdecmsg("leng",leng);print2msg("tmzone",tmzone);
					response_timezone(clientfd,date_time,tmzone,head);}
					else response_nomatch_ecuid(clientfd,date_time,head);
				}

				else
				{
					printmsg("no such message");
				}
				gettimeofday(&etime,NULL);//结束时间
				seconds = ((double)etime.tv_sec * 1000000 + (double)etime.tv_usec)
						 -((double)stime.tv_sec * 1000000 + (double)stime.tv_usec);
				memset(buf, '\0', sizeof(buf));
				sprintf(buf,"time cost: %0.3f ms , start_usec= %d, end_usec= %d Thread: %u\n",
						seconds/1000,stime.tv_usec,etime.tv_usec, (unsigned int)pthread_self());
				printmsg(buf);
				break;
			}
		}
		else{
			break;
		}
	}
	printtime2(2, fd, (unsigned int)pthread_self());
	free(fd);//释放为clientfd指针所分配的内存
	close(clientfd);
	pthread_exit(NULL);
}

int main(void)
{
	int sockfd, *clientfd, ret;
	pthread_t thread;
	
	sockfd = create_socket();
	bind_socket(sockfd);
	listen_socket(sockfd);
	while(1)
	{
		clientfd = (int *)malloc(sizeof(int));
		printtime2(1, clientfd, 0);
		if(NULL == clientfd)
		{
			printmsg("malloc failed");
		}
		if(-1 == (*clientfd = accept_socket(sockfd)))//阻塞等待连接
		{
			printmsg("accept failed");
			free(clientfd);
			continue;
		}
		ret = pthread_create(&thread, NULL, msg_handle, clientfd);//创建子线程,并将连接传给子线程
		if(ret)
		{
			printmsg("pthread_create failed");
			free(clientfd);
			continue;
		}
 //		printtime2(1, clientfd,(unsigned int)thread);
		pthread_detach(thread);//子线程运行结束后会自动释放资源
	}
	close(sockfd);
	return 0;
}
