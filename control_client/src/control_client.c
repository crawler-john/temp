#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "sqlite3.h"

#include "remote_control_protocol.h"
#include "mydebug.h"
#include "myfile.h"
#include "mysocket.h"
#include "mycommand.h"

#include "time_zone.h"
#include "comm_config.h"
#include "custom_command.h"
#include "ecu_flag.h"
#include "grid_quality.h"

#include "inverter_id.h"
#include "inverter_ac_protection.h"
#include "inverter_maxpower.h"
#include "inverter_onoff.h"
#include "inverter_gfdi.h"
#include "inverter_grid_environment.h"
#include "inverter_ird.h"
#include "inverter_restore.h"
#include "inverter_signal_strength.h"
#include "inverter_update.h"
#include "3501uid.h"
#include "set_autoflag_report.h"
#include "encryption.h"

//#define DEBUG 0 //无
#define DEBUG 1 //调试信息
//#define DEBUG 2 //日志文件
//#define DEBUG 3 //调试信息+日志文件
#define ARRAYNUM 6
#define MAXBUFFER 65535
#define FIRST_TIME_CMD_NUM 12

typedef struct socket_config
{
	int timeout;
	int report_interval;
	int port1;
	int port2;
	char domain[32];
	char ip[16];
}Socket_Cfg;

enum CommandID{
	A100, A101, A102, A103, A104, A105, A106, A107, A108, A109, //0-9
	A110, A111, A112, A113, A114, A115, A116, A117, A118, A119, //10-19
	A120, A121, A122, A123, A124, A125, A126, A127, A128, A129, //20-29
	A130, A131, A132, A133, A134, A135, A136, A137, A138, A139, //30-39
	A140, A141, A142, A143, A144, A145, A146, A147, A148, A149,
	A150,A151,
};
int (*pfun[100])(const char *recvbuffer, char *sendbuffer);
Socket_Cfg sockcfg = {'\0'};

void add_functions()
{
    pfun[A102] = response_inverter_id; 			//上报逆变器ID
    pfun[A103] = set_inverter_id; 				//设置逆变器ID
    pfun[A104] = response_time_zone; 			//上报ECU本地时区
	pfun[A105] = set_time_zone; 				//设置ECU本地时区
	pfun[A106] = response_comm_config;			//上报ECU的通信配置参数
	pfun[A107] = set_comm_config;				//设置ECU的通信配置参数
	pfun[A108] = custom_command;				//向ECU发送自定义命令
	pfun[A109] = set_inverter_ac_protection_5; 	//设置逆变器的交流保护参数(5项)
	pfun[A110] = set_inverter_maxpower;			//设置逆变器最大功率
	pfun[A111] = set_inverter_onoff;			//设置逆变器开关机
	pfun[A112] = clear_inverter_gfdi;			//设置逆变器GFDI
	pfun[A113] = response_ecu_ac_protection_5;	//上报ECU级别交流保护参数(5项)
	pfun[A114] = read_inverter_ac_protection_5; //读取逆变器的交流保护参数(5项)
	pfun[A117] = response_inverter_maxpower;	//上报逆变器最大功率及范围
	pfun[A119] = set_ecu_flag;					//设置ECU与EMA的通信开关
	pfun[A120] = response_ecu_ac_protection_13;	//上报ECU级别交流保护参数(13项)
	pfun[A121] = read_inverter_ac_protection_13;//读取逆变器的交流保护参数(13项)
	pfun[A122] = set_inverter_ac_protection_13;	//设置逆变器的交流保护参数(13项)
	pfun[A124] = read_inverter_grid_environment;//读取逆变器的电网环境
	pfun[A125] = set_inverter_grid_environment;	//设置逆变器的电网环境
	pfun[A126] = read_inverter_ird;				//读取逆变器的IRD选项
	pfun[A127] = set_inverter_ird;				//设置逆变器的IRD选项
	pfun[A128] = read_inverter_signal_strength;	//读取逆变器的信号强度
	pfun[A129] = response_grid_quality;			//上报系统的电网质量
	pfun[A130] = response_ecu_ac_protection_17;	//上报ECU级别交流保护参数(17项)
	pfun[A131] = read_inverter_ac_protection_17;//读取逆变器的交流保护参数(17项)
	pfun[A132] = set_inverter_ac_protection_17;	//设置逆变器的交流保护参数(17项)
	pfun[A134] = set_inverter_restore;			//设置逆变器的还原标志
	pfun[A136] = set_inverter_update;			//设置逆变器的升级标志
	pfun[A138] = set_autoflag_report;			//设置ECU自动上报功能
	pfun[A140] = response_inverter_encryption;	//上报逆变器防盗状态
	pfun[A141] = set_encryption;				//重置防盗状态
	pfun[A142] = clear_encryption;				//擦除防盗状态
	pfun[A148] = read_wrong_id;					//读取异常的3501uid
//	pfun[A149] = response_wrong_id;				//上报异常的3501uid
	pfun[A150] = set_unnormal_id;				//设置3501正确的id
//	pfun[A151] = response_changed_id;			//上报修改成功的id
}

/* 分析命令行参数 */
int getoptions(int argc, char *argv[])
{
    signed char ch;
    int debug_flag = DEBUG; //调试标志位

    while((ch = getopt(argc, argv, "df")) != -1){
        switch (ch) {
            case 'd': debug_flag |= (1<<0); break; //开启调试信息
            case 'f': debug_flag |= (1<<1); break; //开启日志文件
            default :printf("Usage: control_client [OPTION]...\n"
            				"Options: -d    Print debug information\n"
            				"         -f    Use Log file\n"); exit(0); break; //打印使用方式
        }
    }
    return debug_flag;
}

/* 【A118】ECU初次连接EMA需要执行的打包命令 */
int first_time_info(const char *recvbuffer, char *sendbuffer)
{
	static int command_id = 0;
	int functions[FIRST_TIME_CMD_NUM] = {
			A102, A104, A106, A113, A114, A117,
			A120, A121, A124, A126, A130, A131,
			A140,
	};

	//调用函数
	(*pfun[functions[command_id++]%100])(recvbuffer, sendbuffer);
//	debug_msg("cmd:A%d", functions[command_id - 1] + 100);
	usleep(100000);

	if(command_id < FIRST_TIME_CMD_NUM)
		return 118;
	else
		return 0;
}

int check_inverter_abnormal_status_sent(int hour)
{
	sqlite3 *db;
	char **azResult = NULL;
	int nrow, ncolumn;
	char sql[1024] = {'\0'};
	int sockfd;
	int i, flag, num = 0;
	char ecuid[13] = {'\0'};
	char datetime[15] = {'\0'};
	char recv_buffer[4096] = {'\0'};
	char send_buffer[MAXBUFFER] = {'\0'};
	time_t tm;         //实例化time_t结构
	struct tm timenow; //实例化tm结构指针

	time(&tm);
	memcpy(&timenow, localtime(&tm), sizeof(timenow));
//	debug_msg("hour:%d", timenow.tm_hour);
	if(timenow.tm_hour != hour)
		return 0;

	file_get_one(ecuid, sizeof(ecuid), "/etc/yuneng/ecuid.conf");
	if(!open_db("/home/record.db", &db))
	{
		//查询是否有flag=2的数据
		snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM inverter_status WHERE flag=2 ");
		if(!get_data(db, sql, &azResult, &nrow, &ncolumn)){
			if(azResult[1] != NULL){
				if(atoi(azResult[1]) <= 0){
					sqlite3_free_table(azResult);
					return 0;
				}
			}
			sqlite3_free_table(azResult);
		}

		//有flag=2的数据,发送一条读取EMA已存时间戳的命令
		debug_msg(">>Start Check abnormal status sent");
		sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
		strcpy(send_buffer, "APS13AAA51A123AAA0");
		strcat(send_buffer, ecuid);
		strcat(send_buffer, "000000000000000000END\n");
		send_socket(sockfd, send_buffer, strlen(send_buffer));

		//接收EMA的应答
		if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) <= 0){
			close_db(db);
			close(sockfd);
			return 0;
		}
		//校验命令
		if(msg_format_check(recv_buffer) < 0){
			close_db(db);
			close(sockfd);
			return 0;
		}
		//解析收到的时间戳,并删除EMA已存的数据
		flag = msg_get_int(&recv_buffer[18], 1);
		num = 0;
		if(flag){
			num = msg_get_int(&recv_buffer[19], 2);
			for(i=0; i<num; i++){
				strncpy(datetime, &recv_buffer[21 + i*14], 14);
				memset(sql, 0, sizeof(sql));
				snprintf(sql, sizeof(sql), "DELETE FROM inverter_status "
						"WHERE date_time='%s' ", datetime);
				delete_data(db, sql);
			}
		}

		//将flag=2的数据改为flag=1
		memset(sql, '\0', sizeof(sql));
		snprintf(sql, sizeof(sql), "UPDATE inverter_status SET flag=1 WHERE flag=2");
		update_data(db, sql);

		close_db(db);
		close(sockfd);
	}
	return 0;
}

/* 从数据库中查询是否存在逆变器异常状态 */
int exist_inverter_abnormal_status()
{
	sqlite3 *db;
	char **azResult = NULL;
	int nrow, ncolumn;
	char sql[1024] = {'\0'};
	int result = 0;

	if(!open_db("/home/record.db", &db))
	{
		//查询该逆变器ID在表中是否存在(建议建表的时候以‘id’为主键，方便使用REPLASE插入)
		snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM inverter_status WHERE flag=1 ");
		if(!get_data(db, sql, &azResult, &nrow, &ncolumn)){
			if(azResult[1] != NULL){
				if(atoi(azResult[1]) > 0){
					result = 1;
				}
			}
			sqlite3_free_table(azResult);
		}
		close_db(db);
	}
	return result;
}

/* 【A123】ECU上报逆变器的异常状态 */
int response_inverter_abnormal_status()
{
	sqlite3 *db;
	int nrow, ncolumn;
	char **azResult = NULL;
	char sql[1024] = {'\0'};
	int result = 0;
	int i, j, sockfd, item, flag, num, cmd_id, next_cmd_id;
	char datetime[15] = {'\0'};
	char recv_buffer[4096] = {'\0'};
	char command[4096] = {'\0'};
	char save_buffer[MAXBUFFER] = {'\0'};

	debug_msg(">>Start Response Abnormal Status");
	if(!open_db("/home/record.db", &db))
	{
		//查询所有逆变器异常状态
		nrow = 0;
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "SELECT item,result FROM inverter_status WHERE flag=1");
		if(get_data(db, sql, &azResult, &nrow, &ncolumn) != 0){
			close_db(db);
			return 0;
		}

		//建立socket连接
		sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);

		//逐条发送逆变器异常状态
		for(i=1; i<=nrow; i++)
		{
			//发送一条逆变器异常状态信息
			if(send_socket(sockfd, azResult[i*ncolumn + 1], strlen(azResult[i*ncolumn + 1])) < 0){
				continue;
			}
			//接收EMA的应答
			if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) <= 0){
				sqlite3_free_table(azResult);
				close_db(db);
				close(sockfd);
				return 0;
			}
			//校验命令
			if(msg_format_check(recv_buffer) < 0){
				continue;
			}
			//将发送与接收都成功那一条状态的标志置2
			item = atoi(azResult[i*ncolumn]);
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "UPDATE inverter_status SET flag=2 WHERE item=%d", item);
			update_data(db, sql);
			//解析收到的时间戳,并删除EMA已存的数据
			flag = msg_get_int(&recv_buffer[18], 1);
			num = 0;
			if(flag){
				num = msg_get_int(&recv_buffer[19], 2);
				for(j=0; j<num; j++){
					strncpy(datetime, &recv_buffer[21 + j*14], 14);
					memset(sql, 0, sizeof(sql));
					snprintf(sql, sizeof(sql), "DELETE FROM inverter_status "
							"WHERE date_time='%s' ", datetime);
					delete_data(db, sql);
				}
			}
			//判断应答帧是否附带命令
			if(strlen(recv_buffer) > (24 + 14*num)){
				memset(command, 0, sizeof(command));
				strncpy(command, &recv_buffer[24 + 14*num], sizeof(command));
				debug_msg("Command:%s", command);
				//校验命令
				if(msg_format_check(command) < 0)
					continue;
				//解析命令号
				cmd_id = msg_cmd_id(command);

				if(cmd_id==118)
				{
					char da_time[20]={'\0'};
					strncpy(da_time, &recv_buffer[72],14);
					FILE *fp;
					fp=fopen("/etc/yuneng/A118.conf","w");
					if(fp==NULL)
						return -1;
					else
						{
							fputs("1",fp);
							fclose(fp);
							char send_buffer[1024]={'\0'};
							msg_ACK(send_buffer, "A118", da_time, 0);
							send_socket(sockfd, send_buffer, strlen(send_buffer));
							debug_msg(">>End\n");debug_msg("socked=%d",sockfd);
							result=1;break;
						}
				}

				//调用函数
				else if(pfun[cmd_id%100]){
					next_cmd_id = (*pfun[cmd_id%100])(command, save_buffer);
					save_to_process_result(cmd_id, save_buffer);
					if(next_cmd_id > 0){
						memset(command, 0, sizeof(command));
						snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0000000000000A%3d00000000000000END", next_cmd_id);
						(*pfun[next_cmd_id%100])(command, save_buffer);
						save_to_process_result(next_cmd_id, save_buffer);
					}
					else if(next_cmd_id < 0){
						result = -1;
					}
				}
			}
		}
		sqlite3_free_table(azResult);
		close_db(db);
		close(sockfd);
	}
	return result;
}

/* 与EMA进行通信 */
int communication_with_EMA(int next_cmd_id)
{printf("mn-%d\n",__LINE__);
	int sockfd;
	int cmd_id;
	char msg_length[6] = {'\0'};
	char ecuid[13] = {'\0'};
	char timestamp[15] = "00000000000000";
	char recv_buffer[4096] = {'\0'};
	char send_buffer[MAXBUFFER] = {'\0'};
	int one_a118=0;

	file_get_one(ecuid, sizeof(ecuid), "/etc/yuneng/ecuid.conf");

	while(1)
	{printf("mn-%d\n",__LINE__);
		debug_msg(">>Start Communication with EMA");
		sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
		if(next_cmd_id <= 0)
		{
			//ECU向EMA发送请求命令的指令
			msg_REQ(send_buffer);
			send_socket(sockfd, send_buffer, strlen(send_buffer));
			memset(send_buffer, '\0', sizeof(send_buffer));

			//接收EMA发来的命令
			if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) < 0){
				close(sockfd);
				break;
			}

			//校验命令
			if(msg_format_check(recv_buffer) < 0){
				close(sockfd);
				continue;
			}

			//解析命令号
			cmd_id = msg_cmd_id(recv_buffer);
		}
		else{
			//生成下一条命令（用于设置命令结束后，上报设置后的ECU状态）
			cmd_id = next_cmd_id;
			next_cmd_id = 0;
			memset(recv_buffer, 0, sizeof(recv_buffer));
			snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0%.12sA%3d%.14sEND",
					ecuid, cmd_id, timestamp);
		}

		//ECU注册后初次与EMA通信
		if(cmd_id == 118){
			if(one_a118==0){
				one_a118=1;
				system("rm /etc/yuneng/fill_up_data.conf");
				system("echo '1'>>/etc/yuneng/fill_up_data.conf");
				system("killall main.exe");
			}
			strncpy(timestamp, &recv_buffer[34], 14);
			next_cmd_id = first_time_info(recv_buffer, send_buffer);
			if(next_cmd_id == 0){
				strncpy(timestamp, "00000000000000", 14);
			}
		}
		//根据命令号调用函数
		else if(pfun[cmd_id%100]){printf("mn-%d\n",__LINE__);
			//若设置函数调用完毕后需要执行上报，则会返回上报函数的命令号，否则返回0
			next_cmd_id = (*pfun[cmd_id%100])(recv_buffer, send_buffer);
		}
		//EMA命令发送完毕
		else if(cmd_id == 100){
			close(sockfd);
			break;
		}
		else{
			//若命令号不存在，则发送设置失败应答（每条设置协议的时间戳位置不统一，返回时间戳是个问题...）
			memset(send_buffer, 0, sizeof(send_buffer));
			snprintf(send_buffer, 52+1, "APS13AAA52A100AAA0%sA%3d000000000000002END",
					ecuid, cmd_id);
		}

		//将消息发送给EMA（自动计算长度，补上回车）
		send_socket(sockfd, send_buffer, strlen(send_buffer));
		debug_msg(">>End\n");
		close(sockfd);

		//如果功能函数返回值小于0，则返回-1，程序会自动退出
		if(next_cmd_id < 0){
			return -1;
		}
	}
	debug_msg(">>End\n");
	return 0;
}

/* 上报process_result表中的信息 */
int response_process_result()
{
	sqlite3 *db;
	int nrow, ncolumn;
	char **azResult = NULL;
	char sql[1024] = {'\0'};
	char ecuid[13] = {'\0'};
	char sendbuffer[MAXBUFFER] = {'\0'};
	int sockfd, item, num, i;
	int item_num[32] = {0};

	file_get_one(ecuid, sizeof(ecuid), "/etc/yuneng/ecuid.conf");
	if(!open_db("/home/database.db", &db))
	{
		//查询所有[ECU级别]处理结果
		nrow = 0;
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "SELECT item,result FROM process_result WHERE flag=1");
		if(get_data(db, sql, &azResult, &nrow, &ncolumn) != 0){
			close_db(db);
			return 0;
		}
		//逐条上报ECU级别处理结果
		for(i=1; i<=nrow; i++)
		{
			debug_msg(">>Start Response ECU Process Result");
			sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
			//发送一条记录
			if(send_socket(sockfd, azResult[i*ncolumn + 1], strlen(azResult[i*ncolumn + 1])) < 0){
				close(sockfd);
				continue;
			}
			//发送成功则将标志位置0
			item = atoi(azResult[i*ncolumn]);
			memset(sql, 0, sizeof(sql));
			snprintf(sql, sizeof(sql), "UPDATE process_result SET flag=0 WHERE item=%d", item);
			update_data(db, sql);
			close(sockfd);
			debug_msg(">>End");
		}
		if(nrow)sqlite3_free_table(azResult);

		//查询[逆变器级别]处理结果的命令号,存入数组
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "SELECT distinct(item) FROM inverter_process_result WHERE flag=1");
		if(get_data(db, sql, &azResult, &nrow, &ncolumn) != 0){
			close_db(db);
			return 0;
		}
		for(i=1; i<=nrow; i++)
		{
			item_num[i-1] = atoi(azResult[i]);
		}
		if(nrow)sqlite3_free_table(azResult);
		//逐条拼接并上报逆变器级别处理结果
		num = 0;
		while(item_num[num] != 0)
		{
			debug_msg("item:%d", item_num[num]);
			item = item_num[num++];
			memset(sql, '\0', sizeof(sql));
			snprintf(sql, sizeof(sql), "SELECT result FROM inverter_process_result WHERE item=%d and flag=1", item);
			if(get_data(db, sql, &azResult, &nrow, &ncolumn) != 0){
				close_db(db);
				return 0;
			}
			//拼接数据
			memset(sendbuffer, 0, sizeof(sendbuffer));
			if(item==139)
			{
				//int typ=0;
				//char type[4]={'0','0','0','0'};
				//file_get_one(type, 1, "/etc/yuneng/emergrncy_type.conf");
				//system("rm /etc/yuneng/emergrncy_type.conf");
				//typ=atoi(type);
				//printf("typ=%d",typ);
				char ttm[15]={'\0'};
				char ttm2[3]={'\0'};
				get_time(ttm,ttm2);
				sprintf(sendbuffer, "APS1300000A%03dAAA0%.12s%04d%.14sEND", item, ecuid, nrow,ttm);
			}
			else
				sprintf(sendbuffer, "APS1300000A%03dAAA0%.12s%04d00000000000000END", item, ecuid, nrow);
			for(i=1; i<=nrow; i++)
			{
				strcat(sendbuffer, azResult[i]);
			}
			if(nrow)sqlite3_free_table(azResult);
			//发送数据
			debug_msg(">>Start Response Inverter Process Result");
			sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
			if(send_socket(sockfd, sendbuffer, strlen(sendbuffer)) < 0){
				close(sockfd);
				continue;
			}
			memset(sql, 0, sizeof(sql));
			snprintf(sql, sizeof(sql), "UPDATE inverter_process_result SET flag=0 WHERE item=%d", item);
			update_data(db, sql);
			close(sockfd);
		}
		close_db(db);
	}
	return 0;
}

/* 将从文件中读取的键值对保存到socket配置结构体中 */
int get_socket_config(Socket_Cfg *cfg, MyArray *array)
{
	int i;

	for(i=0; i<ARRAYNUM; i++){
		if(!strlen(array[i].name))break;
		//超时时间
		if(!strcmp(array[i].name, "Timeout")){
			cfg->timeout = atoi(array[i].value);
		}
		//轮询时间
		else if(!strcmp(array[i].name, "Report_Interval")){
			cfg->report_interval = atoi(array[i].value);
		}
		//域名
		else if(!strcmp(array[i].name, "Domain")){
			strncpy(cfg->domain, array[i].value, 32);
		}
		//IP地址
		else if(!strcmp(array[i].name, "IP")){
			strncpy(cfg->ip, array[i].value, 16);
		}
		//端口1
		else if(!strcmp(array[i].name, "Port1")){
			cfg->port1 = atoi(array[i].value);
		}
		//端口2
		else if(!strcmp(array[i].name, "Port2")){
			cfg->port2 = atoi(array[i].value);
		}
	}
	return 0;
}

/* 随机取port1或port2 */
int randport(Socket_Cfg cfg)
{
	srand((unsigned)time(NULL));
	if(rand()%2)
		return cfg.port1;
	else
		return cfg.port2;
}

/* 获取当前时间的秒数 */
int get_ecu_time()
{
	return time((time_t*)NULL);
}

/* 主函数 */
int main(int argc, char *argv[])
{
	printf("mn-%d\n",__LINE__);
	int result, ecu_time = 0, ecu_flag = 1;
	char buffer[16] = {'\0'};
	const char *app_name = "control_client";
	const char *log_path = "/home/control_client.log";
	MyArray array[ARRAYNUM] = {'\0'};
	printf("mn-%d\n",__LINE__);
	/* 程序初始化 */
	//打印调试信息初始化
    debug_init(getoptions(argc, argv), app_name, log_path);
    //添加功能函数
    add_functions();
	//获取ECU的通讯开关flag
	if(file_get_one(buffer, sizeof(buffer), "/etc/yuneng/ecu_flag.conf")){
		ecu_flag = atoi(buffer);
	}
	debug_msg("ecu_flag = %d", ecu_flag);

	/* ECU轮询主循环 */
	while(1)
	{printf("mn-%d\n",__LINE__);
	    //从配置文件中获取socket通信参数
		if(file_get_array(array, ARRAYNUM, "/etc/yuneng/controlclient.conf") == 0){
			get_socket_config(&sockcfg, array);
		}
		//每天1点时向EMA确认逆变器异常状态是否被存储
		check_inverter_abnormal_status_sent(1);

		FILE *fp;
		fp=fopen("/etc/yuneng/A118.conf","r");
		if(fp!=NULL)
		{
			char c='0';
			c=fgetc(fp);
			if(c=='1')
				result = communication_with_EMA(118);
			fclose(fp);
			system("rm /etc/yuneng/A118.conf");
		}
		if(exist_inverter_abnormal_status() && ecu_flag){
			ecu_time = get_ecu_time();
			result = response_inverter_abnormal_status();
			response_process_result();
		}
		else if((get_ecu_time() - ecu_time) >= 60*sockcfg.report_interval){
			ecu_time = get_ecu_time();
			if(ecu_flag){ //如果ecu_flag = 0 则不上报处理结果
				response_process_result();
			}printf("mn-%d\n",__LINE__);debug_msg("qqq");
			result = communication_with_EMA(0);printf("mn-%d\n",__LINE__);
		}
		//程序自行退出
		if(result < 0){
			result = 0;
			debug_msg("Quit control_client");
			exit(0);
		}
		sleep(sockcfg.report_interval*60/3);
	}

    return 0;
}
