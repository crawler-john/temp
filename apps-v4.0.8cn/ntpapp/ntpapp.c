/******************************************************************************
name:ntpapp.c
date:2011.01.24
version:1.0
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
#include<sys/time.h>
#include<unistd.h>

#include "ntpapp.h"

//#define DEBUG 0

int main(void)
{
    int sockfd;
    int ret,times, i=0;
    struct timeval oldtime,newtime,timeout;
    struct sockaddr_in serversocket;
    NTPPACKET receivepacket;
    fd_set readfd;

    sockfd = create_socket();
    if(-1==sockfd)
    {
      #ifdef DEBUG
        printf("Create socket error!\n");
      #endif 
      return 0;
    }
    else{
      #ifdef DEBUG
      printf("socket=%d\n",sockfd);
      #endif
    }
    ret = connecttoserver(sockfd, &serversocket);
    if(-1==ret)
    {
      #ifdef DEBUG
        printf("Connect server error!\n");
      #endif
      return 0;
    }
    else{
      #ifdef DEBUG
      printf("socket=%d\n",ret);
      #endif
    }

    for(i=0; i<5; i++){
	send_packet(sockfd);
	for(times=0;times<5;times++){
		FD_ZERO(&readfd);
		FD_SET(sockfd, &readfd);

		timeout.tv_sec = 6;
		timeout.tv_usec = 0;
		ret = select(sockfd+1, &readfd, NULL, NULL, &timeout);
		/*for(times=0; (0==ret || !FD_ISSET(sockfd, &readfd))&&(times<50); times++){
			send_packet(sockfd);
		}*/
#ifdef DEBUG
		printf("ret=%d\n",ret);
#endif
	//if (0 == ret || !FD_ISSET(sockfd, &readfd)){
		/* send ntp protocol packet. */
	//send_packet(sockfd);
	//continue;
	//}
		if(ret>0){
			if(-1!=receive_packet(sockfd, &receivepacket, &serversocket)){
				gettimepacket(&receivepacket, &newtime);
#ifdef DEBUG
				printf("server time= %s\n",ctime(&(newtime.tv_sec)));
#endif
			}
			update_time(&newtime);
			break;
		}
	}
	if(ret>0)
		break;
    }
    
    close(sockfd);
    return 0;
}
