#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"

extern ecu_info ecu;

int transsyscurgen(char *buff, float curgen)		//增加系统当前一轮发电量
{
	int i, k, temp=0;
	char curgentemp[10]={'\0'};

	temp = (int)(curgen*1000000.0);
	sprintf(curgentemp, "%d", temp);

	k=CURSYSGENLEN-strlen(curgentemp);

	if(k>=0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, curgentemp);
	}
	else
	{
		strncat(buff, curgentemp,CURSYSGENLEN);
	}
	return 0;
}

float calsystemgeneration(struct inverter_info_t *inverter)		//计算当前一轮的系统发电量
{
	int i;
	float temp=0.0;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++){
		if('1'==inverter->dataflag)
			temp = temp + inverter->curgeneration + inverter->curgenerationb;
		inverter++;
	}

	printfloatmsg("System generation", temp);
	
	return temp;
}

int transltgen(char *buff, float ltgen)		//增加历史发电量
{
	int i,k;
	char ltgentemp[10]={'\0'};
	int temp = (int)ltgen;

	sprintf(ltgentemp, "%d", temp);
	k=LIFETIMEGENLEN-strlen(ltgentemp);

	if(k>=0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, ltgentemp);
	}
	else
	{
		strncat(buff, ltgentemp,LIFETIMEGENLEN);
	}
	return 0;
}

int transdv(char *buff, float dv)				//增加逆变器的直流电压
{
	int i,k;
	char dvtemp[10]={'\0'};
	int temp = (int)dv;
	
	sprintf(dvtemp, "%d", temp);
	k=DVLENGTH-strlen(dvtemp);
	
	if(k>=0)
	{
		for(i=0; i<k; i++)		//直流电压占5个字节，与EMA协议规定
		{
			strcat(buff, "A");
		}
		strcat(buff, dvtemp);
	}
	else
	{
		strncat(buff, dvtemp,DVLENGTH);
	}
	return 0;
}

int transdi(char *buff, float di)				//增加逆变器的直流电流
{
	int i,k;
	char ditemp[10]={'\0'};
	int temp = (int)di;

	sprintf(ditemp, "%d", temp);
	k=DILENGTH-strlen(ditemp);

	if(k>=0)
	{
		for(i=0; i<k; i++)		//直流电流占3个字节，与EMA协议规定
		{
			strcat(buff, "A");
		}
		strcat(buff, ditemp);

	}
	else
	{
		strncat(buff, ditemp,DILENGTH);
	}
	return 0;
}

int transpower(char *buff, int power)			//增加逆变器的功率
{
	int i,k;
	char powertemp[10]={'\0'};

	sprintf(powertemp, "%d", power*100);
	k=POWERLENGTH-strlen(powertemp);

	if(k>0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, powertemp);
	}
	else
	{
		strncat(buff, powertemp,POWERLENGTH);
	}
	return 0;
}

int transreactivepower(char *buff, int reactivepower)			//增加逆变器的无功功率
{
	int i,k;
	char powertemp[10]={'\0'};

	sprintf(powertemp, "%d", reactivepower*100);
	k=REACTIVEPOWERLENGTH-strlen(powertemp);

	if(k>=0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, powertemp);
	}
	else
	{
		strncat(buff, powertemp,REACTIVEPOWERLENGTH);
	}
	return 0;
}

int transactivepower(char *buff, int activepower)			//增加逆变器的有功功率
{
	int i,k;
	char powertemp[10]={'\0'};

	sprintf(powertemp, "%d", activepower*100);
	k=ACTIVEPOWERLENGTH-strlen(powertemp);
	if(k>=0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, powertemp);
	}
	else
	{
		strncat(buff, powertemp,ACTIVEPOWERLENGTH);
	}
	return 0;
}

int transfrequency(char *buff, float frequency)			//增加逆变器的电网频率
{
	int i,k;
	char fretemp[10]={'\0'};
	int temp = (int)frequency;

	sprintf(fretemp, "%d", temp);
	k=FREQUENCYLENGTH-strlen(fretemp);
	if(k>=0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, fretemp);
	}
	else
	{
		strncat(buff, fretemp,FREQUENCYLENGTH);
	}
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
	int i,k;
	char gvtemp[10]={'\0'};

	sprintf(gvtemp, "%d", voltage);
	k=GRIDVOLTLENGTH-strlen(gvtemp);

	if(k>=0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, gvtemp);
	}
	else
	{
		strncat(buff, gvtemp,GRIDVOLTLENGTH);
	}
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
	
	return 0;
}

int transcurgen(char *buff, float gen)		//增加逆变器的当前一轮发电量
{
	int i,k;
	char gentemp[10]={'\0'};
	int temp = (int)gen;
	sprintf(gentemp, "%d", temp);
	k=CURGENLENGTH-strlen(gentemp);
	if(k>=0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, gentemp);
	}
	else									//如果长度超过最大长度，得出值为负值，则直接拼接CURGENLENGTH个字节
	{
		strncat(buff, gentemp,CURGENLENGTH);
	}

	return 0;
}

int transsyspower(char *buff, int syspower)			//增加系统功率
{
	int i,k;
	char syspowertemp[10]={'\0'};

	sprintf(syspowertemp, "%d", syspower*100);
	k=SYSTEMPOWERLEN-strlen(syspowertemp);
	if(k>=0)
	{
		for(i=0; i<k; i++)
		{
			strcat(buff, "A");
		}
		strcat(buff, syspowertemp);
	}
	else
	{
		strncat(buff, syspowertemp,SYSTEMPOWERLEN);
	}
	return 0;
}


int protocol_optimizer(struct inverter_info_t *firstinverter, char *sendcommanddatetime)
{
	int i;
	char temp[256] = {'\0'};
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL]={'\0'};
	struct inverter_info_t *inverter = firstinverter;//printf("protocal1\n");

	printmsg("AAAAAAAAAAAAAAAAA");
	sprintf(temp, "APS160000000010001%s%010d%010d%010d%010d%s%03d000000END", ecu.id, (int)(ecu.input_power_optimizer*1000), (int)(ecu.output_power_optimizer*1000),
			(int)(ecu.input_energy_optimizer*1000), (int)(ecu.output_energy_optimizer*1000000), sendcommanddatetime, ecu.count);
	strcat(buff, temp);


	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++)
	{
		if(1 == inverter->dataflag)
		{
			//YC500机型
			memset(temp, '\0', sizeof(temp));

			sprintf(temp, "%s%06d%06d%06d%010d%03d1%06d%06d%06d%010d2%06d%06d%06d%010dEND", inverter->inverterid, (int)(inverter->output_voltage*1000), (int)(inverter->output_current*1000),
					(int)(inverter->output_power*1000), (int)(inverter->output_energy_optimizer*1000000), (int)(100+inverter->temperature), (int)(inverter->input_voltage_pv1*1000), (int)(inverter->input_current_pv1*1000), (int)(inverter->input_power_pv1*1000),
					(int)(inverter->input_energy_optimizer_pv1*1000000), (int)(inverter->input_voltage_pv2*1000), (int)(inverter->input_current_pv2*1000), (int)(inverter->input_power_pv2*1000),(int)(inverter->input_energy_optimizer_pv2*1000000));
			strcat(buff, temp);

		}
		inverter++;
	}

	memset(temp, '\0', sizeof(temp));
	sprintf(temp, "%05d", strlen(buff));
	for(i=0; i<strlen(temp); i++)
		buff[5+i] = temp[i];

	strcat(buff, "\n");

	printmsg(buff);


	save_record(buff,sendcommanddatetime);//printf("point9 ");			//把发送给EMA的记录保存在数据库中
//	print2msg("Record", buff);

	return 0;
}


//把所有逆变器的数据按照ECU和EMA的通信协议转换，见协议
int protocol(struct inverter_info_t *firstinverter, char *sendcommanddatetime)
{
	int i;
	char temp[50] = {'\0'};
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL]={'\0'};
	struct inverter_info_t *inverter = firstinverter;//printf("protocal1\n");
	strcat(buff, "APS1300000AAAAAAA1");
	strcat(buff, ecu.id);//printf("protocal3 ");
	transsyspower(buff, ecu.system_power);//printf("protocal4 ");
	transsyscurgen(buff, ecu.current_energy);//printf("protocal5 ");
	transltgen(buff, ecu.life_energy*10.0);//printf("protocal6 ");
	strcat(buff, sendcommanddatetime);//printf("protocal7\n");

	sprintf(temp, "%d", ecu.count);
	for(i=0; i<(3-strlen(temp)); i++)
		strcat(buff, "A");
	strcat(buff, temp);

	if(!ecu.zoneflag)
		strcat(buff, "0");
	else
		strcat(buff, "1");
	strcat(buff, "00000END");


	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->inverterid)); i++)
	{
		if(1 == inverter->dataflag)
		{
			//printdecmsg("inverter->model",inverter->model);
			if((1 == inverter->model)||(2 == inverter->model))	//YC250机型
			{
				strcat(buff, inverter->inverterid);//printf("protocal11");
				strcat(buff, "0");
				transdv(buff, inverter->dv);//printf("protocal12");
				transdi(buff, inverter->di);//printf("protocal13");
				transpower(buff, inverter->op);//printf("protocal14");
				transfrequency(buff, inverter->gf*10.0);//printf("protocal15");
				transtemperature(buff, inverter->it);//printf("protocal16");
				transgridvolt(buff, inverter->gv);//printf("protocal17");
				transstatus(buff, inverter->status);//printf("protocal18");
				transcurgen(buff, inverter->curgeneration*1000000.0);//printf("protocal19\n");
				strcat(buff, "END");
			}
			else if((3 == inverter->model)||(4 == inverter->model))	//YC500机型
			{
				strcat(buff, inverter->inverterid);
				strcat(buff, "03");
				transgridvolt(buff, inverter->gv);
				transfrequency(buff, inverter->gf*10.0);
				transtemperature(buff, inverter->it);
				transreactivepower(buff, inverter->reactive_power);
				transactivepower(buff, inverter->active_power);
				transcurgen(buff, inverter->output_energy*1000000.0);
				strcat(buff, "A");
				transdv(buff, inverter->dv);
				transdi(buff, inverter->di);
		//		transcurgen(buff, inverter->curaccgen*1000000.0);
				transcurgen(buff, inverter->curgeneration*1000000.0);
				transpower(buff, inverter->op);
				transstatus(buff, inverter->status);
				strcat(buff, "B");
				transdv(buff, inverter->dvb);
				transdi(buff, inverter->dib);
		//		transcurgen(buff, inverter->curaccgenb*1000000.0);
				transcurgen(buff, inverter->curgenerationb*1000000.0);
				transpower(buff, inverter->opb);
				transstatus(buff, inverter->statusb);
				strcat(buff, "END");

			}
			else if((5 == inverter->model)||(6 == inverter->model))		//YC1000CN机型
			{
				strcat(buff, inverter->inverterid);
				strcat(buff, "04");
				transdv(buff, inverter->dv);
				transfrequency(buff, inverter->gf*10.0);
				transtemperature(buff, inverter->it);
				strcat(buff, "1");
				transdi(buff, inverter->di);
				transgridvolt(buff, inverter->gv);
				transreactivepower(buff, inverter->reactive_power);
				transactivepower(buff, inverter->active_power);
				transcurgen(buff, inverter->output_energy*1000000.0);
			//	transcurgen(buff, inverter->curaccgen*1000000.0);
				transcurgen(buff, inverter->curgeneration*1000000.0);
				transpower(buff, inverter->op);
				transstatus(buff, inverter->status);
				strcat(buff, "2");
				transdi(buff, inverter->dib);
				transgridvolt(buff, inverter->gvb);
				transreactivepower(buff, inverter->reactive_powerb);
				transactivepower(buff, inverter->active_powerb);
				transcurgen(buff, inverter->output_energyb*1000000.0);
			//	transcurgen(buff, inverter->curaccgenb*1000000.0);
				transcurgen(buff, inverter->curgenerationb*1000000.0);
				transpower(buff, inverter->opb);
				transstatus(buff, inverter->statusb);
				strcat(buff, "3");
				transdi(buff, inverter->dic);
				transgridvolt(buff, inverter->gvc);
				transreactivepower(buff, inverter->reactive_powerc);
				transactivepower(buff, inverter->active_powerc);
				transcurgen(buff, inverter->output_energyc*1000000.0);
			//	transcurgen(buff, inverter->curaccgenc*1000000.0);
				transcurgen(buff, inverter->curgenerationc*1000000.0);
				transpower(buff, inverter->opc);
				transstatus(buff, inverter->statusc);
				strcat(buff, "4");
				transdi(buff, inverter->did);
				transcurgen(buff, inverter->curgenerationd*1000000.0);
			//	transcurgen(buff, inverter->curaccgend*1000000.0);
				transpower(buff, inverter->opd);
				strcat(buff, "END");
			}
		}
		inverter++;
	}
	print2msg("Record", buff);
	memset(temp, '\0', 50);
	sprintf(temp, "%d", strlen(buff));


	for(i=0; i<(5-strlen(temp)); i++)
		buff[5+i] =  'A';
	for(i=0; i<strlen(temp); i++)
		buff[5+5-strlen(temp)+i] = temp[i];

	strcat(buff, "\n");
	save_record(buff,sendcommanddatetime);//printf("point9 ");			//把发送给EMA的记录保存在数据库中
//	print2msg("Record", buff);

	return 0;
}


