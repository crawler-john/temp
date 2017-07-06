#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"

extern ecu_info ecu;
extern sqlite3 *db;
int channel_need_change();
int getOldChannel();
int getNewChannel();
void saveECUChannel(int channel);
void changeChannelOfInverters(int oldChannel, int newChannel);

/* 信道操作 */
int process_channel()
{
	FILE *fp;
	int oldChannel, newChannel;

	if (channel_need_change()) {

		//获取变更前后的信道
		oldChannel = getOldChannel();
		newChannel = getNewChannel();

		//修改信道
		changeChannelOfInverters(oldChannel, newChannel);

		//将ECU的新信道存入配置文件
		saveECUChannel(newChannel);

		//清空标志位
		fp = fopen("/tmp/change_channel.conf", "w");
		fclose(fp);
	}
	return 0;
}

/* 判断是否需要改变信道 */
int channel_need_change()
{
	FILE *fp;
	char buff[2] = {'\0'};

	fp = fopen("/tmp/change_channel.conf", "r");
	if (fp) {
		fgets(buff, 2, fp);
		fclose(fp);
	}

	return ('1' == buff[0]);
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
	return 0; //未知信道
}
int getNewChannel()
{
	FILE *fp;
	char buffer[4] = {'\0'};

	fp = fopen("/tmp/new_channel.conf", "r");
	if (fp) {
		fgets(buffer, 4, fp);
		fclose(fp);
		return atoi(buffer);
	}
	return 16; //默认信道
}

void saveECUChannel(int channel)
{
	FILE *fp;
	char buffer[5] = {'\0'};

	snprintf(buffer, sizeof(buffer), "0x%02X", channel);
	printf("%s\n", buffer);
	fp = fopen("/etc/yuneng/channel.conf", "w");
	if (fp) {
		fputs(buffer, fp);
		fclose(fp);
	}
	ecu.channel = channel;
}

void changeChannelOfInverters(int oldChannel, int newChannel)
{
	char sql[1024] = {'\0'};
	char **azResult;
	char *zErrMsg = 0;
	int i, nChannel, nrow, ncolumn;

	//获取逆变器ID
	memset(sql, 0, sizeof(sql));
	snprintf(sql, sizeof(sql), "SELECT id FROM id WHERE flag=1 ");
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);

	//更改信道
	if (nrow > 0) {
		//原信道已知
		if (oldChannel) {
			//更改ECU信道为原信道
			zb_restore_ecu_panid_0xffff(oldChannel);

			//更改每台逆变器信道
			for (i=1; i<=nrow; i++) {
				zb_change_inverter_channel_one(azResult[i], newChannel);
				memset(sql, 0, sizeof(sql));
				snprintf(sql, sizeof(sql), "UPDATE id SET flag=0 WHERE id='%s' ", azResult[i]);
				sqlite3_exec(db, sql, 0, 0, &zErrMsg);
			}
		}
		//原信道未知
		else {
			//ECU在每一个信道都给每一台逆变器发送更改信道的指令
			for (nChannel=11; nChannel<=26; nChannel++ ) {
				zb_restore_ecu_panid_0xffff(nChannel);
				for (i=1; i<=nrow; i++) {
					zb_change_inverter_channel_one(azResult[i], newChannel);
				}
			}
			memset(sql, 0, sizeof(sql));
			snprintf(sql, sizeof(sql), "UPDATE id SET flag=0 ");
			sqlite3_exec(db, sql, 0, 0, &zErrMsg);
		}
	}

	//释放资源
	sqlite3_free_table(azResult);
}
