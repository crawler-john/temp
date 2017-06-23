/*
 * 程序用于处理逆变器flash中的参数异常变化的问题。
 * Created by zhyf on 2016/06/16
 * Version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"
#include "debug.h"

extern int ecu_type;	//1:SAA; 2:NA; 3:MX
extern int caltype;		//计算方式，NA版和非NA版的区别
extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID

typedef struct protection_parameters_t{
	int under_voltage_fast;
	int over_voltage_fast;
	int under_voltage_stage_2;
	int under_voltage_slow;
	int over_voltage_slow;
	float under_frequency_fast;
	float over_frequency_fast;
	float under_frequency_slow;
	float over_frequency_slow;
	int mode;		//功率模式：0表示IPP，1表示MPPT
}protection_parameters;

protection_parameters paras;

int reset_paras()
{
	paras.under_voltage_fast = 0;
	paras.over_voltage_fast = 0;
	paras.under_voltage_stage_2 = 0;
	paras.under_voltage_slow = 0;
	paras.over_voltage_slow = 0;
	paras.under_frequency_fast = 0;
	paras.over_frequency_fast = 0;
	paras.under_frequency_slow = 0;
	paras.over_frequency_slow = 0;
	paras.mode = 1;

	return 0;
}

/*向逆变器读取设置的值*/
int get_paras_de(char *id)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xDE;
	sendbuff[21] = 0x11;

	for(i=2; i<22; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	write(plcmodem, sendbuff, 26);

	res = get_reply_from_serial(plcmodem, 2, 0, readbuff);

	if((37 == res) &&
			(0xFB == readbuff[0])&&
			(0xFB == readbuff[1])&&
			(0x01 == readbuff[2])&&
			(0x00 == readbuff[3])&&
			(0x1C == readbuff[4])&&
			(ccuid[0] == readbuff[5])&&
			(ccuid[1] == readbuff[6])&&
			(ccuid[2] == readbuff[7])&&
			(ccuid[3] == readbuff[8])&&
			(ccuid[4] == readbuff[9])&&
			(ccuid[5] == readbuff[10])&&
			(readbuff[11] == sendbuff[11]) &&
			(readbuff[12] == sendbuff[12]) &&
			(readbuff[13] == sendbuff[13]) &&
			(readbuff[14] == sendbuff[14]) &&
			(readbuff[15] == sendbuff[15]) &&
			(readbuff[16] == sendbuff[16]) &&
			(0x4F == readbuff[17])&&
			(0x00 == readbuff[18])&&
			(0x00 == readbuff[19])&&
			(0xDE == readbuff[20])&&
			(0xFE == readbuff[35])&&
			(0xFE == readbuff[36])){
		if(caltype)
			paras.under_voltage_slow = readbuff[28]*4/1.452;
		else
			paras.under_voltage_slow = readbuff[28]*4/1.48975;
		return 3;
	}
	else
		return 0;
}

/*解析参数，并保存到数据库中*/
int resolve_paras(char *readbuff, int size)
{
	int under_voltage_fast;
	int over_voltage_fast;
	int under_voltage_slow;
	int over_voltage_slow;
	float under_frequency_fast;
	float over_frequency_fast;
	float under_frequency_slow;
	float over_frequency_slow;
	float voltage_triptime_fast;
	float voltage_triptime_slow;
	float frequency_triptime_fast;
	float frequency_triptime_slow;
	float temp;

	reset_paras();

	if(42 == size){		//13项参数
		paras.mode = readbuff[21] & 0x01;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[24]*4/1.452;
		else
			temp = readbuff[24]*4/1.48975;

		if((temp-(int)temp)>=0.5)
			under_voltage_fast = (int)temp +1;
		else
			under_voltage_fast = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[25]*4/1.452;
		else
			temp = readbuff[25]*4/1.48975;

		if((temp-(int)temp)>=0.5)
			over_voltage_fast = (int)temp +1;
		else
			over_voltage_fast = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[26]*4/1.452;
		else
			temp = readbuff[26]*4/1.48975;

		if((temp-(int)temp)>0.5)
			under_voltage_slow = (int)temp +1;
		else
			under_voltage_slow = (int)temp;

		if((2 == caltype)||(1 == caltype))
			temp = readbuff[27]*4/1.452;
		else
			temp = readbuff[27]*4/1.48975;

		if((temp-(int)temp)>0.5)
			over_voltage_slow = (int)temp +1;
		else
			over_voltage_slow = (int)temp;

		if((2 == caltype)||(1 == caltype))
			under_frequency_fast = (600 - readbuff[28])/10.0;
		else
			under_frequency_fast = (500 - readbuff[28])/10.0;

		if((2 == caltype)||(1 == caltype))
			over_frequency_fast = (600 + readbuff[29])/10.0;
		else
			over_frequency_fast = (500 + readbuff[29])/10.0;

		if((2 == caltype)||(1 == caltype))
			under_frequency_slow = (600 - readbuff[30])/10.0;
		else
			under_frequency_slow = (500 - readbuff[30])/10.0;

		if((2 == caltype)||(1 == caltype))
			over_frequency_slow = (600 + readbuff[31])/10.0;
		else
			over_frequency_slow = (500 + readbuff[31])/10.0;

		if((2 == caltype)||(1 == caltype))
			voltage_triptime_fast = readbuff[32]*0.0083;
		else
			voltage_triptime_fast = readbuff[32]*0.01;

		if((2 == caltype)||(1 == caltype))
			voltage_triptime_slow = readbuff[33]*1.07;
		else
			voltage_triptime_slow = readbuff[33]*1.28;

		if((2 == caltype)||(1 == caltype))
			frequency_triptime_fast = readbuff[34]*0.033;
		else
			frequency_triptime_fast = readbuff[34]*0.04;

		if((2 == caltype)||(1 == caltype))
			frequency_triptime_slow = readbuff[35]*1.07;
		else
			frequency_triptime_slow = readbuff[35]*1.28;

		if((0 == (int)(voltage_triptime_fast)) && (0 == (int)(voltage_triptime_slow)) && (0 == (int)(frequency_triptime_fast)) &&
				(0 == (int)(frequency_triptime_slow)))
		{
			paras.under_voltage_fast = under_voltage_fast;
			paras.over_voltage_fast = over_voltage_fast;
			paras.under_voltage_stage_2 = under_voltage_slow;
			paras.over_voltage_slow = over_voltage_slow;
			paras.under_frequency_fast = under_frequency_fast;
			paras.over_frequency_fast = over_frequency_fast;
			paras.under_frequency_slow = under_frequency_slow;
			paras.over_frequency_slow = over_frequency_slow;

			return 3;
		}

		paras.under_voltage_fast = under_voltage_fast;
		paras.over_voltage_fast = over_voltage_fast;
		paras.under_voltage_slow = under_voltage_slow;
		paras.over_voltage_slow = over_voltage_slow;
		paras.under_frequency_fast = under_frequency_fast;
		paras.over_frequency_fast = over_frequency_fast;
		paras.under_frequency_slow = under_frequency_slow;
		paras.over_frequency_slow = over_frequency_slow;

		return 2;
	}
	if(37 == size){		//5项参数
		if((54336 != (readbuff[23]*256 + readbuff[24])) && (55593 != (readbuff[23]*256 + readbuff[24])))
			paras.mode = readbuff[21] & 0x01;

		if(2 == caltype)
			temp = (readbuff[25]*256 + readbuff[26])/2.93;
		else if(1 == caltype)
			temp = (readbuff[25]*256 + readbuff[26])/2.90345;
		else
			temp = (readbuff[25]*256 + readbuff[26])/1.48975;
		if((temp-(int)temp)>0.5)
			paras.under_voltage_slow = (int)temp +1;
		else
			paras.under_voltage_slow = (int)temp;

		if(2 == caltype)
			temp = (readbuff[27]*256 + readbuff[28])/2.93;
		else if(1 == caltype)
			temp = (readbuff[27]*256 + readbuff[28])/2.90345;
		else
			temp = (readbuff[27]*256 + readbuff[28])/1.48975;
		if((temp-(int)temp)>0.5)
			paras.over_voltage_slow = (int)temp +1;
		else
			paras.over_voltage_slow = (int)temp;

		if(caltype)
			paras.under_frequency_slow = (600-readbuff[29])/10.0;
		else
			paras.under_frequency_slow = (500-readbuff[29])/10.0;

		if(caltype)
			paras.over_frequency_slow = (600+readbuff[30])/10.0;
		else
			paras.over_frequency_slow = (500+readbuff[30])/10.0;

		return 1;
	}

	return 0;
}

/*向逆变器读取设置的值*/
int get_paras(char *id)
{
	unsigned char sendbuff[512]={'\0'};
	unsigned char readbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;
	int type;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x01;			//CMD
	sendbuff[3] = 0x00;			//LENGTH
	sendbuff[4] = 0x11;			//LENGTH
	sendbuff[5] = ccuid[0];		//ccuid
	sendbuff[6] = ccuid[1];		//ccuid
	sendbuff[7] = ccuid[2];		//ccuid
	sendbuff[8] = ccuid[3];		//ccuid
	sendbuff[9] = ccuid[4];		//ccuid
	sendbuff[10] = ccuid[5];		//ccuid
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xDD;
	sendbuff[21] = 0x11;

	for(i=2; i<22; i++)
	{
		check = check + sendbuff[i];
	}

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	write(plcmodem, sendbuff, 26);

	res = get_reply_from_serial(plcmodem, 2, 0, readbuff);

	if((42 == res) &&		//13项参数
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0x01 == readbuff[2]) &&
			(0x00 == readbuff[3]) &&
			(0x21 == readbuff[4]) &&
			(ccuid[0] == readbuff[5]) &&
			(ccuid[1] == readbuff[6]) &&
			(ccuid[2] == readbuff[7]) &&
			(ccuid[3] == readbuff[8]) &&
			(ccuid[4] == readbuff[9]) &&
			(ccuid[5] == readbuff[10]) &&
			(readbuff[11] == sendbuff[11]) &&
			(readbuff[12] == sendbuff[12]) &&
			(readbuff[13] == sendbuff[13]) &&
			(readbuff[14] == sendbuff[14]) &&
			(readbuff[15] == sendbuff[15]) &&
			(readbuff[16] == sendbuff[16]) &&
			(0x4F == readbuff[17]) &&
			(0x00 == readbuff[18]) &&
			(0x00 == readbuff[19]) &&
			(0xDA == readbuff[20]) &&
			(0xFE == readbuff[40]) &&
			(0xFE == readbuff[41])){
		type = resolve_paras(readbuff, res);
		if(3 == type)
			type = get_paras_de(id);
		return type;
	}
	else if((37 == res) &&		//5项参数
			(0xFB == readbuff[0])&&
			(0xFB == readbuff[1])&&
			(0x01 == readbuff[2])&&
			(0x00 == readbuff[3])&&
			(0x1C == readbuff[4])&&
			(ccuid[0] == readbuff[5])&&
			(ccuid[1] == readbuff[6])&&
			(ccuid[2] == readbuff[7])&&
			(ccuid[3] == readbuff[8])&&
			(ccuid[4] == readbuff[9])&&
			(ccuid[5] == readbuff[10])&&
			(readbuff[11] == sendbuff[11]) &&
			(readbuff[12] == sendbuff[12]) &&
			(readbuff[13] == sendbuff[13]) &&
			(readbuff[14] == sendbuff[14]) &&
			(readbuff[15] == sendbuff[15]) &&
			(readbuff[16] == sendbuff[16]) &&
			(0x4F == readbuff[17])&&
			(0x00 == readbuff[18])&&
			(0x00 == readbuff[19])&&
			(0xDD == readbuff[20])&&
			(0xFE == readbuff[35])&&
			(0xFE == readbuff[36])){
		type = resolve_paras(readbuff, res);
		return type;
	}
	else
		return 0;
}

//检查逆变器的电网环境，部分NA的ECU用于120V（MX）的电网，无法确定地区，只能根据实际电网电压确定
int check_grid_env(int vol)
{
	if(1 == ecu_type)		//SAA
		return 1;
	else
	{
		if((vol> 210) && (vol<270))		//240V电网
			return 2;
		if((vol>90) && (vol<150))		//120V电网
			return 3;
	}
}

int handle_paras(char *id, int env)
{
	int type=0;
	type = get_paras(id);
	if(1 == env)	//SAA
	{
		if(1 == type)	//5项
		{
			if((paras.under_voltage_slow<180) || (paras.under_voltage_slow>220))
				return -1;
			if((paras.over_voltage_slow<220) || (paras.over_voltage_slow>300))
				return -1;
			if((paras.under_frequency_slow<45) || (paras.under_frequency_slow>50))
				return -1;
			if((paras.over_frequency_slow<50) || (paras.over_frequency_slow>55))
				return -1;
		}
		if(2 == type)	//13项
		{
			if((paras.under_voltage_slow<150) || (paras.under_voltage_slow>220))
				return -1;
			if((paras.over_voltage_slow<220) || (paras.over_voltage_slow>300))
				return -1;
			if((paras.under_voltage_fast<100) || (paras.under_voltage_fast>220))
				return -1;
			if((paras.over_voltage_fast<220) || (paras.over_voltage_fast>320))
				return -1;
			if((paras.under_frequency_slow<45) || (paras.under_frequency_slow>50))
				return -1;
			if((paras.over_frequency_slow<50) || (paras.over_frequency_slow>55))
				return -1;
			if((paras.under_frequency_fast<45) || (paras.under_frequency_fast>50))
				return -1;
			if((paras.over_frequency_fast<50) || (paras.over_frequency_fast>55))
				return -1;
		}
		if(3 == type)	//17项
		{
			if((paras.under_voltage_slow<150) || (paras.under_voltage_slow>220))
				return -1;
			if((paras.over_voltage_slow<220) || (paras.over_voltage_slow>300))
				return -1;
			if((paras.under_voltage_fast<100) || (paras.under_voltage_fast>220))
				return -1;
			if((paras.over_voltage_fast<220) || (paras.over_voltage_fast>320))
				return -1;
			if((paras.under_frequency_slow<45) || (paras.under_frequency_slow>50))
				return -1;
			if((paras.over_frequency_slow<50) || (paras.over_frequency_slow>55))
				return -1;
			if((paras.under_frequency_fast<45) || (paras.under_frequency_fast>50))
				return -1;
			if((paras.over_frequency_fast<50) || (paras.over_frequency_fast>55))
				return -1;
			if((paras.under_voltage_stage_2<100) || (paras.under_voltage_stage_2>220))
				return -1;
		}
	}
	if(2 == env)	//NA
	{
		if(1 == type)
		{
			if((paras.under_voltage_slow<180) || (paras.under_voltage_slow>240))
				return -1;
			if((paras.over_voltage_slow<220) || (paras.over_voltage_slow>300))
				return -1;
			if((paras.under_frequency_slow<55) || (paras.under_frequency_slow>60))
				return -1;
			if((paras.over_frequency_slow<60) || (paras.over_frequency_slow>65))
				return -1;
		}
		if(2 == type)
		{
			if((paras.under_voltage_slow<150) || (paras.under_voltage_slow>240))
				return -1;
			if((paras.over_voltage_slow<220) || (paras.over_voltage_slow>300))
				return -1;
			if((paras.under_voltage_fast<100) || (paras.under_voltage_fast>240))
				return -1;
			if((paras.over_voltage_fast<220) || (paras.over_voltage_fast>320))
				return -1;
			if((paras.under_frequency_slow<55) || (paras.under_frequency_slow>60))
				return -1;
			if((paras.over_frequency_slow<60) || (paras.over_frequency_slow>65))
				return -1;
			if((paras.under_frequency_fast<55) || (paras.under_frequency_fast>60))
				return -1;
			if((paras.over_frequency_fast<60) || (paras.over_frequency_fast>65))
				return -1;
		}
		if(3 == type)
		{
			if((paras.under_voltage_slow<150) || (paras.under_voltage_slow>240))
				return -1;
			if((paras.over_voltage_slow<220) || (paras.over_voltage_slow>300))
				return -1;
			if((paras.under_voltage_fast<100) || (paras.under_voltage_fast>240))
				return -1;
			if((paras.over_voltage_fast<220) || (paras.over_voltage_fast>320))
				return -1;
			if((paras.under_frequency_slow<55) || (paras.under_frequency_slow>60))
				return -1;
			if((paras.over_frequency_slow<60) || (paras.over_frequency_slow>65))
				return -1;
			if((paras.under_frequency_fast<55) || (paras.under_frequency_fast>60))
				return -1;
			if((paras.over_frequency_fast<60) || (paras.over_frequency_fast>65))
				return -1;
			if((paras.under_voltage_stage_2<100) || (paras.under_voltage_stage_2>240))
				return -1;
		}
	}
	if(3 == env)	//MX
	{
		if(1 == type)
		{
			if((paras.under_voltage_slow<80) || (paras.under_voltage_slow>120))
				return -1;
			if((paras.over_voltage_slow<120) || (paras.over_voltage_slow>180))
				return -1;
			if((paras.under_frequency_slow<55) || (paras.under_frequency_slow>60))
				return -1;
			if((paras.over_frequency_slow<60) || (paras.over_frequency_slow>65))
				return -1;
		}
		if(2 == type)
		{
			if((paras.under_voltage_slow<80) || (paras.under_voltage_slow>120))
				return -1;
			if((paras.over_voltage_slow<120) || (paras.over_voltage_slow>180))
				return -1;
			if((paras.under_voltage_fast<50) || (paras.under_voltage_fast>120))
				return -1;
			if((paras.over_voltage_fast<120) || (paras.over_voltage_fast>170))
				return -1;
			if((paras.under_frequency_slow<55) || (paras.under_frequency_slow>60))
				return -1;
			if((paras.over_frequency_slow<60) || (paras.over_frequency_slow>65))
				return -1;
			if((paras.under_frequency_fast<55) || (paras.under_frequency_fast>60))
				return -1;
			if((paras.over_frequency_fast<60) || (paras.over_frequency_fast>65))
				return -1;
		}
		if(3 == type)
		{
			if((paras.under_voltage_slow<80) || (paras.under_voltage_slow>120))
				return -1;
			if((paras.over_voltage_slow<120) || (paras.over_voltage_slow>180))
				return -1;
			if((paras.under_voltage_fast<70) || (paras.under_voltage_fast>120))
				return -1;
			if((paras.over_voltage_fast<120) || (paras.over_voltage_fast>160))
				return -1;
			if((paras.under_frequency_slow<55) || (paras.under_frequency_slow>60))
				return -1;
			if((paras.over_frequency_slow<60) || (paras.over_frequency_slow>65))
				return -1;
			if((paras.under_frequency_fast<55) || (paras.under_frequency_fast>60))
				return -1;
			if((paras.over_frequency_fast<60) || (paras.over_frequency_fast>65))
				return -1;
			if((paras.under_voltage_stage_2<80) || (paras.under_voltage_stage_2>120))
				return -1;
		}
	}

	return 0;
}

int save_turned_off_operation(char *id)
{
	char sql[1024]={'\0'};
	sqlite3 *db=NULL;

	if(SQLITE_OK != sqlite3_open("/home/operation.db", &db))
		return -1;

	sprintf(sql, "INSERT INTO turned_on_off_operation (id, turned_off_opreation) VALUES('%s', 1)", id);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int clear_turned_off_operation(char *id)
{
	char sql[1024]={'\0'};
	sqlite3 *db=NULL;

	if(SQLITE_OK != sqlite3_open("/home/operation.db", &db))
		return -1;

	sprintf(sql, "DELETE FROM turned_on_off_operation WHERE id='%s'", id);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

	return 0;
}

int get_last_turned_on_off_operation(char *id)
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;
	sqlite3 *db=NULL;
	int result=0;

	if(SQLITE_OK != sqlite3_open("/home/operation.db", &db))
		return -1;

	sprintf(sql, "SELECT turned_off_opreation FROM turned_on_off_operation WHERE id='%s'", id);
	if((SQLITE_OK == sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg)) && (nrow > 0))
		result = atoi(azResult[1]);

	sqlite3_free_table( azResult );
	sqlite3_close( db );

	return result;
}

int handle_turned_off(struct inverter_info_t *inverter)		//处理逆变器flash突变后关机的问题
{
	int i;

	if('1' == inverter->status_web[12])
	{
		if(!get_last_turned_on_off_operation(inverter->inverterid))
		{
			for(i=0; i<5; i++)
			{
				if(1 == wakeup(inverter))
					break;
			}
		}
	}

	return 0;
}

int handle_max_power(struct inverter_info_t *curinverter)		//处理逆变器flash突变后工作在定IPP模式的问题
{
	int limitedpower, i, res;
	char limitedvalue;
	char readpresetdata[20] = {'\0'};
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;
	sqlite3 *db;

	res = sqlite3_open("/home/database.db", &db);
	if(SQLITE_OK != res){		//create a database
		return -1;
	}

	sprintf(sql, "SELECT limitedpower FROM power WHERE id='%s'", curinverter->inverterid);
	if((SQLITE_OK == sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg)) && (nrow > 0) && (azResult[1]))
		limitedpower = atoi(azResult[1]);
	else
		limitedpower = 258;
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	if(0 == curinverter->inverter_with_13_parameters){
		for(i=0; i<3; i++){
			if(-1 != askpresetdatacmd(curinverter, readpresetdata, 0x11))
				break;
		}
	}
	if(0 != curinverter->inverter_with_13_parameters)
	{
		if(1 == curinverter->inverter_with_13_parameters)
			limitedvalue = (limitedpower * 7395) >> 14;
		if(2 == curinverter->inverter_with_13_parameters)
			limitedvalue = (limitedpower * 7395) >> 16;
		for(i=0; i<3; i++)
		{
			setlimitedpowerone(curinverter, limitedvalue);
			memset(readpresetdata, '\0', 20);
			res = askpresetdatacmd(curinverter, readpresetdata, 0x11);
			if((-1 != res) && (limitedvalue == readpresetdata[1])){
				break;
			}
		}
	}
}

int process_paras_changed(struct inverter_info_t *firstinverter)
{
	int i, env;
	char inverter_result[65535]={'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	printmsg("/*process_paras_changed*/");
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
	{
		if((curinverter->curacctime>600) && (!curinverter->processed_paras_changed_flag))
		{
			printdecmsg(curinverter->inverterid, curinverter->processed_paras_changed_flag);
			env = check_grid_env(curinverter->nv);
			if(-1 == handle_paras(curinverter->inverterid, env))
			{
				memset(inverter_result, '\0', sizeof(inverter_result));
				sprintf(inverter_result, "%s%03d%03d%03d%03d%03d%03d%03d%03d%03dEND",
						curinverter->inverterid, paras.under_voltage_slow, paras.over_voltage_slow, (int)(paras.under_frequency_slow*10), (int)(paras.over_frequency_slow*10),
						paras.under_voltage_fast, paras.over_voltage_fast, (int)(paras.under_frequency_fast*10),
						(int)(paras.over_frequency_fast*10), paras.under_voltage_stage_2);
				save_inverter_parameters_result_id(curinverter->inverterid, 137, inverter_result);
			}
			if(0 == paras.mode)
				handle_max_power(curinverter);
			handle_turned_off(curinverter);
			curinverter->processed_paras_changed_flag = 1;
		}
	}
	printmsg("/*process_paras_changed*/");

	return 0;
}
