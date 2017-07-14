#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "mydebug.h"

int msg_is_complete(const char *s)
{
	int i, msg_length = 18;
	char buffer[6] = {'\0'};

	//
	if(strlen(s) < 10)
		return 0;

	//从信息头获取信息长度
	strncpy(buffer, &s[5], 5);
	for(i=0; i<5; i++)
		if('A' == buffer[i])
			buffer[i] = '0';
	msg_length = atoi(buffer);

	//将实际收到的长度与信息中定义的长度做比较
	if(strlen(s) < msg_length){
		return 0;
	}
	return 1;
}

/* 建立Socket */
int create_socket(void)
{
	int sockfd, iflags;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
//	iflags = fcntl(sockfd, F_GETFL, 0);
//	fcntl(sockfd, F_SETFL, O_NONBLOCK | iflags);
	if(sockfd == -1){
		debug_err("socket");
        exit(1);
	}
	return sockfd;
}

/* （客户端）连接服务器 */
int connect_socket(int sockfd, int port, const char *ip, const char *domain)
{
	/* 函数说明 */
	//htons():将16位主机字节序转换成网络字节序
	//inet_addr():将网络地址转换成二进制数字
	char ip_addr[32] = {'\0'};
	struct hostent *host;
	struct sockaddr_in serv_addr;

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET; //地址族
	serv_addr.sin_port = htons(port); //服务器端口号
	//解析域名
	host = gethostbyname(domain);
	if(host == NULL){
		debug_msg("Get host failed, use default IP : %s", ip);
		serv_addr.sin_addr.s_addr = inet_addr(ip); //服务器IP地址
	}
	else{
		inet_ntop(AF_INET, *host->h_addr_list, ip_addr, sizeof(ip_addr));
		serv_addr.sin_addr.s_addr = inet_addr(ip_addr); //服务器IP地址
	}
	bzero(&(serv_addr.sin_zero), 8);

	//请求连接服务器
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
	{
		debug_msg("Failed to connect to %s:%d %s", ip, port, system_time());
		debug_err("connect");
		close(sockfd);
		exit(1);
	}
	debug_msg("Connecting EMA successfully : %s", system_time());
	return 0;
}

/* 发送数据 */
int send_socket(int sockfd, char *sendbuffer, int size)
{
	int i, send_count;
	char msg_length[6] = {'\0'};

	if(sendbuffer[strlen(sendbuffer)-1] == '\n'){
		sprintf(msg_length, "%05d", strlen(sendbuffer)-1);
	}
	else{
		sprintf(msg_length, "%05d", strlen(sendbuffer));
		strcat(sendbuffer, "\n");
		size++;
	}
	strncpy(&sendbuffer[5], msg_length, 5);
	for(i=0; i<3; i++){
		send_count = send(sockfd, sendbuffer, size, 0);
		if(send_count >= 0){
			sendbuffer[strlen(sendbuffer)-1] = '\0';
			debug_msg("Sent:%s", sendbuffer);
//			sleep(2);
			return send_count;
		}
	}
	debug_msg("Send failed:");
	return -1;
}

/* 接收数据 */
int recv_socket(int sockfd, char *recvbuffer, int size, int timeout_s)
{
	fd_set fds;
	struct timeval timeout = {10,0};
	int recv_each = 0, recv_count = 0;
	char recv_buffer[4096];

	memset(recvbuffer, '\0', size);
	while(1)
	{
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		timeout.tv_sec = timeout_s;

		switch(select(sockfd+1, &fds, NULL, NULL, &timeout)){
			case -1:
				debug_err("select");
			case 0:
				debug_msg("Receive date from EMA timeout");
				close(sockfd);
				debug_msg(">>End\n");
				return -1;
				break;
			default:
				if(FD_ISSET(sockfd, &fds)){
					memset(recv_buffer, '\0', sizeof(recv_buffer));
					recv_each = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
					strcat(recvbuffer, recv_buffer);
					if(recv_each <= 0){
						debug_msg("Communication over:%d", recv_each);
						return -1;
					}
					debug_msg("Received each time:%d", recv_each);
					recv_count += recv_each;
//					debug_msg("Received Total:%d", recv_count);
					debug_msg("Received:%s", recvbuffer);
					if(msg_is_complete(recvbuffer)){
						return recv_count;
					}
				}
				break;
		}
	}
	return 0;
}

int recv_time(int sockfd, char *recvbuffer, int size, int timeout_s)
{
	fd_set fds;
	struct timeval timeout = {10,0};
	int recv_each = 0, recv_count = 0;
	char recv_buffer[4096];

	memset(recvbuffer, '\0', size);
	while(1)
	{
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		timeout.tv_sec = timeout_s;

		switch(select(sockfd+1, &fds, NULL, NULL, &timeout)){
			case -1:
				debug_err("select");
			case 0:
				debug_msg("Receive date from EMA timeout");
				close(sockfd);
				debug_msg(">>End\n");
				return -1;
				break;
			default:
				if(FD_ISSET(sockfd, &fds)){
					memset(recv_buffer, '\0', sizeof(recv_buffer));
					recv_each = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
					strcat(recvbuffer, recv_buffer);
					if(recv_each <= 0){
						debug_msg("Communication over:%d", recv_each);
						return -1;
					}
					debug_msg("Received each time:%d", recv_each);
					recv_count += recv_each;
//					debug_msg("Received Total:%d", recv_count);
					debug_msg("Received:%s", recvbuffer);
					if(recv_count >= 14)
						return recv_count;
				}
				break;
		}
	}
	return 0;
}


/* Socket客户端初始化 */
int client_socket_init(int port, const char *ip, const char *domain)
{
	int sockfd;

	sockfd = create_socket();
	connect_socket(sockfd, port, ip, domain);
	return sockfd;
}

