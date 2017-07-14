#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "mydebug.h"

static int DEBUG_FLAG = 1; //调试信息标志位
static const char *APP_NAME = NULL; //应用程序名称
static const char *LOG_PATH = NULL; //日志文件路径

/* 初始化 */
void debug_init(int flag, const char *name, const char *path)
{
	DEBUG_FLAG = flag; //调试信息标志位
	APP_NAME = name;   //应用程序名称
	LOG_PATH = path;   //日志文件路径
}

/* 获取系统当前时间 */
char *system_time(void)
{
	static char record_time[20] = {'\0'}; //用于输出时间的静态缓存(只被初始化一次)
	time_t now; //时间类型变量
	struct tm *local_time; //tm结构体指针

	time(&now); //读取当前时间
	local_time = localtime(&now); //转换成本地时间
	sprintf(record_time, "%04d-%02d-%02d %02d:%02d:%02d",
			local_time->tm_year + 1900,
			local_time->tm_mon + 1,
			local_time->tm_mday,
			local_time->tm_hour,
			local_time->tm_min,
			local_time->tm_sec); //将格式化后的时间存入静态缓存

	return record_time;
}

/* 打印调试信息 */
void debug_msg(const char *fmt, ...)
{
	FILE *fp;
	va_list arg_ptr; //定义 va_list型 变量
	va_start(arg_ptr, fmt); //用 va_start宏 初始化变量

	/* 打印调试信息 */
	if(DEBUG_FLAG & 0x01)
	{
		printf("[%s] ", APP_NAME);
		vprintf(fmt, arg_ptr);
		printf("\n");
	}

	/* 打印调试信息到日志文件 */
	if((DEBUG_FLAG>>1) & 0x01)
	{
		fp = fopen(LOG_PATH, "a");
		if(fp)
		{
			fprintf(fp, "(%s)", system_time());
			vfprintf(fp, fmt, arg_ptr);
			fprintf(fp, "\n");
			fclose(fp);
		}
	}
	va_end(arg_ptr); //用 va_end宏 结束可变参数的获取
}

/* 打印错误信息 */
void debug_err(const char *message)
{
	FILE *fp;

	/* 打印错误信息 */
	if(DEBUG_FLAG & 0x01)
	{
		perror(message);
	}

	/* 打印错误信息到日志文件 */
	if((DEBUG_FLAG>>1) & 0x01)
	{
		fp = fopen(LOG_PATH, "a");
		if(fp)
		{
			fprintf(fp, "%s: %s\n", message, strerror(errno));
			fclose(fp);
		}
	}
}
