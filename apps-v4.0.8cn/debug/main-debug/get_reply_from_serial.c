#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_reply_from_serial(int plcmodem, int sec, int usec, char *data)			//读取逆变器的返回帧
{
	int size;
	fd_set rd;
	struct timeval timeout;
	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;

	if(select(plcmodem+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		printmsg("Get reply time out");
		return -1;
	}
	else
	{
		fflush(stdout);
		size = read(plcmodem, data, 255);
		printhexmsg("Reply", data, size);
		return size;
	}
}
