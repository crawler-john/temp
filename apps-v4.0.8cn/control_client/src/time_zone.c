#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

#include "remote_control_protocol.h"
#include "myfile.h"
#include "mycommand.h"

/* 获取系统当前时间 */
void get_local_time(char *str)
{
	time_t now; //时间类型变量
	struct tm *local_time; //tm结构体指针

	time(&now); //读取当前时间
	local_time = localtime(&now); //转换成本地时间
	sprintf(str, "%04d%02d%02d%02d%02d%02d",
			local_time->tm_year + 1900,
			local_time->tm_mon + 1,
			local_time->tm_mday,
			local_time->tm_hour,
			local_time->tm_min,
			local_time->tm_sec); //将格式化后的时间存入静态缓存
}

/* 【A104】ECU上报本地时区 */
int response_time_zone(const char *recvbuffer, char *sendbuffer)
{
	char ecuid[13] = {'\0'};	  //ECU号码
	char local_time[15] = {'\0'}; //ECU本地时间
	char timestamp[15] = {'\0'};  //时间戳
	char timezone[64] = {'\0'};  //时区

	//获取参数
	file_get_one(ecuid, sizeof(ecuid), "/etc/yuneng/ecuid.conf");
	get_local_time(local_time);
	strncpy(timestamp, &recvbuffer[34], 14);
	file_get_one(timezone, sizeof(timezone), "/etc/yuneng/timezone.conf");

	//拼接信息
	msg_Header(sendbuffer, "A104");
	msgcat_s(sendbuffer, 12, ecuid);
	msgcat_s(sendbuffer, 14, local_time);
	msgcat_s(sendbuffer, 14, timestamp);
	strcat(sendbuffer, timezone);
	msgcat_s(sendbuffer, 3, "END");

	return 0;
}

/* 【A105】EMA设置ECU本地时区 */
int set_time_zone(const char *recvbuffer, char *sendbuffer)
{
	int res, ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	char timezone[64] = {'\0'};
	char cmd[256] = {'\0'};

	//获取时间戳
	strncpy(timestamp, &recvbuffer[30], 14);
	//获取时区
	strncpy(timezone, &recvbuffer[44], strlen(&recvbuffer[44])-3);
	debug_msg("zone:%s", timezone);

	/*配置处理*/
	snprintf(cmd, sizeof(cmd), "cp /usr/share/zoneinfo/%s /etc/localtime", timezone);
	ack_flag = mysystem(cmd);
	if(ack_flag == SUCCESS){
		file_set_one(timezone, "/etc/yuneng/timezone.conf");//将时区保存到配置文件
		mysystem("killall main.exe");
		mysystem("killall client");
		mysystem("killall resmonitor");
		usleep(100000);
		mysystem("/home/applications/resmonitor &");
	}

	//拼接应答消息
	msg_ACK(sendbuffer, "A105", timestamp, ack_flag);

	return 104;
}
