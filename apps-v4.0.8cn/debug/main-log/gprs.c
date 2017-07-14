#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS3"
#define _POSIX_SOURCE 1
#define MAXSIZE 56000
#define sendsize 800

int opengprs(void)		//打开串口
{
	int fd, res, i=0;
	char buff[MAXSIZE]={'\0'};
	struct termios newtio;
	
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);		//打开串口
	if(fd<0){
		perror("MODEMDEVICE");
	}
	
	/*bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | HUPCL;
	newtio.c_iflag = IGNBRK;
	newtio.c_oflag = 0;
	newtio.c_lflag = ICANON;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);*/

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 100;
	newtio.c_cc[VMIN] = 140;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	
	return fd;
}

int gprssendrecord(int fd, char *sendbuff)		//使用GPRS发送数据，即往串口中写数据，模块会自己发送
{
	int res, i, j, sendtimes=0, recvbytes;
	char readbuff[255]={'\0'};
	char readsize[255]={'\0'};
	fd_set rd;
	struct timeval timeout;
	
	sendtimes=strlen(sendbuff)/sendsize;
	if((strlen(sendbuff)%sendsize)>0){
		sendtimes++;
	}
	
	for(j=0; j<3; j++){
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		timeout.tv_sec = 20;
		timeout.tv_usec = 0;
			
		for(i=0; i<sendtimes; i++){
			sleep(5);									//最后一次发完直接select，所以先延时
			write(fd, sendbuff+i*800, sendsize);
		}

		res = select(fd+1, &rd, NULL , NULL, &timeout);
		if(res<=0){
			printdecmsg("res", res);
			continue;
		}
		
		if(FD_ISSET(fd, &rd)){
			sleep(1);
			recvbytes = read(fd, readbuff, 255);
			print2msg("readbuff", readbuff);
			/*printf("readbuff:%s, sizeof:%d\n", readbuff, strlen(readbuff));
			sprintf(readsize, "%d", strlen(sendbuff)-2);
			readbuff[strlen(readbuff)-1]='\0';
			printf("%s,%s\n", readbuff, readsize);
			if(0==strcmp(readbuff, readsize)){
				return 1;
			}*/
			//if(14 == recvbytes){					//
				if(!strncmp(readbuff, &sendbuff[60], 14))
					return 1;
				else
					continue;
			//}
			//else
				//return 0;
		}
	}
	
	return 0;
}

int closegprs(int fd)		//关闭串口
{
	close(fd);
	
	return 0;
}
