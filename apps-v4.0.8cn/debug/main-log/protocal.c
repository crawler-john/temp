#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"
#include "debug.h"


/*int transsyscurgen(char *buff, float curgen)		//增加系统当前一轮发电量
{
	int i, temp=0;
	char curgentemp[10]={'\0'};

	temp = curgen*1000000.0;
	sprintf(curgentemp, "%d", temp);
	for(i=0; i<CURSYSGENLEN-strlen(curgentemp); i++)
		strcat(buff, "A");
	strcat(buff, curgentemp);

	return 0;
}

float calsystemgeneration(struct inverter_info_t *inverter)		//计算当前一轮的系统发电量
{
	int i;
	float temp=0;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&('1'==inverter->flag)&&(12==strlen(inverter->inverterid)); i++){
		temp = temp + inverter->curgeneration;
		inverter++;
	}
	
	return temp;
}

int transltgen(char *buff, int ltgen)		//增加历史发电量
{
	int i;
	char ltgentemp[10]={'\0'};

	sprintf(ltgentemp, "%d", ltgen);
	for(i=0; i<LIFETIMEGENLEN-strlen(ltgentemp); i++)
		strcat(buff, "A");
	strcat(buff, ltgentemp);

	return 0;
}

int transdv(char *buff, int dv)				//增加逆变器的直流电压
{
	int i;
	char dvtemp[10]={'\0'};
	
	sprintf(dvtemp, "%d", dv);
	for(i=0; i<DVLENGTH-strlen(dvtemp); i++)		//直流电压占5个字节，与EMA协议规定
		strcat(buff, "A");
	strcat(buff, dvtemp);
	
	return 0;
}

int transdi(char *buff, int di)				//增加逆变器的直流电流
{
	int i;
	char ditemp[10]={'\0'};

	sprintf(ditemp, "%d", di);
	for(i=0; i<DILENGTH-strlen(ditemp); i++)
		strcat(buff, "A");
	strcat(buff, ditemp);
		
	return 0;
}

int transpower(char *buff, int power)			//增加逆变器的功率
{
	int i;
	char powertemp[10]={'\0'};

	sprintf(powertemp, "%d", power*100);
	for(i=0; i<POWERLENGTH-strlen(powertemp); i++)
		strcat(buff, "A");
	strcat(buff, powertemp);
		
	return 0;
}

int transfrequency(char *buff, int frequency)			//增加逆变器的电网频率
{
	int i;
	char fretemp[10]={'\0'};

	sprintf(fretemp, "%d", frequency);
	for(i=0; i<FREQUENCYLENGTH-strlen(fretemp); i++)
		strcat(buff, "A");
	strcat(buff, fretemp);
		
	return 0;
}

int transtemperature(char *buff, int temperature)			//增加逆变器的温度
{
	int i;
	char tempertemp[10]={'\0'};

	if(temperature<0)
		sprintf(tempertemp, "%d", temperature*-1);
	else
		sprintf(tempertemp, "%d", temperature);

	if(temperature<=-10)
		strcat(buff, "B");
	else if(temperature<0)
		strcat(buff, "AB");
	else if(temperature<10)
		strcat(buff, "AA");
	else if(temperature<100)
		strcat(buff, "A");
	else
		;

	strcat(buff, tempertemp);
		
	return 0;
}

int transgridvolt(char *buff, int voltage)			//增加逆变器的电网电压
{
	int i;
	char gvtemp[10]={'\0'};

	sprintf(gvtemp, "%d", voltage);
	for(i=0; i<GRIDVOLTLENGTH-strlen(gvtemp); i++)
		strcat(buff, "A");
	strcat(buff, gvtemp);

	return 0;
}

int transstatus(char *buff, char status[STATUSLENGTH])		//增加逆变器的状态
{
	unsigned char cmp = 0x10;
	int i;
	
	for(i=0; i<8; i++){
		if(1==(status[0]&cmp))
			strcat(buff, "1");
		else
			strcat(buff, "0");
		cmp = cmp>>1;				//右移一位
	}
	
	cmp = 0x10;
	for(i=0; i<3; i++){
		if(1==(status[1]&cmp))
			strcat(buff, "1");
		else
			strcat(buff, "0");
		cmp = cmp>>1;				//右移一位
	}
	
	return 0;
}

int transcurgen(char *buff, int gen)		//增加逆变器的当前一轮发电量
{
	int i;
	char gentemp[10]={'\0'};

	sprintf(gentemp, "%d", gen);
	for(i=0; i<CURGENLENGTH-strlen(gentemp); i++)
		strcat(buff, "A");
	strcat(buff, gentemp);

	return 0;
}

int transsyspower(char *buff, int syspower)			//增加系统功率
{
	int i;
	char syspowertemp[10]={'\0'};

	sprintf(syspowertemp, "%d", syspower*100);
	for(i=0; i<SYSTEMPOWERLEN-strlen(syspowertemp); i++)
		strcat(buff, "A");
	strcat(buff, syspowertemp);

	return 0;
}

int protocal(struct inverter_info_t *firstinverter, char *id, char *buff, char *sendcommanddatetime, int maxcount, int syspower, float curgeneration, float ltgeneration)
{
	int i;
	//char datetime[20]={'\0'};
	struct inverter_info_t *inverter = firstinverter;
	
	//get_time(datetime);
	for(i=0; (i<MAXINVERTERCOUNT)&&('1'==inverter->flag)&&(strlen(inverter->inverterid)==12); i++){
		//if(() && ){
			strcat(buff, inverter->inverterid);
			transdv(buff, inverter->dv);
			transdi(buff, inverter->di);
			transpower(buff, inverter->op);
			transfrequency(buff, inverter->np*10);
			transtemperature(buff, inverter->it);
			transgridvolt(buff, inverter->nv);
			transstatus(buff, inverter->status);
			transcurgen(buff, inverter->curgeneration*1000000);
			strcat(buff, "END");
		//}
		inverter++;
	}
	
	strcat(buff, id);
	transsyspower(buff, syspower);
	transsyscurgen(buff, curgeneration);
	transltgen(buff, ltgeneration*10.0);
	strcat(buff, sendcommanddatetime);
	strcat(buff, "\n");
#ifdef DEBUGINFO
	printf("%s", buff);
#endif
	
	return 0;
}*/

/*int transsyscurgen(char *buff, float curgen)		//增加系统当前一轮发电量
{
	int i, temp=0;
	char curgentemp[10]={'\0'};

	temp = (int)(curgen*1000000.0);
	sprintf(curgentemp, "%d", temp);
	for(i=0; i<CURSYSGENLEN-strlen(curgentemp); i++)
		strcat(buff, "A");
	strcat(buff, curgentemp);

	return 0;
}

float calsystemgeneration(struct inverter_info_t *inverter)		//计算当前一轮的系统发电量
{
	int i;
	float temp=0.0;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){
		if('1'==inverter->flag)
			temp = temp + inverter->curgeneration + inverter->curgenerationb;
		inverter++;
	}

	printfloatmsg("temp", temp);

	return temp;
}

int transltgen(char *buff, float ltgen)		//增加历史发电量
{
	int i;
	char ltgentemp[10]={'\0'};
	int temp = (int)ltgen;

	sprintf(ltgentemp, "%d", temp);
	for(i=0; i<LIFETIMEGENLEN-strlen(ltgentemp); i++)
		strcat(buff, "A");
	strcat(buff, ltgentemp);

	return 0;
}

int transdv(char *buff, float dv)				//增加逆变器的直流电压
{
	int i;
	char dvtemp[10]={'\0'};
	int temp = (int)dv;

	sprintf(dvtemp, "%d", temp);
	for(i=0; i<DVLENGTH-strlen(dvtemp); i++)		//直流电压占5个字节，与EMA协议规定
		strcat(buff, "A");
	strcat(buff, dvtemp);

	return 0;
}

int transdi(char *buff, float di)				//增加逆变器的直流电流
{
	int i;
	char ditemp[10]={'\0'};
	int temp = (int)di;

	sprintf(ditemp, "%d", temp);
	for(i=0; i<DILENGTH-strlen(ditemp); i++)
		strcat(buff, "A");
	strcat(buff, ditemp);

	return 0;
}

int transpower(char *buff, int power)			//增加逆变器的功率
{
	int i;
	char powertemp[10]={'\0'};

	sprintf(powertemp, "%d", power*100);
	for(i=0; i<POWERLENGTH-strlen(powertemp); i++)
		strcat(buff, "A");
	strcat(buff, powertemp);

	return 0;
}

int transfrequency(char *buff, float frequency)			//增加逆变器的电网频率
{
	int i;
	char fretemp[10]={'\0'};
	int temp = (int)frequency;

	sprintf(fretemp, "%d", temp);
	for(i=0; i<FREQUENCYLENGTH-strlen(fretemp); i++)
		strcat(buff, "A");
	strcat(buff, fretemp);

	return 0;
}

int transtemperature(char *buff, int temperature)			//增加逆变器的温度
{
	int i;
	char tempertemp[10]={'\0'};

	if(temperature<0)
		sprintf(tempertemp, "%d", temperature*-1);
	else
		sprintf(tempertemp, "%d", temperature);

	if(temperature<=-10)
		strcat(buff, "B");
	else if(temperature<0)
		strcat(buff, "AB");
	else if(temperature<10)
		strcat(buff, "AA");
	else if(temperature<100)
		strcat(buff, "A");
	else
		;

	strcat(buff, tempertemp);

	return 0;
}

int transgridvolt(char *buff, int voltage)			//增加逆变器的电网电压
{
	int i;
	char gvtemp[10]={'\0'};

	sprintf(gvtemp, "%d", voltage);
	for(i=0; i<GRIDVOLTLENGTH-strlen(gvtemp); i++)
		strcat(buff, "A");
	strcat(buff, gvtemp);

	return 0;
}

int transstatus(char *buff, char *status)		//增加逆变器的状态
{
	unsigned char cmp = 0x10;
	int i;

	for(i=0; i<11; i++){
		if('1' == status[i])
			strcat(buff, "1");
		else
			strcat(buff, "0");
		//cmp = cmp>>1;				//右移一位
	}

	/*cmp = 0x10;
	for(i=0; i<3; i++){
		if(1==(status[1]&cmp))
			strcat(buff, "1");
		else
			strcat(buff, "0");
		cmp = cmp>>1;				//右移一位
	}*/

	/*return 0;
}

int transcurgen(char *buff, float gen)		//增加逆变器的当前一轮发电量
{
	int i;
	char gentemp[10]={'\0'};
	int temp = (int)gen;

	sprintf(gentemp, "%d", temp);
	for(i=0; i<CURGENLENGTH-strlen(gentemp); i++)
		strcat(buff, "A");
	strcat(buff, gentemp);

	return 0;
}

int transsyspower(char *buff, int syspower)			//增加系统功率
{
	int i;
	char syspowertemp[10]={'\0'};

	sprintf(syspowertemp, "%d", syspower*100);
	for(i=0; i<SYSTEMPOWERLEN-strlen(syspowertemp); i++)
		strcat(buff, "A");
	strcat(buff, syspowertemp);

	return 0;
}*/

//把所有逆变器的数据按照ECU和EMA的通信协议转换，见协议
/*int protocal(struct inverter_info_t *firstinverter, char *id, char *buff, char *sendcommanddatetime, int syspower, float curgeneration, float ltgeneration)
{
	int i;
	char temp[50] = {'\0'};
	struct inverter_info_t *inverter = firstinverter;//printf("protocal1\n");
	char inverter_data[256];

	sprintf(buff, "APS1500000AAA1AAA1%s%010d%010d%010d%s%03d", id, syspower, curgeneration*1000000, ltgeneration*10, sendcommanddatetime, displaycount);

//	strcat(buff, "APS1500000AAA1AAA1");
//	strcat(buff, id);//printf("protocal3 ");
//	transsyspower(buff, syspower);//printf("protocal4 ");
//	transsyscurgen(buff, curgeneration);//printf("protocal5 ");
//	transltgen(buff, ltgeneration*10.0);//printf("protocal6 ");
//	strcat(buff, sendcommanddatetime);//printf("protocal7\n");

//	sprintf(temp, "%d", displaycount);
//	for(i=0; i<(3-strlen(temp)); i++)
//		strcat(buff, "A");
//	strcat(buff, temp);

	if(!zoneflag)
		strcat(buff, "0");
	else
		strcat(buff, "1");
	strcat(buff, "00000END");

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){
		if('1' == inverter->flag){
			memset(inverter_data, '\0', sizeof(inverter_data));
			if(1 == inverter->flagyc500){
				sprintf(inverter_data, "%sA%05d%03d%05d%05d%03d%03d%11s%010dEND", inverter->inverterid, (int)(inverter->dv), (int)(inverter->di), inverter->op, (int)(inverter->np*10), inverter->it+100, inverter->nv, inverter->status, (int)(inverter->curgeneration*1000000));
				strcat(buff, inverter_data);
				sprintf(inverter_data, "%sB%05d%03d%05d%05d%03d%03d%11s%010dEND", inverter->inverterid, (int)(inverter->dvb), (int)(inverter->dib), inverter->opb, (int)(inverter->np*10), inverter->it+100, inverter->nv, inverter->statusb, (int)(inverter->curgenerationb*1000000));
				strcat(buff, inverter_data);
//				strcat(buff, inverter->inverterid);//printf("protocal11");
//				strcat(buff, "A");
//				transdv(buff, inverter->dv);//printf("protocal12");
//				transdi(buff, inverter->di);//printf("protocal13");
//				transpower(buff, inverter->op);//printf("protocal14");
//				transfrequency(buff, inverter->np*10.0);//printf("protocal15");
//				transtemperature(buff, inverter->it);//printf("protocal16");
//				transgridvolt(buff, inverter->nv);//printf("protocal17");
//				transstatus(buff, inverter->status);//printf("protocal18");
//				transcurgen(buff, inverter->curgeneration*1000000.0);//printf("protocal19\n");
//				strcat(buff, "END");

//				strcat(buff, inverter->inverterid);//printf("protocal11");
//				strcat(buff, "B");
//				transdv(buff, inverter->dvb);//printf("protocal12");
//				transdi(buff, inverter->dib);//printf("protocal13");
//				transpower(buff, inverter->opb);//printf("protocal14");
//				transfrequency(buff, inverter->np*10.0);//printf("protocal15");
//				transtemperature(buff, inverter->it);//printf("protocal16");
//				transgridvolt(buff, inverter->nv);//printf("protocal17");
//				transstatus(buff, inverter->statusb);//printf("protocal18");
//				transcurgen(buff, inverter->curgenerationb*1000000.0);//printf("protocal19\n");
//				strcat(buff, "END");
			}
			else{
				sprintf(inverter_data, "%s0%05d%03d%05d%05d%03d%03d%11s%010dEND", inverter->inverterid, (int)(inverter->dv), (int)(inverter->di), inverter->op, (int)(inverter->np*10), inverter->it+100, inverter->nv, inverter->status, (int)(inverter->curgeneration*1000000));
				strcat(buff, inverter_data);
//				strcat(buff, inverter->inverterid);//printf("protocal11");
//				strcat(buff, "0");
//				transdv(buff, inverter->dv);//printf("protocal12");
//				transdi(buff, inverter->di);//printf("protocal13");
//				transpower(buff, inverter->op);//printf("protocal14");
//				transfrequency(buff, inverter->np*10.0);//printf("protocal15");
//				transtemperature(buff, inverter->it);//printf("protocal16");
//				transgridvolt(buff, inverter->nv);//printf("protocal17");
//				transstatus(buff, inverter->status);//printf("protocal18");
//				transcurgen(buff, inverter->curgeneration*1000000.0);//printf("protocal19\n");
//				strcat(buff, "END");
			}
		}
		inverter++;
	}//printf("protocal2 ");
	memset(temp, '\0', 50);
	sprintf(temp, "%d", strlen(buff));
	for(i=0; i<(5-strlen(temp)); i++)
		buff[5+i] =  'A';
	for(i=0; i<strlen(temp); i++)
		buff[5+5-strlen(temp)+i] = temp[i];

	strcat(buff, "\n");

	printmsg(buff);

	return 0;
}*/
