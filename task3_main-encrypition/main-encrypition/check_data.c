#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "variation.h"

int ecu_type;	//1:SAA; 2:NA; 3:MX

int get_ecu_type()			//在初始化initinverter()函数调用
{
	FILE *fp;
	char version[256] = {'\0'};
	
	fp = fopen("/etc/yuneng/area.conf", "r");
	if(fp)
	{
		fgets(version, sizeof(version), fp);
		if('\n' == version[strlen(version)-1])
			version[strlen(version)-1] = '\0';
		if(!strncmp(&version[strlen(version)-2], "MX", 2))
			ecu_type = 3;
		else if(!strncmp(&version[strlen(version)-2], "NA", 2))
			ecu_type = 2;
		else
			ecu_type = 1;
		fclose(fp);
	}
	
	return 0;
}

int correct_power_yc500(struct inverter_info_t *inverter)
{
	if(((strcmp(inverter->inverterid, "403000056551")>0) && (strcmp(inverter->inverterid, "403999999999")<0))|| ((strcmp(inverter->inverterid, "404000104436")>0) && (strcmp(inverter->inverterid, "404999999999")<0)))
	{
		inverter->op = inverter->op * 1.07;
		inverter->opb = inverter->opb * 1.07;
	}
}

int check_yc200_yc250(struct inverter_info_t *inverter)		//在解析函数的最后调用
{
	if(inverter->dv > 1500)
		inverter->dv = 1500;
	if(inverter->dv < 0)
		inverter->dv = 0;
	if(inverter->di > 150)
		inverter->di = 150;
	if(inverter->di < 0)
		inverter->di = 0;
//	if(inverter->op > 300)
//		inverter->op = 300;
	if(inverter->op < 0)
		inverter->op = 0;
//	if(0 == inverter->duration)
//		inverter->op = 0;
	
	if(1 == ecu_type)	//SAA,45-55Hz
	{
		if(inverter->np > 55)
			inverter->np = 55;
		if(inverter->np < 0)
			inverter->np = 0;
	}
	else		//NA/MX,55-65Hz
	{
		if(inverter->np > 65)
			inverter->np = 65;
		if(inverter->np < 0)
			inverter->np = 0;
	}
	
	if(3 == ecu_type)		//MX,82-155V
	{
		if(inverter->nv > 155)
			inverter->nv = 155;
		if(inverter->nv < 0)
			inverter->nv = 0;
	}
	else if(2 == ecu_type)	//NA,181-298V
	{
		if(inverter->nv > 298)
			inverter->nv = 298;
		if(inverter->nv < 0)
			inverter->nv = 0;
	}
	else					//SAA,149-278V
	{
		if(inverter->nv > 278)
			inverter->nv = 278;
		if(inverter->nv < 0)
			inverter->nv = 0;
	}
	
	if(inverter->it > 150)
		inverter->it = 150;
	if(inverter->it < -50)
		inverter->it = -50;
	
	if(inverter->curgeneration > 0.999999)
		inverter->curgeneration = 0.999999;
	if(inverter->curgeneration < 0)
		inverter->curgeneration = 0;
	
	return 0;
}

int check_yc500(struct inverter_info_t *inverter)		//在解析函数的最后调用
{
	if(inverter->dv > 1500)
		inverter->dv = 1500;
	if(inverter->dvb > 1500)
		inverter->dvb = 1500;
	if(inverter->dv < 0)
		inverter->dv = 0;
	if(inverter->dvb < 0)
		inverter->dvb = 0;
	if(inverter->di > 150)
		inverter->di = 150;
	if(inverter->dib > 150)
		inverter->dib = 150;
	if(inverter->di < 0)
		inverter->di = 0;
	if(inverter->dib < 0)
		inverter->dib = 0;

	correct_power_yc500(inverter);

//	if(inverter->op > 300)
//		inverter->op = 300;
//	if(inverter->opb > 300)
//		inverter->opb = 300;
	if(inverter->op < 0)
		inverter->op = 0;
	if(inverter->opb < 0)
		inverter->opb = 0;
//	if(0 == inverter->duration)
//	{
//		inverter->op = 0;
//		inverter->opb = 0;
//	}
	
	if(1 == ecu_type)	//SAA,45-55Hz
	{
		if(inverter->np > 55)
			inverter->np = 55;
		if(inverter->np < 0)
			inverter->np = 0;
	}
	else		//NA/MX,55-65Hz
	{
		if(inverter->np > 65)
			inverter->np = 65;
		if(inverter->np < 0)
			inverter->np = 0;
	}
	
	if(3 == ecu_type)		//MX,82-155V
	{
		if(inverter->nv > 155)
			inverter->nv = 155;
		if(inverter->nv < 0)
			inverter->nv = 0;
	}
	else if(2 == ecu_type)	//NA,181-298V
	{
		if(inverter->nv > 298)
			inverter->nv = 298;
		if(inverter->nv < 0)
			inverter->nv = 0;
	}
	else					//SAA,149-278V
	{
		if(inverter->nv > 278)
			inverter->nv = 278;
		if(inverter->nv < 0)
			inverter->nv = 0;
	}
	
	if(inverter->it > 150)
		inverter->it = 150;
	if(inverter->it < -50)
		inverter->it = -50;
	
	if(inverter->curgeneration > 0.999999)
		inverter->curgeneration = 0.999999;
	if(inverter->curgenerationb > 0.999999)
		inverter->curgenerationb = 0.999999;
	if(inverter->curgeneration < 0)
		inverter->curgeneration = 0;
	if(inverter->curgenerationb < 0)
		inverter->curgenerationb = 0;
	
	return 0;
}

/*void modify_data(struct inverter_info_t *inverter)	//构造错误的数据，用于测试
{
//	inverter->dv = -1;
//	inverter->di = -1;
	inverter->op = -1;
//	inverter->np = 64.9;
//	inverter->nv = 81;
	inverter->it = -51;
//	inverter->curgeneration = -1;

}

void modify_data_yc500(struct inverter_info_t *inverter)	//构造错误的数据，用于测试
{
//	inverter->dv = -1;
//	inverter->dvb = -1;
//	inverter->di = -1;
//	inverter->dib = -1;
	inverter->op = -1;
	inverter->opb = -1;
//	inverter->np = 64.9;
//	inverter->nv = 81;
	inverter->it = -51;
//	inverter->curgeneration = -1;
//	inverter->curgenerationb = -1;

}*/
