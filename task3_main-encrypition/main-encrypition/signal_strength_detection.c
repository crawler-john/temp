#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID
int flag=0;					//是否需要上传EMA的标志,1表示上传，0不上传

int show_grid_qualtity(char quality)
{
	FILE *fp;
	int result;		//最后保存的结果，取高4位。0x4A，最后显示等级为4。最到等级为5
	
	result = quality >> 4;
	if(result > 5)
		result = 5;

	fp = fopen("/etc/yuneng/plc_grid_quality.txt", "w");
	if(fp){
		fprintf(fp, "%d", result);
		fclose(fp);
	}
	
	return 0;
}

int clear_all_flag()
{
	FILE *fp;

	fp = fopen("/tmp/read_all_signal_strength.conf", "w");
	if(fp){
		fclose(fp);
	}

	return 0;
}

int get_upload_flag()
{
	FILE *fp;
	char temp[256]={'\0'};

	fp = fopen("/tmp/upload_signal_strength.conf", "r");
	if(fp){
		fgets(temp, sizeof(temp), fp);
		fclose(fp);
		flag = atoi(temp);
	}
	else
		flag = 0;

	return 0;
}

int clear_upload_flag()
{
	FILE *fp;

	fp = fopen("/tmp/upload_signal_strength.conf", "w");
	if(fp){
		fclose(fp);
	}

	flag = 0;

	return 0;
}

int save_signal_strength(int strength, char *id)
{
	sqlite3 *db=NULL;
	char sql[1024];
	char inverter_result[256]={'\0'};

	sprintf(sql, "REPLACE INTO signal_strength (id, signal_strength) VALUES('%s', %d)", id, strength);

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );
	
	if(flag){
		sprintf(inverter_result, "%s%03dEND", id, strength);
		save_inverter_parameters_result2(id, 128, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA
	}

	return 0;
}

int send_signal_strength_test(char *id)		//ccid：ECU上3501的UID；tnid：逆变器上3501的UID；cmd：命令；data：保存逆变器返回的有效数据
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
	char data[50] = {'\0'};
	int i, j, res=0;
	fd_set rd;
	struct timeval timeout;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//CCID
	sendbuff[6] = ccuid[1];		//CCID
	sendbuff[7] = ccuid[2];		//CCID
	sendbuff[8] = ccuid[3];		//CCID
	sendbuff[9] = ccuid[4];		//CCID
	sendbuff[10] = ccuid[5];		//CCID
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xBB;			//CMD
	sendbuff[21] = 0x11;

	for(i=2; i<22; i++)
		check = check + sendbuff[i];

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 26);

	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		return -1;
	}
	else{
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		if((res == 47) && (readbuff[11] == sendbuff[11]) && (readbuff[12] == sendbuff[12]) && (readbuff[13] == sendbuff[13]) && (readbuff[14] == sendbuff[14]) && (readbuff[15] == sendbuff[15]) && (readbuff[16] == sendbuff[16]) && (0xFE == readbuff[45]) && (0xFE == readbuff[46])){		//是否47个字节，且是否是对应的逆变器
			return 1;
		}
		else if((res == 51) && (readbuff[11] == sendbuff[11]) && (readbuff[12] == sendbuff[12]) && (readbuff[13] == sendbuff[13]) && (readbuff[14] == sendbuff[14]) && (readbuff[15] == sendbuff[15]) && (readbuff[16] == sendbuff[16]) && (0xFE == readbuff[49]) && (0xFE == readbuff[50])){		//YC500
			return 1;
		}
		else{
			return -1;
		}
	}

	return 0;
}

int get_grid_quality()
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int count=0, res=0;
	int quality, strength;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[count++] = 0xFB;			//HEAD
	sendbuff[count++] = 0xFB;			//HEAD
	sendbuff[count++] = 0xF9;			//CMD
	sendbuff[count++] = 0x00;			//LENGTH
	sendbuff[count++] = 0x06;			//LENGTH
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0xFF;
	sendbuff[count++] = 0xFE;
	sendbuff[count++] = 0xFE;

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, count);
	
	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		print2msg("Get signal quality timeout");
		return -1;
	}
	else{
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		if((res == 25) &&
			(0xFB == readbuff[0]) && 
			(0xFB == readbuff[1]) && 
			(0xF9 == readbuff[2]) && 
			(0x00 == readbuff[3]) && 
			(0x10 == readbuff[4]) && 
			(0xFE == readbuff[23]) && 
			(0xFE == readbuff[24])){		//是否47个字节，且是否是对应的逆变器
			show_grid_qualtity(readbuff[18]);
			return 1;
		}
	}
	
	return -1;
}

int get_signal_strength(char *id)
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned char readbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int count=0, res=0;
	int quality, strength;
	fd_set rd;
	struct timeval timeout;
	
	sendbuff[count++] = 0xFB;			//HEAD
	sendbuff[count++] = 0xFB;			//HEAD
	sendbuff[count++] = 0xF9;			//CMD
	sendbuff[count++] = 0x00;			//LENGTH
	sendbuff[count++] = 0x06;			//LENGTH
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0x00;
	sendbuff[count++] = 0xFF;
	sendbuff[count++] = 0xFE;
	sendbuff[count++] = 0xFE;
	
	if(1 == send_signal_strength_test(id)){
		FD_ZERO(&rd);
		FD_SET(plcmodem, &rd);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		write(plcmodem, sendbuff, count);

		res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
		if(res <= 0){
			print2msg("Get signal quality timeout");
			return -1;
		}
		else{
			memset(readbuff, '\0', 255);
			res = read(plcmodem, readbuff, 255);
			if((res == 25) &&
				(0xFB == readbuff[0]) &&
				(0xFB == readbuff[1]) &&
				(0xF9 == readbuff[2]) &&
				(0x00 == readbuff[3]) &&
				(0x10 == readbuff[4]) &&
				(0xFE == readbuff[23]) &&
				(0xFE == readbuff[24])){		//是否47个字节，且是否是对应的逆变器
				save_signal_strength(readbuff[20], id);
				printhexmsg("Signal quality", readbuff, res);
				return 1;
			}
		}
	}
	
	return -1;
}

int clear_signal_strength_flag(char *id)					//设置后清除数据库中参数的设置标志
{
	sqlite3 *db=NULL;
	char sql[1024];

	sprintf(sql, "UPDATE signal_strength SET set_flag=0 WHERE id='%s'", id);

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int get_inverter_with_signal_strength_flag(char *id)
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	sqlite3 *db;
	
	if(SQLITE_OK != sqlite3_open("/home/database.db", &db)){		//create a database
		return -1;
	}

	strcpy(sql, "SELECT id FROM signal_strength WHERE set_flag=1 LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		strcpy(id, azResult[1]);
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	if(1 == nrow)
		return 1;
	else
		return 0;
}

int get_all_signal_strength(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	char temp[16]={'\0'};
	char inverter_id[16];
	struct inverter_info_t *inverter = firstinverter;
	int i;

	get_upload_flag();
	get_grid_quality();

	memset(temp, '\0', sizeof(temp));
	fp = fopen("/tmp/read_all_signal_strength.conf", "r");
	if(fp){
		fgets(temp, sizeof(temp), fp);
		fclose(fp);
	}
	clear_all_flag();
	if(!strncmp(temp, "ALL", 3)){
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++, inverter++)
			get_signal_strength(inverter->inverterid);
	}

	while(1){
		if(1 != get_inverter_with_signal_strength_flag(inverter_id)){
			break;
		}
		else{
			clear_signal_strength_flag(inverter_id);
			get_signal_strength(inverter_id);
		}
	}

	clear_upload_flag();
	
	return 0;
}
