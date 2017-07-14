#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "variation.h"
#include "process_protect_parameters.h"
#include "check_data.h"

extern int caltype;		//计算方式，NA版和非NA版的区别

int resolvedata_yt(char *inverter_data, struct inverter_info_t *inverter)
{
	unsigned char data[20];
	int i, seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*2277/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*92/4096;
	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;

	for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
	}
	seconds = inverter_data[18]*256 + inverter_data[19];
	data[1] &= 0x0f;
	inverter->op = (int)((data[1]*256 + data[2])/8038.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/8038.0/seconds);
	inverter->thistime = time(NULL);
	//inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
	inverter->curgeneration = (float)(((data[1]*256 + data[2])/8038.0/1000.0*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/8038.0/1000.0)/3600.0);
	inverter->lasttime = inverter->thistime;
	//inverter->op = (int)(inverter->dv*inverter->di*1.05/100.0);

	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);

	if(1 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;
	
	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[6]='0';
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]='0';
	inverter->status_web[9]='0';
	inverter->status_web[10]='0';
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]='0';
	inverter->status_web[14]='0';
	inverter->status_web[15]='0';
	inverter->status_web[16]='0';			//Over zero protection(YC500中HV和过0公用标志)
	inverter->status_web[17]='0';		//GFDI-B
	inverter->status_web[18]='0';		//Epprom_done
	
	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[4];
	inverter->status[5]=inverter->status_web[5];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8]=inverter->status_web[12];
	inverter->status[9]='0';
	inverter->status[10]='0';
	
	yc200_status(inverter);

	inverter->lastreporttime[0] = inverter_data[20];
	inverter->lastreporttime[1] = inverter_data[21];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	return 1;
}

int resolvedata_yt_60(char *inverter_data, struct inverter_info_t *inverter)
{
	unsigned char data[20];
	int i, seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*1320/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*172/4096;
	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;

	for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
	}
	seconds = inverter_data[18]*256 + inverter_data[19];
	data[1] &= 0x0f;
	//inverter->op = (int)((data[1]*256 + data[2])/8038.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/8038.0/seconds);
	inverter->op = (int)(inverter->dv*inverter->di*1.05/100.0);
	inverter->thistime = time(NULL);
	inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
	//inverter->curgeneration = (float)(((data[1]*256 + data[2])/8038.0/1000.0*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/8038.0/1000.0)/3600.0);
	inverter->lasttime = inverter->thistime;
	//inverter->op = (int)(inverter->dv*inverter->di*1.05/100.0);
	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);

	if(1 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;
	
	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[6]='0';
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]='0';
	inverter->status_web[9]='0';
	inverter->status_web[10]='0';
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]='0';
	inverter->status_web[14]='0';
	inverter->status_web[15]='0';
	inverter->status_web[16]='0';			//Over zero protection(YC500中HV和过0公用标志)
	inverter->status_web[17]='0';		//GFDI-B
	inverter->status_web[18]='0';		//Epprom_done
	
	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[4];
	inverter->status[5]=inverter->status_web[5];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8]=inverter->status_web[12];
	inverter->status[9]='0';
	inverter->status[10]='0';
	
	yc200_status(inverter);

	inverter->lastreporttime[0] = inverter_data[20];
	inverter->lastreporttime[1] = inverter_data[21];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	return 1;
}

int resolvedata(char *inverter_data, struct inverter_info_t *inverter)
{
	unsigned char data[20];
	int i, seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*825/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*275/4096;
	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;

	if(1 == check_in800(inverter)){					//800个逆变器程序问题，使用以下计算方法
		inverter->op=(int)(inverter->dv*inverter->di*1.05/100.0);
		inverter->thistime = time(NULL);
		inverter->curgeneration = (float)inverter->op * (float)(inverter->thistime - inverter->lasttime)/ 3600.0 /1000.0;
		inverter->lasttime = inverter->thistime;
	}
	else{
		for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
		}
		seconds = inverter_data[18]*256 + inverter_data[19];
		data[1] &= 0x0f;
		inverter->op = (int)((data[1]*256 + data[2])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/seconds);
		inverter->thistime = time(NULL);
		//inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
		inverter->curgeneration = (float)(((data[1]*256 + data[2])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/1000.0)/3600.0);
		inverter->curgeneration = inverter->curgeneration * 0.91;
		inverter->lasttime = inverter->thistime;

		if(inverter->op>260)
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		/*if(1 == caltype)
			inverter->op = (int)(inverter->dv*inverter->di*0.95/100.0);
		else
			inverter->op = (int)(inverter->dv*inverter->di*0.99/100.0);*/
	}
	if(1 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[6]='0';
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]='0';
	inverter->status_web[9]='0';
	inverter->status_web[10]='0';
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]='0';
	inverter->status_web[14]='0';
	inverter->status_web[15]='0';
	inverter->status_web[16]='0';			//Over zero protection(YC500中HV和过0公用标志)
	inverter->status_web[17]='0';		//GFDI-B
	inverter->status_web[18]='0';		//Epprom_done
	
	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[4];
	inverter->status[5]=inverter->status_web[5];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8]=inverter->status_web[12];
	inverter->status[9]='0';
	inverter->status[10]='0';
	
	yc200_status(inverter);

	inverter->lastreporttime[0] = inverter_data[20];
	inverter->lastreporttime[1] = inverter_data[21];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	return 1;
}

int resolvedata_yc250_100(char *inverter_data, struct inverter_info_t *inverter)		//B4命令
{
	unsigned char data[20];
	int i, seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*2337.5/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*97.06/4096;
	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;

	if(1 == check_in800(inverter)){					//800个逆变器程序问题，使用以下计算方法
		inverter->op=(int)(inverter->dv*inverter->di*1.05/100.0);
		inverter->thistime = time(NULL);
		inverter->curgeneration = (float)inverter->op * (float)(inverter->thistime - inverter->lasttime)/ 3600.0 /1000.0;
		inverter->lasttime = inverter->thistime;
	}
	else{
		for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
		}
		/*seconds = inverter_data[18]*256 + inverter_data[19];
		data[1] &= 0x0f;
		inverter->op = (int)((data[1]*256 + data[2])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/seconds);
		inverter->thistime = time(NULL);
		//inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
		inverter->curgeneration = (float)(((data[1]*256 + data[2])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/1000.0)/3600.0);
		inverter->curgeneration = inverter->curgeneration * 0.91;
		inverter->lasttime = inverter->thistime;*/

		inverter->curacctime = inverter_data[18]*256 + inverter_data[19];		//最近两次上报的累计时间的差值，即两次上报的时间间隔
		inverter->curaccgen = (float)(((inverter_data[13]*256 + inverter_data[14])/8681.0/1000.0*256.0*256.0*256.0 + (inverter_data[15]*256.0*256.0 + inverter_data[16]*256.0 + inverter_data[17])/8681.0/1000.0)/3600.0);

		if((inverter->curaccgen > inverter->preaccgen)&&(inverter->curacctime > inverter->preacctime)){
			seconds = inverter->curacctime - inverter->preacctime;
			inverter->duration = seconds;
			inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		}
		else{
			seconds = inverter->curacctime;
			inverter->curgeneration = inverter->curaccgen;
		}

		inverter->preacctime = inverter->curacctime;
		inverter->preaccgen = inverter->curaccgen;

		if(inverter->curacctime > 600){
			inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		}
		else{
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		}


		if(inverter->op>260)
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		/*if(1 == caltype)
			inverter->op = (int)(inverter->dv*inverter->di*0.95/100.0);
		else
			inverter->op = (int)(inverter->dv*inverter->di*0.99/100.0);*/
	}

	if(1 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[6]='0';
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]='0';
	inverter->status_web[9]='0';
	inverter->status_web[10]='0';
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]='0';
	inverter->status_web[14]='0';
	inverter->status_web[15]='0';
	inverter->status_web[16]='0';			//Over zero protection(YC500中HV和过0公用标志)
	inverter->status_web[17]='0';		//GFDI-B
	inverter->status_web[18]='0';		//Epprom_done
	
	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[4];
	inverter->status[5]=inverter->status_web[5];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8]=inverter->status_web[12];
	inverter->status[9]='0';
	inverter->status[10]='0';
	
	yc200_status(inverter);

	inverter->lastreporttime[0] = inverter_data[20];
	inverter->lastreporttime[1] = inverter_data[21];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	check_yc200_yc250(inverter);

	return 1;
}

int resolvedata_b8(char *inverter_data, struct inverter_info_t *inverter)
{
	unsigned char data[20];
	int i, seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*825/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*275/4096;
	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;

	if(1 == check_in800(inverter)){					//800个逆变器程序问题，使用以下计算方法
		inverter->op=(int)(inverter->dv*inverter->di*1.05/100.0);
		inverter->thistime = time(NULL);
		inverter->curgeneration = (float)inverter->op * (float)(inverter->thistime - inverter->lasttime)/ 3600.0 /1000.0;
		inverter->lasttime = inverter->thistime;
	}
	else{
		for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
		}
		/*seconds = inverter_data[18]*256 + inverter_data[19];
		data[1] &= 0x0f;
		inverter->op = (int)((data[1]*256 + data[2])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/seconds);
		inverter->thistime = time(NULL);
		//inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
		inverter->curgeneration = (float)(((data[1]*256 + data[2])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/1000.0)/3600.0);
		inverter->curgeneration = inverter->curgeneration * 0.91;
		inverter->lasttime = inverter->thistime;*/

		inverter->curacctime = inverter_data[18]*256 + inverter_data[19];		//最近两次上报的累计时间的差值，即两次上报的时间间隔
		inverter->curaccgen = (float)(((inverter_data[13]*256 + inverter_data[14])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[15]*256.0*256.0 + inverter_data[16]*256.0 + inverter_data[17])/1661900.0*220.0/1000.0)/3600.0);

		if((inverter->curaccgen > inverter->preaccgen)&&(inverter->curacctime > inverter->preacctime)){
			seconds = inverter->curacctime - inverter->preacctime;
			inverter->duration = seconds;
			inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		}
		else{
			seconds = inverter->curacctime;
			inverter->curgeneration = inverter->curaccgen;
		}

		inverter->preacctime = inverter->curacctime;
		inverter->preaccgen = inverter->curaccgen;

		if(inverter->curacctime > 600){
			inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		}
		else{
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		}


		if(inverter->op>260)
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		/*if(1 == caltype)
			inverter->op = (int)(inverter->dv*inverter->di*0.95/100.0);
		else
			inverter->op = (int)(inverter->dv*inverter->di*0.99/100.0);*/
	}
	if(2 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7]) / 2.93;
	else if(1 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[6]='0';
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]='0';
	inverter->status_web[9]='0';
	inverter->status_web[10]='0';
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]=((inverter_data[10]>>1)&0x01)+0x30;		//Active anti-island protection
	inverter->status_web[14]=((inverter_data[11]>>7)&0x01)+0x30;		//CP protection
	inverter->status_web[15]=((inverter_data[11]>>1)&0x01)+0x30;		//HV protection
	inverter->status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection
	inverter->status_web[17]='0';										//GFDI-B
	inverter->status_web[18]='0';//((inverter_data[11]>>2)&0x01)+0x30;		//Epprom_done

	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[4];
	inverter->status[5]=inverter->status_web[5];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8]=inverter->status_web[12];
	inverter->status[9]='0';
	inverter->status[10]='0';

	yc200_status(inverter);

	inverter->lastreporttime[0] = inverter_data[20];
	inverter->lastreporttime[1] = inverter_data[21];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	check_yc200_yc250(inverter);

	return 1;
}

int resolvedata_yt280_100(char *inverter_data, struct inverter_info_t *inverter)
{
	unsigned char data[20];
	int i, seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*2277/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*92/4096;
	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;

	if(1 == check_in800(inverter)){					//800个逆变器程序问题，使用以下计算方法
		inverter->op=(int)(inverter->dv*inverter->di*1.05/100.0);
		inverter->thistime = time(NULL);
		inverter->curgeneration = (float)inverter->op * (float)(inverter->thistime - inverter->lasttime)/ 3600.0 /1000.0;
		inverter->lasttime = inverter->thistime;
	}
	else{
		for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
		}
		/*seconds = inverter_data[18]*256 + inverter_data[19];
		data[1] &= 0x0f;
		inverter->op = (int)((data[1]*256 + data[2])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/seconds);
		inverter->thistime = time(NULL);
		//inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
		inverter->curgeneration = (float)(((data[1]*256 + data[2])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/1000.0)/3600.0);
		inverter->curgeneration = inverter->curgeneration * 0.91;
		inverter->lasttime = inverter->thistime;*/

		inverter->curacctime = inverter_data[18]*256 + inverter_data[19];		//最近两次上报的累计时间的差值，即两次上报的时间间隔
		inverter->curaccgen = (float)((((inverter_data[13] & 0x0F)*256 + inverter_data[14])/8038.0/1000.0*256.0*256.0*256.0 + (inverter_data[15]*256.0*256.0 + inverter_data[16]*256.0 + inverter_data[17])/8038.0/1000.0)/3600.0);

		if((inverter->curaccgen > inverter->preaccgen)&&(inverter->curacctime > inverter->preacctime)){
			seconds = inverter->curacctime - inverter->preacctime;
			inverter->duration = seconds;
			inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		}
		else{
			seconds = inverter->curacctime;
			inverter->curgeneration = inverter->curaccgen;
		}

		inverter->preacctime = inverter->curacctime;
		inverter->preaccgen = inverter->curaccgen;

		if(inverter->curacctime > 600){
			inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		}
		else{
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		}


		if(inverter->op>260)
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		/*if(1 == caltype)
			inverter->op = (int)(inverter->dv*inverter->di*0.95/100.0);
		else
			inverter->op = (int)(inverter->dv*inverter->di*0.99/100.0);*/
	}
	if(2 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7]) / 2.93;
	else if(1 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[6]='0';
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]='0';
	inverter->status_web[9]='0';
	inverter->status_web[10]='0';
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]=((inverter_data[10]>>1)&0x01)+0x30;		//Active anti-island protection
	inverter->status_web[14]=((inverter_data[11]>>7)&0x01)+0x30;		//CP protection
	inverter->status_web[15]=((inverter_data[11]>>1)&0x01)+0x30;		//HV protection
	inverter->status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection
	inverter->status_web[17]='0';		//GFDI-B
	inverter->status_web[18]='0';		//Epprom_done

	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[4];
	inverter->status[5]=inverter->status_web[5];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8]=inverter->status_web[12];
	inverter->status[9]='0';
	inverter->status[10]='0';

	yc200_status(inverter);

	inverter->lastreporttime[0] = inverter_data[20];
	inverter->lastreporttime[1] = inverter_data[21];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	check_yc200_yc250(inverter);

	return 1;
}

int resolvedata_yt280_60(char *inverter_data, struct inverter_info_t *inverter)
{
	unsigned char data[20];
	int i, seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*1320/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*172/4096;
	inverter->np=(inverter_data[4]*256+inverter_data[5])/10.0;

	if(1 == check_in800(inverter)){					//800个逆变器程序问题，使用以下计算方法
		inverter->op=(int)(inverter->dv*inverter->di*1.05/100.0);
		inverter->thistime = time(NULL);
		inverter->curgeneration = (float)inverter->op * (float)(inverter->thistime - inverter->lasttime)/ 3600.0 /1000.0;
		inverter->lasttime = inverter->thistime;
	}
	else{
		for(i=0; i<6; i++){
			data[i] = inverter_data[12+i];
		}
		/*seconds = inverter_data[18]*256 + inverter_data[19];
		data[1] &= 0x0f;
		inverter->op = (int)((data[1]*256 + data[2])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/seconds);
		inverter->thistime = time(NULL);
		//inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
		inverter->curgeneration = (float)(((data[1]*256 + data[2])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/1000.0)/3600.0);
		inverter->curgeneration = inverter->curgeneration * 0.91;
		inverter->lasttime = inverter->thistime;*/

		inverter->curacctime = inverter_data[18]*256 + inverter_data[19];		//最近两次上报的累计时间的差值，即两次上报的时间间隔
		inverter->curaccgen = (float)((((inverter_data[13] & 0x0F)*256 + inverter_data[14])/8038.0/1000.0*256.0*256.0*256.0 + (inverter_data[15]*256.0*256.0 + inverter_data[16]*256.0 + inverter_data[17])/8038.0/1000.0)/3600.0);

		if((inverter->curaccgen > inverter->preaccgen)&&(inverter->curacctime > inverter->preacctime)){
			seconds = inverter->curacctime - inverter->preacctime;
			inverter->duration = seconds;
			inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		}
		else{
			seconds = inverter->curacctime;
			inverter->curgeneration = inverter->curaccgen;
		}

		inverter->preacctime = inverter->curacctime;
		inverter->preaccgen = inverter->curaccgen;

		if(inverter->curacctime > 600){
			inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		}
		else{
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		}


		if(inverter->op>260)
			inverter->op = (int)(inverter->dv*inverter->di/100.0);
		/*if(1 == caltype)
			inverter->op = (int)(inverter->dv*inverter->di*0.95/100.0);
		else
			inverter->op = (int)(inverter->dv*inverter->di*0.99/100.0);*/
	}
	if(2 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7]) / 2.93;
	else if(1 == caltype)
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nv=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[6]='0';
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]='0';
	inverter->status_web[9]='0';
	inverter->status_web[10]='0';
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]=((inverter_data[10]>>1)&0x01)+0x30;		//Active anti-island protection
	inverter->status_web[14]=((inverter_data[11]>>7)&0x01)+0x30;		//CP protection
	inverter->status_web[15]=((inverter_data[11]>>1)&0x01)+0x30;		//HV protection
	inverter->status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection
	inverter->status_web[17]='0';		//GFDI-B
	inverter->status_web[18]='0';		//Epprom_done

	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[4];
	inverter->status[5]=inverter->status_web[5];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8]=inverter->status_web[12];
	inverter->status[9]='0';
	inverter->status[10]='0';

	yc200_status(inverter);

	inverter->lastreporttime[0] = inverter_data[20];
	inverter->lastreporttime[1] = inverter_data[21];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	check_yc200_yc250(inverter);

	return 1;
}

int resolvedata_500(char *inverter_data, struct inverter_info_t *inverter)
{
	//unsigned char data[20];
	unsigned char temp[2] = {'\0'};
	int i, seconds;

	inverter->dv = (((inverter_data[0] >> 4) & 0x0F) * 256 + ((inverter_data[0] << 4) & 0xF0) | ((inverter_data[1] >> 4) & 0x0F)) * 825 / 4096;
	inverter->di = ((inverter_data[1] & 0x0F) * 256 + inverter_data[2]) * 275 / 4096;
	inverter->dvb = (((inverter_data[3] >> 4) & 0x0F) * 256 + ((inverter_data[3] << 4) & 0xF0) | ((inverter_data[4] >> 4) & 0x0F)) * 825 / 4096;
	inverter->dib = ((inverter_data[4] & 0x0F) * 256 + inverter_data[5]) * 275 / 4096;
	inverter->np = (((inverter_data[6] >> 6) & 0x03) * 256 + (((inverter_data[6] << 2) & 0xFC) | ((inverter_data[7] >> 6) & 0x03))) / 10.0;
	if(1 == caltype)
		inverter->nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 352 / 1024;
	else
		inverter->nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 687 / 1024;
	inverter->it = (((inverter_data[8] & 0x0F) * 256 + inverter_data[9]) * 330 - 245760) / 4096;

	/*inverter->dvb=(inverter_data[0]*256+inverter_data[1])*825/4096;
	inverter->dib=(inverter_data[2]*256+inverter_data[3])*275/4096;
	inverter->npb=(inverter_data[4]*256+inverter_data[5])/10.0;*/

	//for(i=0; i<6; i++)
	//	data[i] = inverter_data[12+i];

	seconds = inverter_data[22]*256 + inverter_data[23];

	inverter->op = (int)(((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/seconds);
	inverter->opb = (int)(((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/seconds);

	/*data[1] &= 0x0f;
	inverter->opb = (int)((data[1]*256 + data[2])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (data[3]*256.0*256.0 + data[4]*256.0 + data[5])/1661900.0*220.0/seconds);*/
	inverter->thistime = time(NULL);
	//inverter->curgeneration = (float)(inverter->op) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
	//inverter->curgenerationb = (float)(inverter->opb) * (float)(inverter->thistime - inverter->lasttime) / 3600.0 /1000.0;
	inverter->lasttime = inverter->thistime;
	inverter->curgeneration = (float)((((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curgenerationb = (float)((((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curgeneration = inverter->curgeneration * 0.91;
	inverter->curgenerationb = inverter->curgenerationb * 0.91;

	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
	if(inverter->opb>260)
		inverter->opb = (int)(inverter->dvb*inverter->dib/100.0);

	/*if(1 == caltype){
		inverter->op = (int)(inverter->dv*inverter->di*0.94/100.0);
		inverter->opb = (int)(inverter->dvb*inverter->dib*0.94/100.0);
	}
	else{
		inverter->op = (int)(inverter->dv*inverter->di*0.97/100.0);
		inverter->opb = (int)(inverter->dvb*inverter->dib*0.97/100.0);
	}*/
	/*if(1 == caltype)
		inverter->nvb=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->nvb=(inverter_data[6]*256+inverter_data[7])*687/1024;
	inverter->itb=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;*/

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]='0';
	inverter->status_web[5]='0';
	inverter->status_web[6]=((inverter_data[10]>>1)&0x01)+0x30;		//DC-A Voltage Too High 1bit
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]=((inverter_data[11]>>7)&0x01)+0x30;		//DC-A Voltage Too Low 1bit
	inverter->status_web[9]=((inverter_data[11]>>6)&0x01)+0x30;		//DC-B Voltage Too High 1bit
	inverter->status_web[10]=((inverter_data[11]>>5)&0x01)+0x30;		//DC-B Voltage Too Low 1bit
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]='0';
	inverter->status_web[14]='0';
	inverter->status_web[15]='0';
	inverter->status_web[16]='0';			//Over zero protection(YC500中HV和过0公用标志)
	inverter->status_web[17]='0';		//GFDI-B
	inverter->status_web[18]='0';		//Epprom_done
	
	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[6];
	inverter->status[5]=inverter->status_web[8];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8] = inverter->status_web[12];
	inverter->status[9] = '0';
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[0];
	inverter->statusb[1]=inverter->status_web[1];
	inverter->statusb[2]=inverter->status_web[2];
	inverter->statusb[3]=inverter->status_web[3];
	inverter->statusb[4]=inverter->status_web[9];
	inverter->statusb[5]=inverter->status_web[10];
	inverter->statusb[6]=inverter->status_web[7];
	inverter->statusb[7]=inverter->status_web[11];
	inverter->statusb[8] = inverter->status_web[12];
	inverter->statusb[9] = '0';
	inverter->statusb[10] = '0';
	
	yc500_status(inverter);

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	inverter->lastreporttime[0] = inverter_data[24];
	inverter->lastreporttime[1] = inverter_data[25];

	return 1;
}

int resolvedata_500_b8(char *inverter_data, struct inverter_info_t *inverter)
{
	unsigned char temp[2] = {'\0'};
	int i, seconds;

	inverter->dv = (((inverter_data[0] >> 4) & 0x0F) * 256 + ((inverter_data[0] << 4) & 0xF0) | ((inverter_data[1] >> 4) & 0x0F)) * 825 / 4096;
	inverter->di = ((inverter_data[1] & 0x0F) * 256 + inverter_data[2]) * 275 / 4096;
	inverter->dvb = (((inverter_data[3] >> 4) & 0x0F) * 256 + ((inverter_data[3] << 4) & 0xF0) | ((inverter_data[4] >> 4) & 0x0F)) * 825 / 4096;
	inverter->dib = ((inverter_data[4] & 0x0F) * 256 + inverter_data[5]) * 275 / 4096;
	inverter->np = (((inverter_data[6] >> 6) & 0x03) * 256 + (((inverter_data[6] << 2) & 0xFC) | ((inverter_data[7] >> 6) & 0x03))) / 10.0;
	if(2 == caltype)
		inverter->nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) / 2.93;
	else if(1 == caltype)
		inverter->nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 352 / 1024;
	else
		inverter->nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 687 / 1024;
	inverter->it = (((inverter_data[8] & 0x0F) * 256 + inverter_data[9]) * 330 - 245760) / 4096;

	inverter->curacctime = inverter_data[22]*256 + inverter_data[23];		//最近两次上报的累计时间的差值，即两次上报的时间间隔
	inverter->curaccgen = (float)(((inverter_data[12]*256 + inverter_data[13])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curaccgenb = (float)(((inverter_data[17]*256 + inverter_data[18])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/1000.0)/3600.0);

	if(inverter->curacctime == inverter->preacctime)
	{
		inverter->curflag = '0';
		return 0;
	}

	if((inverter->curaccgen >= inverter->preaccgen)&&(inverter->curacctime > inverter->preacctime)&&(inverter->curaccgenb >= inverter->preaccgenb)){
		seconds = inverter->curacctime - inverter->preacctime;
		inverter->duration = seconds;
		inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		inverter->curgenerationb = inverter->curaccgenb - inverter->preaccgenb;
	}
	else{
		seconds = inverter->curacctime;
		inverter->duration = seconds;
		inverter->curgeneration = inverter->curaccgen;
		inverter->curgenerationb = inverter->curaccgenb;
	}

	inverter->preacctime = inverter->curacctime;
	inverter->preaccgen = inverter->curaccgen;
	inverter->preaccgenb = inverter->curaccgenb;

	if(inverter->curacctime > 600){
		inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		inverter->opb = inverter->curgenerationb * 1000.0 * 3600.0 / seconds;
	}
	else{
		inverter->op = (int)(inverter->dv * inverter->di / 100.0);
		inverter->opb = (int)(inverter->dvb * inverter->dib / 100.0);
	}

	/*inverter->op = (int)(((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/seconds);
	inverter->opb = (int)(((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/seconds);

	
	inverter->thistime = time(NULL);
	
	inverter->lasttime = inverter->thistime;
	inverter->curgeneration = (float)((((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curgenerationb = (float)((((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curgeneration = inverter->curgeneration * 0.91;
	inverter->curgenerationb = inverter->curgenerationb * 0.91;*/

	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
	if(inverter->opb>260)
		inverter->opb = (int)(inverter->dvb*inverter->dib/100.0);

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]='0';
	inverter->status_web[5]='0';
	inverter->status_web[6]=((inverter_data[10]>>1)&0x01)+0x30;		//DC-A Voltage Too High 1bit
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]=((inverter_data[11]>>7)&0x01)+0x30;		//DC-A Voltage Too Low 1bit
	inverter->status_web[9]=((inverter_data[11]>>6)&0x01)+0x30;		//DC-B Voltage Too High 1bit
	inverter->status_web[10]=((inverter_data[11]>>5)&0x01)+0x30;		//DC-B Voltage Too Low 1bit
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI-A
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]=((inverter_data[10]>>3)&0x01)+0x30;		//Active anti-island protection
	inverter->status_web[14]=((inverter_data[10]>>2)&0x01)+0x30;		//CP protection
	inverter->status_web[15]=(inverter_data[11]&0x01)+0x30;			//HV protection(YC500中HV和过0公用标志)
	inverter->status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection(YC500中HV和过0公用标志)
	inverter->status_web[17]=((inverter_data[11]>>1)&0x01)+0x30;		//GFDI-B
	inverter->status_web[18]='0';//((inverter_data[11]>>2)&0x01)+0x30;		//Epprom_done
	
	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[6];
	inverter->status[5]=inverter->status_web[8];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8] = inverter->status_web[12];
	inverter->status[9] = '0';
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[0];
	inverter->statusb[1]=inverter->status_web[1];
	inverter->statusb[2]=inverter->status_web[2];
	inverter->statusb[3]=inverter->status_web[3];
	inverter->statusb[4]=inverter->status_web[9];
	inverter->statusb[5]=inverter->status_web[10];
	inverter->statusb[6]=inverter->status_web[7];
	inverter->statusb[7]=inverter->status_web[11];
	inverter->statusb[8] = inverter->status_web[12];
	inverter->statusb[9] = '0';
	inverter->statusb[10] = '0';
	
	yc500_status(inverter);

	inverter->lastreporttime[0] = inverter_data[24];
	inverter->lastreporttime[1] = inverter_data[25];
	
	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	check_yc500(inverter);

	return 1;
}

int resolvedata_500_b7(char *inverter_data, struct inverter_info_t *inverter)
{
	unsigned char temp[2] = {'\0'};
	int i, seconds;

	inverter->dv = (((inverter_data[0] >> 4) & 0x0F) * 256 + ((inverter_data[0] << 4) & 0xF0) | ((inverter_data[1] >> 4) & 0x0F)) * 1320 / 4096;
	inverter->di = ((inverter_data[1] & 0x0F) * 256 + inverter_data[2]) * 172 / 4096;
	inverter->dvb = (((inverter_data[3] >> 4) & 0x0F) * 256 + ((inverter_data[3] << 4) & 0xF0) | ((inverter_data[4] >> 4) & 0x0F)) * 1320 / 4096;
	inverter->dib = ((inverter_data[4] & 0x0F) * 256 + inverter_data[5]) * 172 / 4096;
	inverter->np = (((inverter_data[6] >> 6) & 0x03) * 256 + (((inverter_data[6] << 2) & 0xFC) | ((inverter_data[7] >> 6) & 0x03))) / 10.0;
	if(2 == caltype)
		inverter->nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) / 2.93;
	else if(1 == caltype)
		inverter->nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 352 / 1024;
	else
		inverter->nv = (((inverter_data[7] >> 4) & 0x03) *256 + ((inverter_data[7] << 4 & 0xF0) | ((inverter_data[8] >> 4) & 0x0F))) * 687 / 1024;
	inverter->it = (((inverter_data[8] & 0x0F) * 256 + inverter_data[9]) * 330 - 245760) / 4096;

	inverter->curacctime = inverter_data[22]*256 + inverter_data[23];		//最近两次上报的累计时间的差值，即两次上报的时间间隔
	inverter->curaccgen = (float)(((inverter_data[12]*256 + inverter_data[13])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curaccgenb = (float)(((inverter_data[17]*256 + inverter_data[18])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/1000.0)/3600.0);

	if((inverter->curaccgen >= inverter->preaccgen)&&(inverter->curacctime > inverter->preacctime)&&(inverter->curaccgenb >= inverter->preaccgenb)){
		seconds = inverter->curacctime - inverter->preacctime;
		inverter->duration = seconds;
		inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		inverter->curgenerationb = inverter->curaccgenb - inverter->preaccgenb;
	}
	else{
		seconds = inverter->curacctime;
		inverter->duration = seconds;
		inverter->curgeneration = inverter->curaccgen;
		inverter->curgenerationb = inverter->curaccgenb;
	}

	inverter->preacctime = inverter->curacctime;
	inverter->preaccgen = inverter->curaccgen;
	inverter->preaccgenb = inverter->curaccgenb;

	if(inverter->curacctime > 600){
		inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		inverter->opb = inverter->curgenerationb * 1000.0 * 3600.0 / seconds;
	}
	else{
		inverter->op = (int)(inverter->dv * inverter->di / 100.0);
		inverter->opb = (int)(inverter->dvb * inverter->dib / 100.0);
	}

	/*inverter->op = (int)(((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/seconds);
	inverter->opb = (int)(((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/seconds*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/seconds);


	inverter->thistime = time(NULL);

	inverter->lasttime = inverter->thistime;
	inverter->curgeneration = (float)((((inverter_data[12] & 0x0f)*256 + inverter_data[13])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[14]*256.0*256.0 + inverter_data[15]*256.0 + inverter_data[16])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curgenerationb = (float)((((inverter_data[17] & 0x0f)*256 + inverter_data[18])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[19]*256.0*256.0 + inverter_data[20]*256.0 + inverter_data[21])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curgeneration = inverter->curgeneration * 0.91;
	inverter->curgenerationb = inverter->curgenerationb * 0.91;*/

	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
	if(inverter->opb>260)
		inverter->opb = (int)(inverter->dvb*inverter->dib/100.0);

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]='0';
	inverter->status_web[5]='0';
	inverter->status_web[6]=((inverter_data[10]>>1)&0x01)+0x30;		//DC-A Voltage Too High 1bit
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]=((inverter_data[11]>>7)&0x01)+0x30;		//DC-A Voltage Too Low 1bit
	inverter->status_web[9]=((inverter_data[11]>>6)&0x01)+0x30;		//DC-B Voltage Too High 1bit
	inverter->status_web[10]=((inverter_data[11]>>5)&0x01)+0x30;		//DC-B Voltage Too Low 1bit
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI-A
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]=((inverter_data[10]>>3)&0x01)+0x30;		//Active anti-island protection
	inverter->status_web[14]=((inverter_data[10]>>2)&0x01)+0x30;		//CP protection
	inverter->status_web[15]=(inverter_data[11]&0x01)+0x30;			//HV protection(YC500中HV和过0公用标志)
	inverter->status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection(YC500中HV和过0公用标志)
	inverter->status_web[17]=((inverter_data[11]>>1)&0x01)+0x30;		//GFDI-B
	inverter->status_web[18]='0';//((inverter_data[11]>>2)&0x01)+0x30;		//Epprom_done

	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[6];
	inverter->status[5]=inverter->status_web[8];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8] = inverter->status_web[12];
	inverter->status[9] = '0';
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[0];
	inverter->statusb[1]=inverter->status_web[1];
	inverter->statusb[2]=inverter->status_web[2];
	inverter->statusb[3]=inverter->status_web[3];
	inverter->statusb[4]=inverter->status_web[9];
	inverter->statusb[5]=inverter->status_web[10];
	inverter->statusb[6]=inverter->status_web[7];
	inverter->statusb[7]=inverter->status_web[11];
	inverter->statusb[8] = inverter->status_web[12];
	inverter->statusb[9] = '0';
	inverter->statusb[10] = '0';

	yc500_status(inverter);

	inverter->lastreporttime[0] = inverter_data[24];
	inverter->lastreporttime[1] = inverter_data[25];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	check_yc500(inverter);

	return 1;
}

int cal_avg_power(struct inverter_info_t *firstinverter)
{
	int avgpower=0, syspower=0;		//平均功率，和总功率
	int i, count=0;	//count:系统逆变器个数
	struct inverter_info_t *inverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){		//计算有数据的面板的平均功率
		if(1 == inverter->flagyc500){		//YC500加2，根据面板算
			if('1' == inverter->flag){		//有数据才统计进平均功率
				count = count + 2;
				syspower = syspower + inverter->op + inverter->opb;
			}
		}
		else{
			if('1' == inverter->flag){		//有数据才统计进平均功率
				count++;
				syspower = syspower + inverter->op;
			}
		}
		inverter++;
	}

	avgpower = syspower / count;

	return avgpower;
}

/*int lenddata(struct inverter_info_t *firstinverter)		//当本轮平均功率大于10W时，如果当前一轮有些逆变器没有数据，需要填充上次的数据，最多填充3次
{
	struct inverter_info_t *inverter = firstinverter;
	int avgpower=0, i;

	avgpower = cal_avg_power(firstinverter);
	if(avgpower > 5){
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){		//借用上回的数据
			if((inverter->lendtimes<3) && ('1' != inverter->curflag)){		//只有借用次数小于3次时才可以借用
				if(1 == inverter->flagyc500){				//YC500
					inverter->dv = inverter->predv;
					inverter->dvb = inverter->predvb;
					inverter->di = inverter->predi;
					inverter->dib = inverter->predib;
					inverter->np = inverter->prenp;
					inverter->nv = inverter->prenv;
					inverter->it = inverter->preit;
					inverter->op = inverter->preop;
					inverter->opb = inverter->preopb;
					inverter->curgeneration = inverter->precurgeneration;
					inverter->curgenerationb = inverter->precurgenerationb;
					inverter->status[0] = inverter->prestatus[0];
					inverter->status[1] = inverter->prestatus[1];
					inverter->status[2] = inverter->prestatus[2];
					inverter->status[3] = inverter->prestatus[3];
					inverter->status[4] = inverter->prestatus[4];
					inverter->status[5] = inverter->prestatus[5];
					inverter->status[6] = inverter->prestatus[6];
					inverter->status[7] = inverter->prestatus[7];
					inverter->status[8] = inverter->prestatus[8];
					inverter->status[9] = inverter->prestatus[9];
					inverter->status[10] = inverter->prestatus[10];

					inverter->statusb[0] = inverter->prestatusb[0];
					inverter->statusb[1] = inverter->prestatusb[1];
					inverter->statusb[2] = inverter->prestatusb[2];
					inverter->statusb[3] = inverter->prestatusb[3];
					inverter->statusb[4] = inverter->prestatusb[4];
					inverter->statusb[5] = inverter->prestatusb[5];
					inverter->statusb[6] = inverter->prestatusb[6];
					inverter->statusb[7] = inverter->prestatusb[7];
					inverter->statusb[8] = inverter->prestatusb[8];
					inverter->statusb[9] = inverter->prestatusb[9];
					inverter->statusb[10] = inverter->prestatusb[10];
				}
				else{				//YC200
					inverter->dv = inverter->predv;
					inverter->di = inverter->predi;
					inverter->np = inverter->prenp;
					inverter->nv = inverter->prenv;
					inverter->it = inverter->preit;
					inverter->op = inverter->preop;
					inverter->curgeneration = inverter->precurgeneration;
					inverter->status[0] = inverter->prestatus[0];
					inverter->status[1] = inverter->prestatus[1];
					inverter->status[2] = inverter->prestatus[2];
					inverter->status[3] = inverter->prestatus[3];
					inverter->status[4] = inverter->prestatus[4];
					inverter->status[5] = inverter->prestatus[5];
					inverter->status[6] = inverter->prestatus[6];
					inverter->status[7] = inverter->prestatus[7];
					inverter->status[8] = inverter->prestatus[8];
					inverter->status[9] = inverter->prestatus[9];
					inverter->status[10] = inverter->prestatus[10];
				}
				inverter->lendtimes++;
				inverter->curflag = '1';
				inverter->flag = '1';
			}
			inverter++;
		}
	}
}*/

int backup_data(struct inverter_info_t *inverter)		//放入备用数据
{
	inverter->lendtimes = 0;

	if(1 == inverter->flagyc500){
		inverter->predv = inverter->dv;
		inverter->predvb = inverter->dvb;
		inverter->predi = inverter->di;
		inverter->predib = inverter->dib;
		inverter->prenp = inverter->np;
		inverter->prenv = inverter->nv;
		inverter->preit = inverter->it;
		inverter->preop = inverter->op;
		inverter->preopb = inverter->opb;
		inverter->precurgeneration = inverter->curgeneration;
		inverter->precurgenerationb = inverter->curgenerationb;
		inverter->prestatus[0] = inverter->status[0];
		inverter->prestatus[1] = inverter->status[1];
		inverter->prestatus[2] = inverter->status[2];
		inverter->prestatus[3] = inverter->status[3];
		inverter->prestatus[4] = inverter->status[4];
		inverter->prestatus[5] = inverter->status[5];
		inverter->prestatus[6] = inverter->status[6];
		inverter->prestatus[7] = inverter->status[7];
		inverter->prestatus[8] = inverter->status[8];
		inverter->prestatus[9] = inverter->status[9];
		inverter->prestatus[10] = inverter->status[10];

		inverter->prestatusb[0] = inverter->statusb[0];
		inverter->prestatusb[1] = inverter->statusb[1];
		inverter->prestatusb[2] = inverter->statusb[2];
		inverter->prestatusb[3] = inverter->statusb[3];
		inverter->prestatusb[4] = inverter->statusb[4];
		inverter->prestatusb[5] = inverter->statusb[5];
		inverter->prestatusb[6] = inverter->statusb[6];
		inverter->prestatusb[7] = inverter->statusb[7];
		inverter->prestatusb[8] = inverter->statusb[8];
		inverter->prestatusb[9] = inverter->statusb[9];
		inverter->prestatusb[10] = inverter->statusb[10];
	}
	else{
		inverter->predv = inverter->dv;
		inverter->predi = inverter->di;
		inverter->prenp = inverter->np;
		inverter->prenv = inverter->nv;
		inverter->preit = inverter->it;
		inverter->preop = inverter->op;
		inverter->precurgeneration = inverter->curgeneration;
		inverter->prestatus[0] = inverter->status[0];
		inverter->prestatus[1] = inverter->status[1];
		inverter->prestatus[2] = inverter->status[2];
		inverter->prestatus[3] = inverter->status[3];
		inverter->prestatus[4] = inverter->status[4];
		inverter->prestatus[5] = inverter->status[5];
		inverter->prestatus[6] = inverter->status[6];
		inverter->prestatus[7] = inverter->status[7];
		inverter->prestatus[8] = inverter->status[8];
		inverter->prestatus[9] = inverter->status[9];
		inverter->prestatus[10] = inverter->status[10];
	}
}

int yc200_status(struct inverter_info_t *inverter)
{
	memset(inverter->status_ema, '\0', sizeof(inverter->status_ema));
	inverter->status_send_flag=0;

	if(('1' == inverter->status_web[1]) || ('1' == inverter->status_web[0]) || ('1' == inverter->status_web[2]) ||
			('1' == inverter->status_web[3]) || ('1' == inverter->status_web[15]) || ('1' == inverter->status_web[14]) ||
			('1' == inverter->status_web[13]) || ('1' == inverter->status_web[16]) || ('1' == inverter->status_web[7]) ||
			('1' == inverter->status_web[11]) || ('1' == inverter->status_web[12])){
		strcat(inverter->status_ema, inverter->inverterid);
		strcat(inverter->status_ema, "01");
		if(('1' == inverter->status_web[1]) || ('1' == inverter->status_web[0]) || ('1' == inverter->status_web[2]) ||
				('1' == inverter->status_web[3]) || ('1' == inverter->status_web[15]) || ('1' == inverter->status_web[14]) ||
				('1' == inverter->status_web[13]) || ('1' == inverter->status_web[16]))
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[7])
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[11])
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[12])
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");

		inverter->status_ema[18] = inverter->status_web[1];
		inverter->status_ema[19] = inverter->status_web[0];
		inverter->status_ema[20] = inverter->status_web[2];
		inverter->status_ema[21] = inverter->status_web[3];
		inverter->status_ema[22] = inverter->status_web[16];
		inverter->status_ema[23] = inverter->status_web[15];
		inverter->status_ema[24] = inverter->status_web[14];
		inverter->status_ema[25] = inverter->status_web[13];
		inverter->status_ema[26] = inverter->status_web[7];
		inverter->status_ema[27] = inverter->status_web[11];
		inverter->status_ema[28] = inverter->status_web[12];
		inverter->status_ema[29] = '0';//inverter->status_web[18];
		strcat(inverter->status_ema, "END");

		inverter->status_send_flag=1;
	}
}

int yc500_status(struct inverter_info_t *inverter)
{
	memset(inverter->status_ema, '\0', sizeof(inverter->status_ema));
	inverter->status_send_flag=0;

	if(('1' == inverter->status_web[1]) || ('1' == inverter->status_web[0]) || ('1' == inverter->status_web[2]) ||
			('1' == inverter->status_web[3]) || ('1' == inverter->status_web[15]) || ('1' == inverter->status_web[14]) ||
			('1' == inverter->status_web[13]) || ('1' == inverter->status_web[7]) || ('1' == inverter->status_web[12]) ||
			('1' == inverter->status_web[11]) || ('1' == inverter->status_web[17])){
		strcat(inverter->status_ema, inverter->inverterid);
		strcat(inverter->status_ema, "02");
		if(('1' == inverter->status_web[1]) || ('1' == inverter->status_web[0]) || ('1' == inverter->status_web[2]) ||
				('1' == inverter->status_web[3]) || ('1' == inverter->status_web[15]) || ('1' == inverter->status_web[14]) ||
				('1' == inverter->status_web[13]))
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[7])
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if(('1' == inverter->status_web[11]) || ('1' == inverter->status_web[17]))
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[12])
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");

		inverter->status_ema[18] = inverter->status_web[1];
		inverter->status_ema[19] = inverter->status_web[0];
		inverter->status_ema[20] = inverter->status_web[2];
		inverter->status_ema[21] = inverter->status_web[3];
		inverter->status_ema[22] = inverter->status_web[16];
		inverter->status_ema[23] = inverter->status_web[14];
		inverter->status_ema[24] = inverter->status_web[13];
		inverter->status_ema[25] = inverter->status_web[7];
		inverter->status_ema[26] = inverter->status_web[12];
		inverter->status_ema[27] = '0';//;inverter->status_web[18];
		inverter->status_ema[28] = 'A';
		inverter->status_ema[29] = inverter->status_web[11];
		inverter->status_ema[30] = 'B';
		inverter->status_ema[31] = inverter->status_web[17];
		strcat(inverter->status_ema, "END");

		inverter->status_send_flag=1;
	}
}
