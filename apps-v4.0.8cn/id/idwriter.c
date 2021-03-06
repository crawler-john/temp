#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>

#include "sqlite3.h"

#define SERVERPORT 4540
#define MAXCONNECTNUMBER 10
#define BACKLOG 5

int create_socket(void)
{
	int sockfd;
	if(-1==(sockfd=socket(AF_INET,SOCK_STREAM,0))){
		perror("socket");
		exit(1);
	}
	printf("Create socket successfully!  socket:%d\n",sockfd);
	return sockfd;
}

void bind_socket(int sockfd)
{
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(SERVERPORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero),8);

	if(-1==bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))){
		perror("bind");
		exit(0);
	}
	printf("Bind socket successfully!\n");
}

void listen_socket(int sockfd)
{
	if(-1==listen(sockfd,BACKLOG)){
		perror("listen");
		exit(-1);
	}
	printf("Listen socket successfully!\n");
}

int accept_socket(int sockfd)
{
	int sin_size;
	int clientfd;
	struct sockaddr_in client_sockaddr;

	sin_size=sizeof(struct sockaddr_in);
	if(-1==(clientfd=accept(sockfd,(struct sockaddr *)&client_sockaddr,&sin_size))){
		perror("accept");
		exit(1);
	}

	return clientfd;
}

int recv_cmd(int fd_sock, char *readbuff)
{
	fd_set rd;
	struct timeval timeout;
	int recvbytes, readbytes = 0, res;
	char recvbuff[65535], temp[16];

	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(fd_sock, &rd);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		res = select(fd_sock+1, &rd, NULL, NULL, &timeout);
		if(res <= 0){
			//printmsg("Receive command timeout");
			return -1;
		}
		else{
			memset(recvbuff, '\0', sizeof(recvbuff));
			memset(temp, '\0', sizeof(temp));
			recvbytes = recv(fd_sock, recvbuff, 65535, 0);
			if(0 == recvbytes)
				return -1;
			strcat(readbuff, recvbuff);
			readbytes += recvbytes;
			return readbytes;
//			if(readbytes >= 10)
//			{
//				for(i=5, j=0; i<10; i++)
//				{
//					if(readbuff[i] <= '9' && readbuff[i] >= '0')
//						temp[j++] = readbuff[i];
//				}
//				if(atoi(temp) == strlen(readbuff))
//				{
//					print2msg("Recv", readbuff);
//					printdecmsg("Receive", readbytes);
//					return readbytes;
//				}
//			}
		}
	}
}

void version_change(char *version)
{
    char sql[1000]={'\0'};
    char *zErrMsg=0;
    sqlite3 *db;

    sqlite3_open("/home/database.db", &db);

	if((version[strlen(version)-2] == 'N') && (version[strlen(version)-1] == 'A'))
		sprintf(sql,"UPDATE preset SET lv1=119, uv1=295, lv2=211, uv2=264, lf=593, uf=605, rt=300 WHERE id=1;");
	else if(!strncmp(&version[strlen(version)-6], "NA-120", 6))
		sprintf(sql,"UPDATE preset SET lv1=79, uv1=181, lv2=95, uv2=155, lf=570, uf=620, rt=300 WHERE id=1;");
	else
		sprintf(sql,"UPDATE preset SET lv1=119, uv1=295, lv2=200, uv2=270, lf=475, uf=505, rt=80 WHERE id=1;");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	sqlite3_close(db);
}

int settime(char *datetime)
{
	time_t tm;
	struct tm set_tm;
	struct timeval tv;
	time_t timep;

	char year[5] = {'\0'};
	char month[5] = {'\0'};
	char day[5] = {'\0'};
	char hour[5] = {'\0'};
	char minute[5] = {'\0'};
	char second[5] = {'\0'};

	strncpy(year, datetime, 4);
	strncpy(month, &datetime[4], 2);
	strncpy(day, &datetime[6], 2);
	strncpy(hour, &datetime[8], 2);
	strncpy(minute, &datetime[10], 2);
	strncpy(second, &datetime[12], 2);

	int fp;
	fp = open("/dev/rtc2",O_WRONLY);
	write(fp, datetime, strlen(datetime));
	close(fp);

	set_tm.tm_year = atoi(year)-1900;
	set_tm.tm_mon = atoi(month)-1;
	set_tm.tm_mday = atoi(day);
	set_tm.tm_hour = atoi(hour);
	set_tm.tm_min = atoi(minute);
	set_tm.tm_sec = atoi(second);

    timep = mktime(&set_tm);
//	if(set_tm.tm_isdst>0)
//		timep -= set_tm.tm_isdst*3600;
    tv.tv_sec = timep;
    tv.tv_usec = 0;

	if(settimeofday(&tv,NULL)<0)
		return -1;
	else
		return 0;
}

int insertinverter(char *buff)
{
	char sql[100] = {'\0'};
	char *zErrMsg = 0;
	sqlite3 *db;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	FILE *fp;
	char id[16];
	int i;

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "DELETE FROM id");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	for(i=0; i<(strlen(buff)+1)/13; i++)
	{
		memset(sql, '\0', sizeof(sql));
		memset(id, '\0', sizeof(id));
		strncpy(id, &buff[i*13], 12);
		//sprintf(sql,"INSERT INTO inverter_info (id) VALUES('%s');", id);
		sprintf(sql,"INSERT INTO id (ID, flag) VALUES('%s',%d);",id, 0);
		sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	}
	sqlite3_close(db);

	fp = fopen("/etc/yuneng/autoflag.conf", "w");
	fputs("0", fp);
	fclose(fp);
	fp = fopen("/etc/yuneng/limitedid.conf", "w");
	fputs("1", fp);
	fclose(fp);
	system("killall main.exe");

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "SELECT ID from id");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	sqlite3_free_table(azResult);
	sqlite3_close(db);

	return nrow;
}

int getrecord(char *record)
{
	char sql[100] = {'\0'};
	char *zErrMsg = 0;
	sqlite3 *db;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	int i;

	sqlite3_open("/home/record.db", &db);
	strcpy(sql, "SELECT record FROM Data ORDER BY date_time DESC LIMIT 1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(nrow > 0)
		strcpy(record, azResult[nrow]);
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	return nrow;
}

int getevent(char *eve)
{
	char sql[100] = {'\0'};
	char *zErrMsg = 0;
	sqlite3 *db;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	int i;

	memset(eve,'\0',sizeof(eve));
	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "SELECT device,date,eve FROM Event WHERE date IN (SELECT date FROM Event ORDER BY date DESC LIMIT 1)");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	for(i=1;i<=nrow;i++)
	{
		strcat(eve, azResult[i*ncolumn]);
		strcat(eve, azResult[(i*ncolumn+1)]);
		strcat(eve, azResult[(i*ncolumn+2)]);
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	return nrow;
}

int serialanswer()
{
	int fd, res;
	char sendbuff[256]={'\0'};
	struct termios newtio;

	fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY);		//打开串口
	if(fd<0){
		perror("MODEMDEVICE");
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 51;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	write(fd, "serialtest", 10);
	close(fd);

	return 0;
}

int serialanswer2()                                       //测试485
{
	int serialfd, res,rs485fd,res2;
	char sendbuff[256]={'\0'};
	struct termios newtio;

	serialfd = open("/dev/ttyO4", O_RDWR | O_NOCTTY);		//打开串口
	if(serialfd<0){
		perror("/dev/ttyO4");
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 51;
	tcflush(serialfd, TCIFLUSH);
	tcsetattr(serialfd, TCSANOW, &newtio);

	struct termios newtio2;
	rs485fd = open("/dev/rs485_dr", O_RDWR | O_NOCTTY);		//打开485
	if(rs485fd<0){
		perror("/dev/rs485_dr");
	}

	bzero(&newtio2, sizeof(newtio2));					//设置串口参数
	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 51;
	tcflush(rs485fd, TCIFLUSH);
	tcsetattr(rs485fd, TCSANOW, &newtio2);

	ioctl(rs485fd,1,NULL);
	usleep(800000);
	write(serialfd,"serialtest2",11);
	close(rs485fd);
	close(serialfd);

	return 0;
}

int get_time(char db_time[])         //读取ECU本地时间
{
	time_t tm;
	struct tm record_time;    //记录时间
	char temp[5]={'\0'};

	memset(db_time,'\0',sizeof(db_time));
	time(&tm);
	memcpy(&record_time,localtime(&tm), sizeof(record_time));
	sprintf(temp, "%d", record_time.tm_year+1900);
	strcat(db_time,temp);
	if(record_time.tm_mon+1<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_mon+1);
	strcat(db_time,temp);
	if(record_time.tm_mday<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_mday);
	strcat(db_time,temp);
	if(record_time.tm_hour<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_hour);
	strcat(db_time,temp);
	if(record_time.tm_min<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_min);
	strcat(db_time,temp);
	if(record_time.tm_sec<10)
		strcat(db_time,"0");
	sprintf(temp, "%d", record_time.tm_sec);
	strcat(db_time,temp);
}

int testrelay()     //测试接触器
{
	int relay;
	relay=open("/dev/relay",O_WRONLY);
//				ioctl(relay,0,NULL);
//				sleep(10);
				ioctl(relay,10);
				write(relay,"0",1);
				ioctl(relay,20);
				write(relay,"0",1);
				sleep(5);
//				printf("relay=%d\n",relay);
				ioctl(relay,10);
				write(relay,"1",1);
				ioctl(relay,20);
				write(relay,"1",1);
				close(relay);

}

int opensensor(void)		//打开meter串口
{
	int fd;
	struct termios newtio;

	fd = open("/dev/ttyO3", O_RDWR | O_NOCTTY);		//打开串口
	if(fd<0){
		perror("MODEMDEVICE");
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = B57600 | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 1;
	newtio.c_cc[VMIN] = 100;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	return fd;
}

int get_reply_from_module(int fd,char *data)			//读取meter的返回帧
{
	int ret, size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(fd, &rd);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	if(select(fd+1, &rd, NULL , NULL, &timeout) <= 0)
	{
//		printmsg("Get reply time out");
		return -1;
	}
	else
	{
		size = read(fd, data, 255);
//		printhexmsg("Reply", data, size);
		return size;
	}
}

int metergetdata(int fd,char *readmeter,int chip)		//meter问询，返回存入data
{
	int res, i, j, z , sendtimes=0, recvbytes;
//	int fre;
	struct timeval timeout;
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	char fre[5]={'\0'};
	char volA[5]={'\0'};
	char volB[5]={'\0'};
	char volC[5]={'\0'};
	char data[255]={'\0'};

	tcflush(fd,TCIOFLUSH);
	sleep(1);      //发送指令前，先清空缓冲区
	sendbuff[0]  = 0xFB;
	sendbuff[1]  = 0xFB;
	sendbuff[2]  = 0x09;
	sendbuff[3]  = 0xE0;
	sendbuff[4]  = 0x00;
	sendbuff[5]  = 0x01;
	if(chip==1)
		sendbuff[6]  = 0x01;
	if(chip==2)
		sendbuff[6]  = 0x02;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	for(i=2;i<10;i++){
		check += sendbuff[i];
		}

	sendbuff[10]  = check >> 8;
	sendbuff[11]  = check;
	sendbuff[12] = 0xFE;
	sendbuff[13] = 0xFE;

//	printhexmsg("metergetdata",sendbuff,14);
	for(j=0; j<3; j++){              //发送3次
		write(fd, sendbuff, 14);
//		sleep(3);
		recvbytes=get_reply_from_module(fd,data);

//		printhexmsg("data", data,recvbytes);
		if ((70 == recvbytes)&& (0xFB == data[0])&& (0xFB == data[1])
			 &&(0x41 == data[2])
			 &&(0xD0 == data[3])
			 &&(0xFE == data[69]))
		{
			sprintf(fre,"%03d",(data[8]*256+data[9]));
			sprintf(volA,"%04d",(data[10]*256+data[11]));
			sprintf(volB,"%04d",(data[12]*256+data[13]));
			sprintf(volC,"%04d",(data[14]*256+data[15]));
//			printf("volA=%s\n",volA);
//			printf("volb=%s\n",volB);
//			printf("volc=%s\n",volC);
//			printf("fre=%s\n",fre);
			if(chip==1){
			strcat(readmeter,volA);
			strcat(readmeter,volB);
			strcat(readmeter,volC);
			strcat(readmeter,fre);
			}
			if(chip==2){
			strcat(readmeter,volA);
			strcat(readmeter,volB);
			strcat(readmeter,volC);
			strcat(readmeter,fre);
			}
			return 1;
		}
		else
			continue;
	}
	return 0;
}

int clearrecord()
{
	char sql[100] = {'\0'};
	char *zErrMsg = 0;
	sqlite3 *db;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	FILE *fp;

	fp = fopen("/etc/yuneng/connect_time.conf", "w");
	fclose(fp);
	fp = fopen("/etc/yuneng/autoflag.conf", "w");
	fputs("0", fp);
	fclose(fp);

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "DELETE FROM ID");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	memset(sql, '\0', sizeof(sql));
	strcpy(sql, "DELETE FROM Event");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	memset(sql, '\0', sizeof(sql));
	strcpy(sql, "UPDATE ltpower SET power=0.0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	memset(sql, '\0', sizeof(sql));
	strcpy(sql, "DELETE FROM tdpower");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	sqlite3_close(db);

	sqlite3_open("/home/record.db", &db);
	memset(sql, '\0', sizeof(sql));
	strcpy(sql, "DELETE FROM Data WHERE item >= 0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	sqlite3_close(db);

	system("killall main.exe");
}

int main(void)
{
	char recvbuff[65535] = {'\0'};
	int sockfd,clientfd,meterfd,channel;
	int recvbytes;
	int ledfd;
	FILE *fp;
	char *cmd;
	char ecuid[13] = {'\0'};
	char mac[32] = {'\0'};
	char version[16] = {'\0'};
	char area[8] = {'\0'};
	char record[65535];
	char eve[65535];
	int row,chip;
	char sendbuff[3];
	char readmeter[36] = {'\0'};
	char gettime[14]={'\0'};
	char chanchr[5] = {'\0'};

	sockfd=create_socket();
	bind_socket(sockfd);
	listen_socket(sockfd);

	while(1){
		clientfd = accept_socket(sockfd);

		memset(recvbuff, '\0', sizeof(recvbuff));
//		if(-1==(recvbytes=recv(clientfd,recvbuff,sizeof(recvbuff),0))){
//			perror("recv");
//		exit(1);
//		}
//		printf("Received: %s\n", recvbuff);
		recv_cmd(clientfd, recvbuff);

		//烧写和读取ECU的ID
		if(!strncmp(recvbuff, "set_ecu_id", 10)){
			strncpy(ecuid, &recvbuff[11], 12);
			printf("ECU id:%s,  length:%d\n",ecuid,strlen(ecuid));
			fp=fopen("/etc/yuneng/ecuid.conf","w");
			fputs(ecuid,fp);
			fclose(fp);
			memset(ecuid,'\0',sizeof(ecuid));
			sleep(1);
			fp=fopen("/etc/yuneng/ecuid.conf","r");
			fgets(ecuid,13,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,ecuid,strlen(ecuid),0));
		}
		if(!strncmp(recvbuff, "get_ecu_id", 10)){
			memset(ecuid,'\0',sizeof(ecuid));
			fp=fopen("/etc/yuneng/ecuid.conf","r");
			fgets(ecuid,13,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,ecuid,strlen(ecuid),0));
		}

		//烧写和读取ECU有线网络的MAC
		if(!strncmp(recvbuff, "set_eth0_mac", 12)){
			mac[0] = recvbuff[13];
			mac[1] = recvbuff[14];
			mac[2] = ':';
			mac[3] = recvbuff[15];
			mac[4] = recvbuff[16];
			mac[5] = ':';
			mac[6] = recvbuff[17];
			mac[7] = recvbuff[18];
			mac[8] = ':';
			mac[9] = recvbuff[19];
			mac[10] = recvbuff[20];
			mac[11] = ':';
			mac[12] = recvbuff[21];
			mac[13] = recvbuff[22];
			mac[14] = ':';
			mac[15] = recvbuff[23];
			mac[16] = recvbuff[24];
			printf("ECU eth0 MAC address:%s,	length:%d\n",mac,strlen(mac));
			fp=fopen("/etc/yuneng/ecu_eth0_mac.conf","w");
			fputs(mac,fp);
			fclose(fp);
			memset(mac,'\0',sizeof(mac));
			sleep(1);
			fp=fopen("/etc/yuneng/ecu_eth0_mac.conf","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}
		if(!strncmp(recvbuff, "get_eth0_mac", 12)){
			memset(mac,'\0',sizeof(mac));
			fp=fopen("/etc/yuneng/ecu_eth0_mac.conf","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}

		//烧写和读取ECU无线网络的MAC
		if(!strncmp(recvbuff, "set_wlan0_mac", 13)){
			mac[0] = recvbuff[14];
			mac[1] = recvbuff[15];
			mac[2] = ':';
			mac[3] = recvbuff[16];
			mac[4] = recvbuff[17];
			mac[5] = ':';
			mac[6] = recvbuff[18];
			mac[7] = recvbuff[19];
			mac[8] = ':';
			mac[9] = recvbuff[20];
			mac[10] = recvbuff[21];
			mac[11] = ':';
			mac[12] = recvbuff[22];
			mac[13] = recvbuff[23];
			mac[14] = ':';
			mac[15] = recvbuff[24];
			mac[16] = recvbuff[25];
			printf("ECU wlan0 MAC address:%s,	length:%d\n",mac,strlen(mac));
			fp=fopen("/etc/yuneng/ecu_wlan0_mac.conf","w");
			fputs(mac,fp);
			fclose(fp);
			memset(mac,'\0',sizeof(mac));
			sleep(1);
			fp=fopen("/etc/yuneng/ecu_wlan0_mac.conf","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}
		if(!strncmp(recvbuff, "get_wlan0_mac", 13)){
			memset(mac,'\0',sizeof(mac));
			fp=fopen("/etc/yuneng/ecu_wlan0_mac.conf","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}

		//设置ECU的本地时间
		if(!strncmp(recvbuff, "set_time", 8)){
			settime(&recvbuff[9]);
			send(clientfd, &recvbuff[9], 14, 0);
		}

		//设置逆变器的ID
		if(!strncmp(recvbuff, "set_inverter_id", 15)){
/*			if(1==zb_change_channel(18)){
			send(clientfd, "0x12", 4, 0);
			saveECUChannel(18);
          }
			else
			send(clientfd, "change channel failed", 21, 0);
*/
			if(1 == saveECUChannel(18))
				send(clientfd, "0x12", 4, 0);
			else
			send(clientfd, "change channel failed", 21, 0);
			row = insertinverter(&recvbuff[16]);
			snprintf(sendbuff, sizeof(sendbuff), "%02d", row);
//			sendbuff[0] = row / 256;
//			sendbuff[1] = row % 256;
			send(clientfd, sendbuff, 3, 0);
		}

		//读取PLC的测试结果
		if(!strncmp(recvbuff, "query_result", 12)){
			if(getrecord(record) > 0)
				send(clientfd, record, strlen(record), 0);
			else
				send(clientfd, "Failed", 6, 0);
		}
		
		//读取逆变器保护状态
		if(!strncmp(recvbuff, "query_protection", 16)){
			if(getevent(eve) > 0)
				send(clientfd, eve, strlen(eve), 0);
			else
				send(clientfd, "Failed", 6, 0);
		}

		//清空测试记录
		if(!strncmp(recvbuff, "clear", 5)){
/*			channel=getOldChannel();
			if(1==zb_change_channel(channel)){
				snprintf(chanchr, sizeof(chanchr), "0x%02X", channel);
				send(clientfd,chanchr, 4, 0);
				saveECUChannel(channel);
			}
			else
			send(clientfd, "change channel failed", 21, 0);
*/
			if(1 == saveECUChannel(16))
				send(clientfd, "0x10", 4, 0);
			else
			send(clientfd, "change channel failed", 21, 0);
			clearrecord();
			system("rm /home/tmpdb");
			system("rm /home/historical_data.db");
			system("rm /home/record.db");
			send(clientfd, "clearok", 7, 0);
		}

		//串口测试
		if(!strncmp(recvbuff, "serialtest", 10)){
			serialanswer();
		}

		//烧写和读取ECU的地区
		if(!strncmp(recvbuff, "set_area", 8)){
			strncpy(area, &recvbuff[9], sizeof(area));
			fp=fopen("/etc/yuneng/area.conf", "w");
			fputs(area,fp);
			fclose(fp);

			memset(area,'\0',sizeof(area));
			sleep(1);
			fp=fopen("/etc/yuneng/area.conf","r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			printf("Send %d\n",send(clientfd,area,strlen(area),0));
		}

		if(!strncmp(recvbuff, "get_area", 8)){
			memset(area,'\0',sizeof(area));
			fp=fopen("/etc/yuneng/area.conf","r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			printf("Send %d\n",send(clientfd,area,strlen(area),0));
		}

		//读取ECU软件版本号
		if(!strncmp(recvbuff, "get_version", 11)){
			memset(version, 0, sizeof(version));
			fp = fopen("/etc/yuneng/version.conf", "r");
			if(fp){
				fgets(version, sizeof(version), fp);
				if(10 == version[strlen(version)-1]){
					version[strlen(version)-1] = '\0';
				}
				fclose(fp);
			}
			memset(area, 0, sizeof(area));
			fp = fopen("/etc/yuneng/area.conf", "r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			strcat(version, area);
			printf("Send %d\n",send(clientfd, version, strlen(version), 0));
		}

		//读取本地时间
		if(!strncmp(recvbuff, "get_time", 8)){
			get_time(gettime);
			send(clientfd, gettime, 14, 0);
		}

		//读取meter电压频率
		if(!strncmp(recvbuff, "test_meter", 10)){
			meterfd=opensensor();
			for(chip=1;chip<3;chip++){
			metergetdata(meterfd,readmeter,chip);
			}
			send(clientfd, readmeter, 30, 0);
			close(meterfd);
		}

		//RS485串口测试
		if(!strncmp(recvbuff, "serialtest2", 11)){
			serialanswer2();
		}

		//继电器测试
		if(!strncmp(recvbuff, "test_relay", 10)){
			testrelay();
		}

		//测试LED灯
		if(!strncmp(recvbuff, "test_led", 8)){
			ledfd = open("/dev/led",O_WRONLY);
			if(ledfd<0){
				perror("/dev/led");
			}
			usleep(500000);
			ioctl(ledfd,10);
			write(ledfd,"0",1);
			ioctl(ledfd,20);
			write(ledfd,"0",1);
			ioctl(ledfd,30);
			write(ledfd,"0",1);
			sleep(10);
			ioctl(ledfd,10);
			write(ledfd,"1",1);
			ioctl(ledfd,20);
			write(ledfd,"1",1);
			ioctl(ledfd,30);
			write(ledfd,"1",1);
			close(ledfd);
		}

		close(clientfd);
	}
}
