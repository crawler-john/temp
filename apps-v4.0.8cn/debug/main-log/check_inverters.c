#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"

int check_plc_connected(struct inverter_info_t *firstinverter, char *id)		//确认当前一轮逆变器是否有数据，有返回1，没有返回0
{
	int i;
	struct inverter_info_t *curinverter = firstinverter;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++,curinverter++)
	{
		if(!strcmp(curinverter->inverterid, id))
			if('1' == curinverter->curflag)
				return 1;
	}
	
	return 0;
}

int check_acc_time(struct inverter_info_t *firstinverter, char *id, int second)		//确认当前一轮逆变器是否有数据，有返回1，没有返回0
{
	int i;
	struct inverter_info_t *curinverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++,curinverter++)
	{
		if(!strcmp(curinverter->inverterid, id))
			if(curinverter->curacctime > second)
				return 1;
	}

	return 0;
}
