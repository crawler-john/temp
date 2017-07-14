/******************************************************************************
Name:ntpcomm.c
Date:2011.01.24
Useage:socket operation
Version:1.0
******************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "ntpapp.h"
#include <fcntl.h>

#define MAINDEBUG 0
#define NTPFRAC(x) (4294 * (x) + ((1981 * (x))>>11))
/****************************创建socket***************************************/
int create_socket( void )
{
    int sockfd;
    int addr_len;
    struct sockaddr_in addr_src;
    int ret;

    addr_len = sizeof( struct sockaddr_in );
    memset( &addr_src, 0, addr_len );
    addr_src.sin_family = AF_INET;
    addr_src.sin_addr.s_addr = htonl( INADDR_ANY );
    addr_src.sin_port = htons( 0 );

    if(-1==(sockfd = socket( AF_INET , SOCK_DGRAM , IPPROTO_UDP )))
    {
      #ifdef MAINDEBUG
        printf("Create socket error!\n");
      #endif
      return -1;
    }

    ret = bind(sockfd , (struct sockaddr*)&addr_src , addr_len);
    if(-1==ret)
    {
      #ifdef MAINDEBUG
        printf("Bind error!\n");
      #endif
      return -1;
    }

    return sockfd;
}

/********************************连接NTP服务器*******************************/
int connecttoserver(int sockfd, struct sockaddr_in * serversocket_in)
{
    FILE *fp;
    int addr_len;
    struct sockaddr_in addr_dst;
    struct hostent * host;
    int ret;
    char dn[100]={'\0'};

    fp = fopen("/etc/yuneng/ntp_server.conf", "r");
    fgets(dn, 100, fp);
    fclose(fp);
    printf("%s, %d\n", dn, strlen(dn));
    addr_len = sizeof(struct sockaddr_in);
    memset(&addr_dst, 0, addr_len);
    addr_dst.sin_family = AF_INET;
    if(0==strlen(dn))
      host = gethostbyname("cn.pool.ntp.org");
    else
      host = gethostbyname(dn);
    memcpy(&(addr_dst.sin_addr.s_addr), host->h_addr_list[0], 4);
    addr_dst.sin_port = htons(123);

    memcpy(serversocket_in, &addr_dst, sizeof(struct sockaddr_in));
    ret = connect(sockfd, (struct sockaddr *)&addr_dst, addr_len);
    if(-1==ret)
    {
      #ifdef MAINDEBUG
        printf("Connect error!\n");
      #endif
      close(sockfd);
      return -1;
    }
    else{
      #ifdef DEBUG
      printf("Connect successfully!\n");
      #endif
    }

    return sockfd;
}

/*********************************发送协议包**********************************/
void send_packet(int sockfd)
{
    int bytes=0;
    NTPPACKET sendpacked;
    struct timeval now;
    memset(&sendpacked, 0, sizeof(sendpacked));
    sendpacked.header.LI = 0;
    sendpacked.header.VN = 3;
    sendpacked.header.mode = 3;
    /*sendpacked.header.stratum = 0;
    sendpacked.header.poll = 4;
    sendpacked.header.precision = -6;
    sendpacked.root_delay = 1<<16;
    sendpacked.root_dispersion = 1<<16;*/

    sendpacked.header.stratum = 0;
    sendpacked.header.poll = 0;
    sendpacked.header.precision = 0;
    sendpacked.root_delay = 0;
    sendpacked.root_dispersion = 0;

    //sendpacked.header.headData = htonl(sendpacked.header.headData);
    sendpacked.header.head = htonl(sendpacked.header.head);
    sendpacked.root_delay = htonl(sendpacked.root_dispersion);
    sendpacked.root_dispersion = htonl(sendpacked.root_dispersion);

    gettimeofday(&now, NULL); /* get current local_time*/
    sendpacked.tratimestamp.sec = htonl(now.tv_sec)+0x83aa7e80; /* Transmit Timestamp coarse */
    //sendpacked.tratimestamp.usec = 0;//htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
    //sendpacked.tratimestamp.usec = htonl(NTPFRAC(now.tv_usec));

    if(bytes=send(sockfd, &sendpacked, sizeof(sendpacked), 0))
    {
      #ifdef DEBUG
      printf("send successfully!\n");
      printf("send bytes=%d\n",bytes);
      #endif
    }
    else{
      #ifdef DEBUG
      printf("send failure!\n");
      #endif
    }
}

int receive_packet(int sockfd, NTPPACKET *recvpacked, struct sockaddr_in * serversocket_in)
{
    int receivebytes=0,times;
    int addr_len = sizeof(struct sockaddr_in);

    //for(times=0;(times<50)&&(receivebytes<=0);times++)
    {
      receivebytes = recvfrom(sockfd, recvpacked, sizeof(NTPPACKET), 0, (struct sockaddr *)serversocket_in, &addr_len);
      #ifdef DEBUG
      printf("recevicing : %d\n",receivebytes);
      #endif
    }


    if(-1==receivebytes)
    {
      #ifdef MAINDEBUG
        printf("Receive error!\n");
      #endif
      close(sockfd);
      return -1;
    }

    return receivebytes;
}

void gettimepacket(NTPPACKET *receivepacket, struct timeval * new_time)
{
    NTPTIME trantime;
    trantime.sec = ntohl(receivepacket->tratimestamp.sec)-0x83aa7e80;
    //trantime.usec = ntohl(receivepacket->tratimestamp.usec);
    new_time->tv_sec = trantime.sec;
    #ifdef DEBUG
    printf("new time:%d\n",trantime.sec);
    #endif
    //new_time->tv_usec = trantime.usec;
}

void update_time(struct timeval * new_time)
{
    int fp;
    char write_buff[50]={'\0'};
    char temp[5]={'\0'};
    time_t tm;
    struct timeval tv;
    time_t timep;

    struct tm *timenow;
    timenow = localtime(&new_time->tv_sec);
    timep = mktime(timenow);
    tv.tv_sec = timep;
    tv.tv_usec = 0;

    settimeofday(&tv, NULL);

    sprintf(temp,"%d",timenow->tm_year+1900);
    strcat(write_buff,temp);
    if(timenow->tm_mon+1<10)
      strcat(write_buff,"0");
    memset(temp,'\0',5);
    sprintf(temp,"%d",timenow->tm_mon+1);
    strcat(write_buff,temp);
    if(timenow->tm_mday<10)
      strcat(write_buff,"0");
    memset(temp,'\0',5);
    sprintf(temp,"%d",timenow->tm_mday);
    strcat(write_buff,temp);
    if(timenow->tm_hour<10)
      strcat(write_buff,"0");
    memset(temp,'\0',5);
    sprintf(temp,"%d",timenow->tm_hour);
    strcat(write_buff,temp);
    if(timenow->tm_min<10)
      strcat(write_buff,"0");
    memset(temp,'\0',5);
    sprintf(temp,"%d",timenow->tm_min);
    strcat(write_buff,temp);
    if(timenow->tm_sec<10)
      strcat(write_buff,"0");
    memset(temp,'\0',5);
    sprintf(temp,"%d",timenow->tm_sec);
    strcat(write_buff,temp);
    #ifdef MAINDEBUG
      printf("write in rtc:%s\n",write_buff);
    #endif

    fp=open("/dev/rtc2",O_WRONLY);
    write(fp,write_buff,strlen(write_buff));
    close(fp);
}
