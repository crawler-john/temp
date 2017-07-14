/*
 * setpower.c
 *	Version: 1.1
 *  Modified on: 2013-3-14
 *  	修改内容：设置失败后，最多再设置3此
 *	Author: aps
 *      使用方法：在plc.c中的process_all()中添加processpower()，在plc.c开头添加processpower()的原型声明
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"

#define DEBUGINFO 0

extern int plcmodem;
extern unsigned char ccuid[7];
extern sqlite3 *db;			//数据库

int getpower(struct inverter_info_t *inverter, int *limitedpower, int *stationarypower)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow, ncolumn;

	sprintf(sql,"SELECT limitedpower,stationarypower FROM power WHERE id=%s;", inverter->inverterid);
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	*limitedpower = atoi(azResult[2]);
	*stationarypower = atoi(azResult[3]);
	sqlite3_free_table( azResult );

	printdecmsg("Maximun power", *limitedpower);
	printdecmsg("Fixed power", *stationarypower);

	return 0;
}

int get_maximum_power(struct inverter_info_t *inverter)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;
	int power;

	sprintf(sql,"SELECT limitedpower FROM power WHERE id=%s AND limitedpower IS NOT NULL;", inverter->inverterid);
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		power = atoi(azResult[1]);
		sqlite3_free_table( azResult );
		return power;
	}
	else
		return -1;
}

int get_fixed_power(struct inverter_info_t *inverter)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow, ncolumn;
	int power;

	sprintf(sql,"SELECT stationarypower FROM power WHERE id=%s;", inverter->inverterid);
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	power = atoi(azResult[1]);
	sqlite3_free_table( azResult );

	printdecmsg("Fixed power", power);

	return power;
}

int getsyspower()		//读取系统最大总功率
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow, ncolumn, syspower;

	sprintf(sql,"SELECT sysmaxpower FROM powerall WHERE item=0;");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	syspower = atoi(azResult[1]);
	sqlite3_free_table( azResult );

	printdecmsg("Maximun system power", syspower);

	return syspower;
}

int updatemaxpower(struct inverter_info_t *inverter, int limitedresult)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	int i;

	sprintf(sql,"UPDATE power SET limitedresult=%d,flag=0 WHERE id=%s ", limitedresult, inverter->inverterid);

	for(i=0; i<3; i++){
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg )){
			print2msg(inverter->inverterid, "Update maximun power successfully");
			break;
		}
		else
			print2msg(inverter->inverterid, "Failed to update maximun power");
	}

	return 0;
}

int updatemaxflag(struct inverter_info_t *inverter)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	int i;

	sprintf(sql,"UPDATE power SET flag=0 WHERE id=%s ", inverter->inverterid);

	for(i=0; i<3; i++){
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg )){
			print2msg(inverter->inverterid, "Update maximun power flag successfully");
				break;
		}
		else
			print2msg(inverter->inverterid, "Failed to update maximun flag power");
	}


	print2msg(inverter->inverterid, "has been changed to Maximun power Mode");
}

int updatefixedpower(struct inverter_info_t *inverter, int stationaryresult)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE power SET stationaryresult=%d WHERE id=%s ", stationaryresult, inverter->inverterid);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	printdecmsg("Fixed power from inverter", stationaryresult);
}

int updatefixedflag(struct inverter_info_t *inverter)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE power SET flag=1 WHERE id=%s ", inverter->inverterid);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	print2msg(inverter->inverterid, "has been changed to fixed power Mode!\n");
}

int calcount(struct inverter_info_t *firstinverter)
{
	int i, panelcount=0;
	struct inverter_info_t *curinverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
		if(1 == curinverter->flagyc500)
			panelcount += 2;
		else
			panelcount++;
	}

	printdecmsg("Panel count", panelcount);

	return panelcount;
}

int setlimitedpowerone(struct inverter_info_t *inverter, char power)		//发送命令，设置单个逆变器限定功率
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int i, j, res=0;

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
	sendbuff[11] = inverter->tnuid[0];		//TNID
	sendbuff[12] = inverter->tnuid[1];		//TNID
	sendbuff[13] = inverter->tnuid[2];		//TNID
	sendbuff[14] = inverter->tnuid[3];		//TNID
	sendbuff[15] = inverter->tnuid[4];		//TNID
	sendbuff[16] = inverter->tnuid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xC4;			//CMD
	sendbuff[21] = power;

	for(i=2; i<22; i++)
		check = check + sendbuff[i];

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	write(plcmodem, sendbuff, 26);
	sleep(1);

	printhexmsg("Set maximun power", sendbuff, 26);

	return 0;
}

int setlimitedpowerall(char power)	//发送广播命令，设置所有逆变器限定功率
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;

	sendbuff[0] = 0xfb;			//数据包头
	sendbuff[1] = 0xfb;			//数据包头
	sendbuff[2] = 0x03;			//命令字
	sendbuff[3] = 0x00;			//数据长度
	sendbuff[4] = 0x12;			//数据长度
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = 0x00;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0x00;
	sendbuff[14] = 0x00;
	sendbuff[15] = 0x00;
	sendbuff[16] = 0x00;
	sendbuff[17] = 0x4f;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xC3;
	sendbuff[21] = power;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check>>8;
	sendbuff[24] = check;
	sendbuff[25] = 0xfe;
	sendbuff[26] = 0xfe;

	res = write(plcmodem, sendbuff, 27);
	sleep(1);

	printhexmsg("Set maximun power", sendbuff, 27);

	return 0;
}

int setstationarypowerone(struct inverter_info_t *inverter, char power)		//发送命令，设置单个逆变器固定功率
{
	unsigned char sendbuff[512] = {'\0'};
	unsigned short check = 0x00;
	int i, j, res=0;

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
	sendbuff[11] = inverter->tnuid[0];		//TNID
	sendbuff[12] = inverter->tnuid[1];		//TNID
	sendbuff[13] = inverter->tnuid[2];		//TNID
	sendbuff[14] = inverter->tnuid[3];		//TNID
	sendbuff[15] = inverter->tnuid[4];		//TNID
	sendbuff[16] = inverter->tnuid[5];		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xC8;			//CMD
	sendbuff[21] = power;

	for(i=2; i<22; i++)
		check = check + sendbuff[i];

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;		//TAIL
	sendbuff[25] = 0xFE;		//TAIL

	write(plcmodem, sendbuff, 26);
	sleep(1);

	printhexmsg("Set fixed power", sendbuff, 26);

	return 0;
}

int setstationarypowerall(char power)	//发送广播命令，设置所有逆变器固定功率
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i, res;

	sendbuff[0] = 0xfb;			//数据包头
	sendbuff[1] = 0xfb;			//数据包头
	sendbuff[2] = 0x03;			//命令字
	sendbuff[3] = 0x00;			//数据长度
	sendbuff[4] = 0x12;			//数据长度
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = 0x00;
	sendbuff[12] = 0x00;
	sendbuff[13] = 0x00;
	sendbuff[14] = 0x00;
	sendbuff[15] = 0x00;
	sendbuff[16] = 0x00;
	sendbuff[17] = 0x4f;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xC7;
	sendbuff[21] = power;
	sendbuff[22] = 0x00;

	for(i=2; i<23; i++){
		check = check + sendbuff[i];
	}

	sendbuff[23] = check>>8;
	sendbuff[24] = check;
	sendbuff[25] = 0xfe;
	sendbuff[26] = 0xfe;

	res = write(plcmodem, sendbuff, 27);
	sleep(1);

	printhexmsg("Set fixed power", sendbuff, 27);

	return 0;
}

int save_max_power_result_all(void)			//设置所有逆变器功率的结果
{
	char ecu_id[16];
	FILE *fp;
	sqlite3 *db;
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	int i;
	char sql[1024];
	char set_max_power_result[65535] = {'\0'};
	char inverter_result[64];

	strcpy(set_max_power_result, "APS13AAAAAA117AAA1");
	fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	strcat(set_max_power_result, ecu_id);					//ECU的ID
	strcat(set_max_power_result, "0000");					//逆变器个数
	strcat(set_max_power_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(set_max_power_result, "END");					//固定格式


	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "SELECT id,limitedresult FROM power");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
		{
			for(i=1; i<=nrow; i++)
			{
				memset(inverter_result, '\0', sizeof(inverter_result));
				sprintf(inverter_result, "%s%03d020300END", azResult[i*ncolumn], atoi(azResult[i*ncolumn+1]));
				strcat(set_max_power_result, inverter_result);
			}
			strcat(set_max_power_result, "\n");
			sqlite3_free_table(azResult);
			break;
		}
		else
			print2msg("SELECT FROM power", zErrMsg);
		sqlite3_free_table(azResult);
		sleep(1);
	}
	sqlite3_close(db);
	set_max_power_result[30] = nrow/1000 + 0x30;
	set_max_power_result[31] = (nrow/100)%10 + 0x30;
	set_max_power_result[32] = (nrow/10)%10 + 0x30;
	set_max_power_result[33] = nrow%10 + 0x30;

	if(strlen(set_max_power_result) > 10000)
		set_max_power_result[5] = (strlen(set_max_power_result)-1)/10000 + 0x30;
	if(strlen(set_max_power_result) > 1000)
		set_max_power_result[6] = ((strlen(set_max_power_result)-1)/1000)%10 + 0x30;
	if(strlen(set_max_power_result) > 100)
		set_max_power_result[7] = ((strlen(set_max_power_result)-1)/100)%10 + 0x30;
	if(strlen(set_max_power_result) > 10)
		set_max_power_result[8] = ((strlen(set_max_power_result)-1)/10)%10 + 0x30;
	if(strlen(set_max_power_result) > 0)
		set_max_power_result[9] = (strlen(set_max_power_result)-1)%10 + 0x30;

	save_process_result(117, set_max_power_result);

	return 0;
}

int save_max_power_result_one(struct inverter_info_t *inverter, int power)		//设置一个逆变器最大功率的结果
{
	char inverter_result[65535] = {'\0'};

	memset(inverter_result, '\0', sizeof(inverter_result));
	sprintf(inverter_result, "%s%03d020300END", inverter->inverterid, power);
	save_inverter_parameters_result(inverter, 117, inverter_result);

	return 0;
}

int process_max_power_all(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	char id[13]={'\0'};
	char limitedvalue;
	char temp[256]={'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	fp = fopen("/tmp/setmaxpower.conf", "r");
	if(fp){
		fgets(id, 50, fp);
		fclose(fp);
	}

	if(!strncmp(id, "ALL", 3)){
		printmsg("Set all inverter's maximum power");

		fp = fopen("/etc/yuneng/max_power_all.conf", "r");
		if(fp){
			fgets(temp, sizeof(temp), fp);
			fclose(fp);
		}

		limitedvalue = (atoi(temp) * 7395) >> 14;
		setlimitedpowerall(limitedvalue);
		fp = fopen("/tmp/getmaxpower.conf", "w");
		if(fp)
		{
			fputs("ALL", fp);
			fclose(fp);
		}

		fp = fopen("/tmp/setmaxpower.conf", "w");
		fclose(fp);
	}

	return 0;
}

int process_max_power(struct inverter_info_t *firstinverter)
{
//	FILE *fp;
	int limitedpower, stationarypower, i, j, k, m, res;
//	char id[13];
	char limitedvalue;
	int limitedresult;
//	int syspower;
//	int panelcount=0;
//	int yc200=0, yc500=0;
	char readpresetdata[20] = {'\0'};
	struct inverter_info_t *curinverter = firstinverter;
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;

//	fp = fopen("/tmp/setmaxpower.conf", "r");
//	memset(id, '\0', 13);
//	curinverter = firstinverter;
//	fgets(id, 50, fp);
//	fclose(fp);
//	if(!strlen(id))
//		return 0;
//	else if(!strncmp(id, "MAX", 3)){		//设置系统总功率
//		syspower = getsyspower();
//		panelcount = calcount(firstinverter);
//		if(syspower/panelcount > 250)
//			limitedvalue = (250*7395)>>14;
//		else
//			limitedvalue = (syspower/panelcount*7395)>>14;
//
//		printmsg("Set system maximum power");
//
//		setlimitedpowerall(limitedvalue);
//
//		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
//			memset(readpresetdata, '\0', 20);
//			//if(1 == curinverter->flagyc500){
//			res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
//			if((1 == res) && (limitedvalue == readpresetdata[1]))
//			{
//				limitedresult = (readpresetdata[1] << 14) / 7395;					//读取成功
//				updatemaxpower(curinverter, limitedresult);
//				updatemaxflag(curinverter);
//			}
//			else{		//如果第一次设置失败，再最多设置三次
//				for(j=0; j<3; j++){
//					setlimitedpowerone(curinverter, limitedvalue);
//					memset(readpresetdata, '\0', 20);
//					res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
//					if((1 == res) && (limitedvalue == readpresetdata[1])){
//						limitedresult = (readpresetdata[1] << 14) / 7395;
//						updatemaxpower(curinverter, limitedresult);
//						updatemaxflag(curinverter);
//						break;
//					}
//				}
//			}
//			/*if(1 == res){
//				limitedresult = (readpresetdata[1] << 14) / 7395;					//读取成功
//				updatemaxpower(curinverter, limitedresult);
//				if(limitedvalue == readpresetdata[1])								//标志设置为限定功率模式
//					updatemaxflag(curinverter);
//			}
//			else{//没有返回结果，继续单个设置
//				setlimitedpowerone(curinverter, limitedvalue);//
//				res == askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
//				if(res)
//
//			}*/
//			//}
//		}
//
//		fp = fopen("/tmp/setmaxpower.conf", "w");		//清空
//		fclose(fp);
//	}
//	else if(!strncmp(id, "ALL", 3)){
//		printmsg("Set all inverter's maximum power");
//
//		//getpower(firstinverter, &limitedpower, &stationarypower);
//		limitedpower = get_maximum_power(firstinverter);
//		if(-1 == limitedpower){
//			fp = fopen("/tmp/setmaxpower.conf", "w");
//			fclose(fp);
//			return 0;
//		}
//		limitedvalue = (limitedpower * 7395) >> 14;
//
//		setlimitedpowerall(limitedvalue);
//
//		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
//			memset(readpresetdata, '\0', 20);
//			//if(1 == curinverter->flagyc500){
//			res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
//			if((1 == res) && (limitedvalue == readpresetdata[1])){
//				limitedresult = (readpresetdata[1] << 14) / 7395;					//读取成功
//				updatemaxpower(curinverter, limitedresult);
//				updatemaxflag(curinverter);
//			}
//			else{
//				for(j=0; j<3; j++){
//					setlimitedpowerone(curinverter, limitedvalue);
//					memset(readpresetdata, '\0', 20);
//					res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
//					if((1 == res) && (limitedvalue == readpresetdata[1])){
//						limitedresult = (readpresetdata[1] << 14) / 7395;
//						updatemaxpower(curinverter, limitedresult);
//						updatemaxflag(curinverter);
//						break;
//					}
//				}
//			}
//		}
//
//		fp = fopen("/tmp/setmaxpower.conf", "w");
//		fclose(fp);
//		save_max_power_result_all();
//	}
//	else{
		//printmsg("Set one inverter's maximum power");

		sprintf(sql,"SELECT id,limitedpower FROM power WHERE flag=1 AND limitedpower IS NOT NULL;");
		for(i=0; i<3; i++){
			res = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
			if(SQLITE_OK == res){
				for(j=1; j<=nrow; j++){
					curinverter = firstinverter;
					for(k=0; (k<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); k++, curinverter++){
						if(!strncmp(azResult[j*ncolumn], curinverter->inverterid, 12)){
							updatemaxflag(curinverter);
							limitedpower = atoi(azResult[j*ncolumn+1]);

							if(0 == curinverter->inverter_with_13_parameters){
								for(m=0; m<3; m++){
									if(-1 != askpresetdatacmd(curinverter, readpresetdata, 0x11))
										break;
								}
								if(0 == curinverter->inverter_with_13_parameters)
									break;
							}
							if(1 == curinverter->inverter_with_13_parameters)
								limitedvalue = (limitedpower * 7395) >> 14;
							if(2 == curinverter->inverter_with_13_parameters)
								limitedvalue = (limitedpower * 7395) >> 16;
							for(m=0; m<3; m++){
								setlimitedpowerone(curinverter, limitedvalue);
								memset(readpresetdata, '\0', 20);
								res = askpresetdatacmd(curinverter, readpresetdata, 0x11);
								if((-1 != res) && (limitedvalue == readpresetdata[1])){
									if(1 == curinverter->inverter_with_13_parameters)
										limitedresult = (readpresetdata[1] << 14) / 7395;
									if(2 == curinverter->inverter_with_13_parameters)
										limitedresult = (readpresetdata[1] << 16) / 7395;
									updatemaxpower(curinverter, limitedresult);
									save_max_power_result_one(curinverter, limitedresult);
									break;
								}
							}
							break;
						}
					}
				}
				sqlite3_free_table( azResult );
				break;
			}
			else
				sqlite3_free_table( azResult );
		}


//	}
//	else{
//		printmsg("Set one inverter's maximum power");
//
//		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
//			if(!strncmp(id, curinverter->inverterid, 12)){
//				printmsg("point1\n");
//				//getpower(curinverter, &limitedpower, &stationarypower);
//				printmsg("point2\n");
//				limitedpower = get_maximum_power(curinverter);
//				if(-1 == limitedpower)
//					break;
//
//				if(0 == curinverter->inverter_with_13_parameters){
//					for(j=0; j<3; j++){
//						if(1 == askpresetdatacmd(curinverter, reference_protect_parameters, 0x11))
//							break;
//					}
//					if(0 == curinverter->inverter_with_13_parameters)
//						break;
//				}
//				if(1 == curinverter->inverter_with_13_parameters)
//					limitedvalue = (limitedpower * 7395) >> 14;
//				if(2 == curinverter->inverter_with_13_parameters)
//					limitedvalue = (limitedpower * 7395) >> 16;
//
////				limitedvalue = (limitedpower * 7395) >> 14;
//				for(j=0; j<4; j++){
//					setlimitedpowerone(curinverter, limitedvalue);
//					memset(readpresetdata, '\0', 20);
//					res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
//					if((1 == res) && (limitedvalue == readpresetdata[1])){
//						if(1 == curinverter->inverter_with_13_parameters)
//							limitedresult = (readpresetdata[1] << 14) / 7395;
//						if(2 == curinverter->inverter_with_13_parameters)
//							limitedresult = (readpresetdata[1] << 16) / 7395;
//						updatemaxpower(curinverter, limitedresult);
//						update_max_flag(curinverter);
//						save_max_power_result_one(curinverter->inverterid, limitedresult);
//						break;
//					}
//				}
//				break;
//			}
//		}
//
//		fp = fopen("/tmp/setmaxpower.conf", "w");
//		fclose(fp);
//	}
}

int process_fixed_power(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	int limitedpower, stationarypower, i, j, res;
	char id[13];
	char stationaryvalue;
	int stationaryresult;
	char readpresetdata[20] = {'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	fp = fopen("/tmp/setfixpower.conf", "r");
	memset(id, '\0', 13);
	curinverter = firstinverter;
	fgets(id, 50, fp);
	fclose(fp);
	if(!strlen(id))
		return 0;
	else if(!strncmp(id, "ALL", 3)){
		printmsg("Set all inverter's fixed power");

		//getpower(firstinverter, &limitedpower, &stationarypower);
		stationarypower = get_fixed_power(firstinverter);
		stationaryvalue = (stationarypower * 7395) >> 14;

		setstationarypowerall(stationaryvalue);

		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
			memset(readpresetdata, '\0', 20);
			res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
			if((1 == res) && (stationaryvalue == readpresetdata[3])){
				stationaryresult = (readpresetdata[3] << 14) / 7395;
				updatefixedpower(curinverter, stationaryresult);
				updatefixedflag(curinverter);
			}
			else{
				for(j=0; j<3; j++){
					setstationarypowerone(curinverter, stationaryvalue);
					memset(readpresetdata, '\0', 20);
					res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
					if((1 == res) && (stationaryvalue == readpresetdata[3])){
						stationaryresult = (readpresetdata[3] << 14) / 7395;
						updatefixedpower(curinverter, stationaryresult);
						updatefixedflag(curinverter);
						break;
					}
				}
			}
		}
		fp = fopen("/tmp/setfixpower.conf", "w");
		fclose(fp);
	}
	else{
		printmsg("Set one inverter's fixed power");

		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++){
			if(!strncmp(id, curinverter->inverterid, 12)){
				//getpower(curinverter, &limitedpower, &stationarypower);
				stationarypower = get_fixed_power(curinverter);
				stationaryvalue = (stationarypower * 7395) >> 14;

				for(j=0; j<4; j++){
					setstationarypowerone(curinverter, stationaryvalue);
					memset(readpresetdata, '\0', 20);
					res = askpresetdatacmd(plcmodem, ccuid, curinverter->tnuid, readpresetdata, 0x11);
					if((1 == res) && (stationaryvalue == readpresetdata[3])){
						stationaryresult = (readpresetdata[3] << 14) / 7395;
						updatefixedpower(curinverter, stationaryresult);
						updatefixedflag(curinverter);
						break;
					}
				}
				break;
			}
		}

		fp = fopen("/tmp/setfixpower.conf", "w");
		fclose(fp);
	}
}

int read_max_power(struct inverter_info_t *firstinverter)
{
	FILE *fp;
	int i, j, res;
	char id[13];
	int limitedresult;
	char readpresetdata[20] = {'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	memset(id, '\0', 13);
	fp = fopen("/tmp/getmaxpower.conf", "r");
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
				res = askpresetdatacmd(curinverter, readpresetdata, 0x11);
				if(-1 != res)
				{
					if(1 == curinverter->inverter_with_13_parameters)
						limitedresult = (readpresetdata[1] << 14) / 7395;
					if(2 == curinverter->inverter_with_13_parameters)
						limitedresult = (readpresetdata[1] << 16) / 7395;					//读取成功
					updatemaxpower(curinverter, limitedresult);
					//updatemaxflag(curinverter);
					save_max_power_result_one(curinverter, limitedresult);
					break;
				}
			}
		}

		fp = fopen("/tmp/getmaxpower.conf", "w");
		fclose(fp);
	}

	return 0;
}

int processpower(struct inverter_info_t *firstinverter)
{
	process_max_power(firstinverter);
	//process_fixed_power(firstinverter);
	//read_max_power(firstinverter);
}
