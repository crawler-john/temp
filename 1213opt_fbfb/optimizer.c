/**
 * 用于优化器DD，CC等
 */

#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "variation.h"
#include "zigbee.h"
#include <string.h>

extern ecu_info ecu;

init_optimizer()
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow, ncolumn;
	int i;
	sqlite3 *db;
	sqlite3_open("/home/database.db", &db);	//create a database

	//建立id表：用于存储逆变器的ID号，短地址，型号，版本号，绑定标志位，ZigBee版本号，Flag标志位
	strcpy(sql,"CREATE TABLE IF NOT EXISTS optimizerconf (item INTEGER,id VARCHAR(256),onoff VARCHAR(256),duty VARCHAR(256),dutyflag INTEGER");
	sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	memset(sql,'\0',1024);
	sprintf(sql,"DELETE FROM optimizerconf");
	sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	memset(sql,'\0',1024);
	sprintf(sql,"SELECT id FROM optimizerconf");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	for(i=0;i<nrow;i++)
	{
		memset(sql,'\0',1024);
		sprintf(sql,"INSERT INTO optimizerconf (item,id) VALUES (%d,'%s')",i,azResult[i+1]);
		sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	}
	sqlite3_close(db);

}

int updateoptonoff(inverter_info *inverter, int onoffresult)//更新优化器开关状态结果
{
	char sql[100] = { '\0' };
	char *zErrMsg = 0;
	int i;
	sqlite3 *db;
	sqlite3_open("/home/database.db", &db);


	sprintf(sql,"UPDATE optimizerconf SET onoff=%d WHERE id=%s ", onoffresult, inverter->inverterid);
	for(i=0; i<3; i++)
	{
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
		{
			print2msg(inverter->inverterid, "Update onoff optimizer successfully");
			break;
		}
		else
			print2msg(inverter->inverterid, "Failed to update onoff optimizer");
	}
	sqlite3_close(db);
	return 0;
}

int updateoptduty(inverter_info *inverter, int dutyresult)//更新逆变器的最大功率结果
{
	char sql[100] = { '\0' };
	char *zErrMsg = 0;
	int i;
	sqlite3 *db;
	sqlite3_open("/home/database.db", &db);

	sprintf(sql,"UPDATE optimizerconf SET duty=%d WHERE id=%s ", dutyresult, inverter->inverterid);
	for(i=0; i<3; i++)
	{
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
		{
			print2msg(inverter->inverterid, "Update duty optimizer successfully");
			break;
		}
		else
			print2msg(inverter->inverterid, "Failed to update duty optimizer");
	}
	sqlite3_close(db);
	return 0;
}

int set_nosetted(inverter_info *firstinverter,int x)
{
	int i,j,l,cout;
	inverter_info *curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++)
	{
		curinverter->onoff=0;
		curinverter++;
	}
	for(l=0;l<10;l++)
	{
		curinverter = firstinverter;
		cout=0;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++)
		{
			if(0==curinverter->onoff)
			{
				printmsg("go off");
				cout=1;printdecmsg(" cout",cout);
				for(j=0;j<3;j++)
				{
					//res=turn_off_optimizer(curinverter);printf("**res=%d\n",res);if(res==1)
					if(1==opt_onoff_single(curinverter,x))
					{
						curinverter->onoff=1;
						break;
					}
				}
			}
			curinverter++;
		}printdecmsg("cout",cout);
		if(cout==0)
			break;
	}
	return 0;
}

int zb_CC_optimizer(inverter_info *inverter, char *optimizer_DD_reply,int x)
{
	int i=0, ret;
	unsigned char sendbuff[512]={'\0'};
	clear_zbmodem();
	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x01;
	sendbuff[4] = 0x06;
	sendbuff[5] = 0XCC;
	sendbuff[6] = 0x01;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	if(x==0){
		sendbuff[10]= 0x00;
		sendbuff[11]= 0x3F;
		sendbuff[12]= 0x13;
	}
	if(x==1){
		sendbuff[10]= 0x01;
		sendbuff[11]= 0x2F;
		sendbuff[12]= 0x32;
	}
	sendbuff[13]= 0xFE;
	sendbuff[14]= 0xFE;
	for(i=0;i<3;i++)
	{
		zb_broadcast_cmd(sendbuff, 15);
		sleep(1);
	}
	printhexmsg("Turn_onoff_optimizer",sendbuff,15);
	if(x==0)
		printmsg("Open optimizer ALL");
	if(x==1)
		printmsg("Close optimizer ALL");
	return 1;
}


int zb_DD_optimizer(inverter_info *inverter, char *optimizer_DD_reply)
{
	int i=0, ret;
	unsigned char sendbuff[512]={'\0'};
	clear_zbmodem();
	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x01;
	sendbuff[4] = 0x06;
	sendbuff[5] = 0XDD;
	sendbuff[6] = 0x01;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	sendbuff[10]= 0x00;
	sendbuff[11]= 0x60;
	sendbuff[12]= 0x37;
	sendbuff[13]= 0xFE;
	sendbuff[14]= 0xFE;
	zb_send_cmd(inverter, sendbuff, 15);
	print2msg(inverter->inverterid, "Query optimizer configuration");

	ret = zb_get_reply(optimizer_DD_reply,inverter);
	if((31 == ret) && (0xDA == optimizer_DD_reply[5]) && (0xFB == optimizer_DD_reply[0]) && (0xFB ==optimizer_DD_reply[1]) && (0xFE == optimizer_DD_reply[29]) && (0xFE == optimizer_DD_reply[30]))
		return 1;
	else
		return -1;


}

int opt_onoff_single(inverter_info *inverter,int x)		//开关机指令单播,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	unsigned short check=0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x01;
	sendbuff[4] = 0x06;
	sendbuff[5] = 0xCD;
	sendbuff[6] = 0x01;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	if(x==0){
		sendbuff[10] = 0x00;
		sendbuff[11] = 0x7A;		//CHK
		sendbuff[12] = 0xB3;		//CHK
	}
	if(x==1){
		sendbuff[10] = 0x01;
		sendbuff[11] = 0x6A;		//CHK
		sendbuff[12] = 0x92;		//CHK
	}
	sendbuff[13] = 0xFE;
	sendbuff[14] = 0xFE;


	zb_send_cmd(inverter, sendbuff, 15);
	print2msg("Turn onoff single",inverter->inverterid);
	ret = zb_get_reply(data,inverter);

	if((15 == ret) && (0xDE == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[13]) && (0xFE == data[14]))
		return 1;
	else
		return -1;
}


int set_opt_conf(inverter_info *firstinverter)
{
	FILE *fp;
	int i, j, res,flag;
	char id[13];
	int limitedresult1,limitedresult2;
	char readpresetdata[20] = {'\0'};
	inverter_info *curinverter = firstinverter;

	memset(id, '\0', 13);
	fp = fopen("/tmp/connect.conf", "r");
	if(fp)
	{
		fgets(id, 50, fp);
		fclose(fp);
		if((!strcmp(id, "connect all"))&&(!ecu.onoff))
		{
			res = zb_CC_optimizer(curinverter, readpresetdata,0);
			set_nosetted(curinverter,0);			//广播之后的单点，确保开关机成功
		}
		if((!strcmp(id, "disconnect all"))&&(!ecu.onoff))
		{
			res = zb_CC_optimizer(curinverter, readpresetdata,1);
			set_nosetted(curinverter,1);
		}
		fp = fopen("/tmp/connect.conf", "w");
		fclose(fp);
	}
	while(1){
		curinverter = firstinverter;
		memset(id, '\0', 13);
		if(!get_turn_on_off_flag(id, &flag))
			break;
		clear_turn_on_off_flag(id);
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){
			if(!strncmp(id, curinverter->inverterid, 12)){
				j = 0;
				if(1 == flag){
					while(j<3){
						if(1 == opt_onoff_single(curinverter,0))
							break;
						j++;
					}
				}
				if(2 == flag){
					while(j<3){
						if(1 == opt_onoff_single(curinverter,1))
							break;
						j++;
					}
				}


				break;
			}
			curinverter++;
		}
	}
}

int read_opt_conf(inverter_info *firstinverter)
{

	FILE *fp;
	int i, j, res;
	char id[13];
	int limitedresult1,limitedresult2;
	char readpresetdata[20] = {'\0'};
	inverter_info *curinverter = firstinverter;

	memset(id, '\0', 13);
	fp = fopen("/tmp/getoptconf.conf", "r");
	if(fp)
	{
		fgets(id, 50, fp);
		fclose(fp);
	}
	if(!strcmp(id, "ALL"))
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
		{
			for(j=0; j<3; j++)
			{
				memset(readpresetdata, '\0', 20);
				res = zb_DD_optimizer(curinverter, readpresetdata);
				if(1 == res)
				{
					limitedresult1 = (readpresetdata[10]);					//读取成功
					limitedresult2 = (readpresetdata[12]*256+readpresetdata[13]);
					updateoptonoff(curinverter, limitedresult1);
					updateoptduty(curinverter, limitedresult2);
					//updatemaxflag(curinverter);
					//save_max_power_result_one(curinverter, limitedresult);
					break;
				}
			}
		}

		fp = fopen("/tmp/getoptconf.conf", "w");
		fclose(fp);
	//	save_max_power_result_all();
	}

	return 0;

}

int process_optimizer(inverter_info *firstinverter)
{
	set_opt_conf(firstinverter);
//	read_opt_conf(firstinverter);			//DD帧读取所有参数
}
