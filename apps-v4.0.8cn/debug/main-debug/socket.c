#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include <arpa/inet.h>
#include <net/if.h>	//ifr
#include <sys/ioctl.h>	//SIOCGIFADDR

#include "access.h"
#include "debug.h"

#define SERVPORT 8990

int createsocket(void)					//创建套接字
{
	int fd_sock;

	fd_sock=socket(AF_INET,SOCK_STREAM,0);
    
	if(-1 == fd_sock)
		printmsg("Create socket failure");
	else
		printmsg("Create socket successfully");

	return fd_sock;
}

int connect_socket(int fd_sock)				//连接到服务器
{
    	char ip[20] = {'\0'};
	char tmp[20] = {'\0'};
	int port = 8990;

	FILE *fp;

	fp = fopen("/etc/yuneng/datacenter.conf", "r");
	fgets(ip, 50, fp);
	fgets(tmp, 50, fp);
	fclose(fp);

	ip[strlen(ip)-1] = '\0';
	if('\n' == tmp[strlen(tmp)-1])
		tmp[strlen(tmp)-1] = '\0';
	port = atoi(tmp);
	//printf("%s, %d\n", ip, port);
	if(!strlen(ip))
		strcpy(ip, "60.190.131.190");
	struct sockaddr_in serv_addr;

	bzero(&serv_addr,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	bzero(&(serv_addr.sin_zero),8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		printmsg("Connect socket failure");	//thread
		return -1;
	}
	else{
		writeconnecttime();
		printmsg("Connect socket successfully");
		return 1;
	}
}

int resend_data(int fd_sock,char *sendbuff)			//发送数据到EMA
{
	int sendbytes=0, res=0, recvbytes = 0;
	char readbuff[65535] = {'\0'};
	fd_set rd;
	struct timeval timeout;

	sendbytes=send(fd_sock,sendbuff,strlen(sendbuff),0);

	printdecmsg("sendbytes", sendbytes);

	if(-1 == sendbytes)
		return -1;

	FD_ZERO(&rd);
	FD_SET(fd_sock, &rd);
	timeout.tv_sec = 120;
	timeout.tv_usec = 0;

	res = select(fd_sock+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		printmsg("socket: receive timeout");
		return 0;
	}
	else{
		sleep(1);
		recvbytes = recv(fd_sock, readbuff, 65535, 0);

		printdecmsg(readbuff, recvbytes);

		if(14 == recvbytes){					//
			if(!strncmp(readbuff, &sendbuff[60], 14))
				return 1;
			else
				return 0;
		}
		else if(0 == recvbytes)
			return -1;
		else if(-1 == recvbytes)
			return -1;
		else
			return 0;
	}
}

int send_record(int fd_sock,char *sendbuff)			//发送数据到EMA
{
	int sendbytes=0, res=0, recvbytes = 0;
	char readbuff[65535] = {'\0'};
	fd_set rd;
	struct timeval timeout;

	sendbytes=send(fd_sock,sendbuff,strlen(sendbuff),0);

	printdecmsg("sendbytes", sendbytes);

	if(-1 == sendbytes)
		return -1;

	FD_ZERO(&rd);
	FD_SET(fd_sock, &rd);
	timeout.tv_sec = 120;
	timeout.tv_usec = 0;

	res = select(fd_sock+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		printmsg("socket: receive timeout");
		return 0;
	}
	else{
		sleep(1);
		recvbytes = recv(fd_sock, readbuff, 65535, 0);

		printdecmsg(readbuff, recvbytes);

		if(14 == recvbytes){					//
			if(!strncmp(readbuff, &sendbuff[60], 14))
				return 1;
			else
				return 0;
		}
		else if(0 == recvbytes)
			return -1;
		else if(-1 == recvbytes)
			return -1;
		else
			return 0;
	}
}

void close_socket(int fd_sock)					//关闭套接字
{
	close(fd_sock);

	printmsg("Close socket");
}

void get_ip(char ip_buff[16])					//获取ECU的IP
{
	struct ifreq ifr;
	int inet_sock;
	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, "eth0");

	if(ioctl(inet_sock,SIOCGIFADDR,&ifr)<0)
		;
	else
		strcpy(ip_buff,inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	close(inet_sock);
}

void get_ip_of_all(char ip_buff[2][20])					//获取ECU的IP
{
	struct ifreq ifr;
	int inet_sock;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	//获取有线IP
	strcpy(ifr.ifr_name, "eth0");
	strcpy(ip_buff[0], "L: ");
	if (ioctl(inet_sock,SIOCGIFADDR,&ifr)<0) {
//		strcat(ip_buff[0]," NO IP Address");
	}
	else{
		strcat(ip_buff[0],inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	}
	//获取无线IP
	strcpy(ifr.ifr_name, "wlan0");
	strcpy(ip_buff[1], "W: ");
	if (ioctl(inet_sock,SIOCGIFADDR,&ifr)<0) {
//		strcat(ip_buff[1]," NO IP Address");
	}
	else{
		strcat(ip_buff[1],inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
	}

	close(inet_sock);
}
