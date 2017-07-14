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

typedef struct protection_parameters_set_t{
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
}protection_parameters_set;

protection_parameters_set paras_set;



int reset_paras_set()
{
	paras_set.under_voltage_fast = 0;
	paras_set.over_voltage_fast = 0;
	paras_set.under_voltage_stage_2 = 0;
	paras_set.under_voltage_slow = 0;
	paras_set.over_voltage_slow = 0;
	paras_set.under_frequency_fast = 0;
	paras_set.over_frequency_fast = 0;
	paras_set.under_frequency_slow = 0;
	paras_set.over_frequency_slow = 0;
	paras_set.mode = 1;

	return 0;
}

int saveinttoset_protection_parameters_inverter(sqlite3 *db,char *para_id,char *para_name,int value)
{
	char *zErrMsg = 0;
	if(value==-1)
		return -1;
	char sql[1024]={'\0'};

	sprintf(sql,"REPLACE INTO set_protection_parameters_inverter (id,parameter_name,parameter_value,set_flag) VALUES('%s','%s','%d',1)",para_id,para_name,value);
	sqlite3_exec(db, sql , 0, 0, &zErrMsg);print2msg("SQL:::",sql);
	return 0;
}

int savefloattoset_protection_parameters_inverter(sqlite3 *db,char *para_id,char *para_name,float value)
{
	char *zErrMsg = 0;
	if(value==-1)
		return -1;
	char sql[1024]={'\0'};
	sprintf(sql,"REPLACE INTO set_protection_parameters_inverter (id,parameter_name,parameter_value,set_flag) VALUES('%s','%s','%f',1)",para_id,para_name,value);
	sqlite3_exec(db, sql , 0, 0, &zErrMsg);
	return 0;
}

int get_paras_set(char *id)
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


int handle_set_parameters(char *id,int type)
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;
	sqlite3 *db=NULL;
	int result=0;
	int i;

	if(type==0)
		return -1;
	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sprintf(sql, "SELECT * FROM protection_parameters WHERE id='%s'", id);
	if((SQLITE_OK == sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg)) && (nrow > 0))
	{

		if(type>0){
			paras_set.under_voltage_slow = atoi(azResult[ncolumn+4]);
			paras_set.over_voltage_slow = atoi(azResult[ncolumn+5]);
			paras_set.under_frequency_slow = atof(azResult[ncolumn+8]);
			paras_set.over_frequency_slow = atof(azResult[ncolumn+9]);
		}

		if(type>1){
			paras_set.under_voltage_fast = atoi(azResult[ncolumn+2]);
			paras_set.over_voltage_fast = atoi(azResult[ncolumn+3]);
			paras_set.under_frequency_fast = atof(azResult[ncolumn+6]);
			paras_set.over_frequency_fast = atof(azResult[ncolumn+7]);
		}

		if(type>2)
			paras_set.under_voltage_stage_2 = atoi(azResult[ncolumn+15]);
	}
	sqlite3_free_table( azResult );

	memset(sql,'\0',1024);
	sprintf(sql, "SELECT * FROM set_protection_parameters");
	if((SQLITE_OK == sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg)) && (nrow > 0))
	{
		for(i=1;i<nrow+1;i++)
		{
			if(!strcmp(azResult[i*ncolumn],"under_voltage_slow"))
				paras_set.under_voltage_slow = atoi(azResult[i*ncolumn+1]);
			else if(!strcmp(azResult[i*ncolumn],"over_voltage_slow"))
				paras_set.over_voltage_slow = atoi(azResult[i*ncolumn+1]);
			else if(!strcmp(azResult[i*ncolumn],"under_frequency_slow"))
				paras_set.under_frequency_slow = atof(azResult[i*ncolumn+1]);
			else if(!strcmp(azResult[i*ncolumn],"over_frequency_slow"))
				paras_set.over_frequency_slow = atof(azResult[i*ncolumn+1]);
			else if(!strcmp(azResult[i*ncolumn],"under_voltage_fast"))
				paras_set.under_voltage_fast = atoi(azResult[i*ncolumn+1]);
			else if(!strcmp(azResult[i*ncolumn],"over_voltage_fast"))
				paras_set.over_voltage_fast = atoi(azResult[i*ncolumn+1]);
			else if(!strcmp(azResult[i*ncolumn],"under_frequency_fast"))
				paras_set.under_frequency_fast = atof(azResult[i*ncolumn+1]);
			else if(!strcmp(azResult[i*ncolumn],"over_frequency_fast"))
				paras_set.over_frequency_fast = atof(azResult[i*ncolumn+1]);
			else if(!strcmp(azResult[i*ncolumn],"under_voltage_stage_2"))
				paras_set.under_voltage_stage_2 = atoi(azResult[i*ncolumn+1]);
		}
	}
	sqlite3_free_table( azResult );

	if(type>0)			//5项
	{
		saveinttoset_protection_parameters_inverter(db,id,"under_voltage_slow",paras_set.under_voltage_slow);
		saveinttoset_protection_parameters_inverter(db,id,"over_voltage_slow",paras_set.over_voltage_slow);
		savefloattoset_protection_parameters_inverter(db,id,"under_frequency_slow",paras_set.under_frequency_slow);
		savefloattoset_protection_parameters_inverter(db,id,"over_frequency_slow",paras_set.over_frequency_slow);
	}
	if(type>1)			//13项
	{
		saveinttoset_protection_parameters_inverter(db,id,"under_voltage_fast",paras_set.under_voltage_fast);
		saveinttoset_protection_parameters_inverter(db,id,"over_voltage_fast",paras_set.over_voltage_fast);
		savefloattoset_protection_parameters_inverter(db,id,"under_frequency_fast",paras_set.under_frequency_fast);
		savefloattoset_protection_parameters_inverter(db,id,"over_frequency_fast",paras_set.over_frequency_fast);
	}
	if(type>2)			//17项
	{
		saveinttoset_protection_parameters_inverter(db,id,"under_voltage_stage_2",paras_set.under_voltage_stage_2);
	}

	sqlite3_close( db );
	return result;

}

int read_each_inverter_paras(struct inverter_info_t *curinverter)
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;
	sqlite3 *db=NULL;
	int result=0;
	int j,k;

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;
	sprintf(sql,"SELECT * FROM protection_parameters WHERE id='%s'",curinverter->inverterid);
	if((SQLITE_OK == sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg)) && (nrow <= 0))
	{
		for(j=0; j<3; j++){
			result = get_parameters_from_inverter(curinverter);
			if(result > 0){
				if(3 == result) {
					for(k=0; k<3; k++)
						if(!get_parameters_from_inverter_de(curinverter->inverterid))
							break;
				}
				break;
			}
		}

	}
	sqlite3_close(db);
	return 0;
}


int process_paras_changed_set(struct inverter_info_t *firstinverter)
{
	int i, env,type;
	char inverter_result[65535]={'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	printmsg("/*process_paras_changed_set*/");
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
	{
		read_each_inverter_paras(curinverter);
		if((curinverter->curacctime>3600) && (!curinverter->processed_paras_changed_set_flag))
		{
			if((curinverter->op<5)&&(curinverter->opb<5))
			{
				printmsg("HERE");
				printdecmsg(curinverter->inverterid, curinverter->processed_paras_changed_set_flag);
				reset_paras_set();printmsg("HERE");
				type = get_paras_set(curinverter->inverterid);printmsg("HERE");
				handle_set_parameters(curinverter->inverterid,type);
			}
			curinverter->processed_paras_changed_set_flag = 1;
		}
	}
	printmsg("/*process_paras_changed_set*/");

	return 0;
}
