#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

#define BAUDRATE B57600
#define MODEMDEVICE "/dev/ttyO2"
int zbmodem;				//zigbee串口

void clear_zbmodem(void)		//清空串口缓冲区的数据
{
	tcflush(zbmodem,TCIOFLUSH);
	sleep(1);
}

int openzigbee(void)		//打开串口
{
	struct termios newtio;
	zbmodem = open(MODEMDEVICE, O_RDWR | O_NOCTTY);		//打开串口
	if(zbmodem < 0){
		perror("MODEMDEVICE");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 1;
	newtio.c_cc[VMIN] = 100;
	tcflush(zbmodem, TCIFLUSH);
	tcsetattr(zbmodem, TCSANOW, &newtio);

	return zbmodem;
}

int zb_get_reply_from_module(char *data)			//读取zigbee模块的返回帧
{
	int ret, size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(zbmodem, &rd);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	if(select(zbmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
		return -1;
	}
	else
	{
		size = read(zbmodem, data, 255);
		printhexmsg("Reply", data, size);
		return size;
	}
}

int ecupanid()
{
	FILE *fp;
	char buff[50];
	unsigned short ecu_panid;
	
	fp = fopen("/etc/yuneng/ecu_eth0_mac.conf", "r");
	if (fp) {
		memset(buff, '\0', sizeof(buff));
		fgets(buff, 18, fp);
		fclose(fp);
		if((buff[12]>='0') && (buff[12]<='9'))
			buff[12] -= 0x30;
		if((buff[12]>='A') && (buff[12]<='F'))
			buff[12] -= 0x37;
		if((buff[13]>='0') && (buff[13]<='9'))
			buff[13] -= 0x30;
		if((buff[13]>='A') && (buff[13]<='F'))
			buff[13] -= 0x37;
		if((buff[15]>='0') && (buff[15]<='9'))
			buff[15] -= 0x30;
		if((buff[15]>='A') && (buff[15]<='F'))
			buff[15] -= 0x37;
		if((buff[16]>='0') && (buff[16]<='9'))
			buff[16] -= 0x30;
		if((buff[16]>='A') && (buff[16]<='F'))
			buff[16] -= 0x37;
		ecu_panid = ((buff[12]) * 16 + (buff[13])) * 256 + (buff[15]) * 16 + (buff[16]);
		}
    return ecu_panid;
}

// 获取信道，范围：11~26共16个信道
int getOldChannel()
{
	FILE *fp;
	char buffer[4] = {'\0'};

	fp = fopen("/tmp/old_channel.conf", "r");
	if (fp) {
		fgets(buffer, 4, fp);
		fclose(fp);
		return atoi(buffer);
	}
	else
	{
	return 16; //未知信道
	}
}


int saveECUChannel(int channel)
{
	FILE *fp;
	char buffer[5] = {'\0'};

	snprintf(buffer, sizeof(buffer), "0x%02X", channel);
	printf("%s\n", buffer);
	fp = fopen("/etc/yuneng/channel.conf", "w");
	if (fp) {
		system("echo '1' > /etc/yuneng/limitedid.conf");
		fputs(buffer, fp);
		fclose(fp);
		sleep(1);
		return 1;
	}
	return 0;
}

int zb_change_channel(int channel)    //更改ECU信道
{
	unsigned char sendbuff[15] = {'\0'};
	char recvbuff[256];
	unsigned short ecu_panid;
	int i;
	int check=0;
	
	ecu_panid=ecupanid();
	openzigbee();
	//向ECU发送命令
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu_panid>>8;
	sendbuff[8]  = ecu_panid;
	sendbuff[9]  = channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;
	write(zbmodem, sendbuff, 15);
	printhexmsg("Change ECU channel ", sendbuff, 15);

	//接收反馈
	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		sleep(2);		
		return 1;
	}

	return -1;
}
