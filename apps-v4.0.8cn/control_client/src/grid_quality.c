#include <stdio.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "mydebug.h"
#include "myfile.h"

/* 【129】ECU上报系统的电网质量 */
int response_grid_quality(const char *recvbuffer, char *sendbuffer)
{
	char grid_quality[2] = {'\0'};
	char ecuid[13] = {'\0'};
	char timestamp[15] = {'\0'};

	//获取ECU_ID
	file_get_one(ecuid, sizeof(ecuid), "/etc/yuneng/ecuid.conf");
	//获取时间戳
	strncpy(timestamp, &recvbuffer[34], 14);
	//获取电网质量
	file_get_one(grid_quality, sizeof(grid_quality), "/etc/yuneng/plc_grid_quality.txt");

	//拼接信息
	msg_Header(sendbuffer, "A129");
	msgcat_s(sendbuffer, 12, ecuid);
	msgcat_s(sendbuffer, 1, grid_quality);
	msgcat_s(sendbuffer, 14, timestamp);
	msgcat_s(sendbuffer, 3, "END");

	return 0;
}
