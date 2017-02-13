#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "mydatabase.h"
#include "mydebug.h"

//[A138]开启自动上报
int set_autoflag_report(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};

	//时间戳
	strncpy(timestamp, &recvbuffer[30], 14);

	FILE *fp;
	fp=fopen("/etc/yuneng/autoflag.conf","w");
	if(fp==NULL)
		ack_flag=FILE_ERROR;
	else
	{
		fputs("1",fp);
		fclose(fp);
		ack_flag=SUCCESS;
		system("killall main.exe");
	}
	//保存到数据库

	//拼接应答消息
	msg_ACK(sendbuffer, "A138", timestamp, ack_flag);
	return 0;
}
