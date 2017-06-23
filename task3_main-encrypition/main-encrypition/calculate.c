#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"

int calsystempower(struct inverter_info_t *inverter)		//计算系统当前总功率
{
	int systempower=0, i;
	
	for(i=0; i<MAXINVERTERCOUNT; i++){
		if(('1'==inverter->flag)&&(12==strlen(inverter->inverterid)))
			systempower = systempower + inverter->op + inverter->opb;
		inverter++;
	}
	
	return systempower;
}

/*float calsystemgeneration(struct inverter_info_t *inverter)		//计算当前一轮的系统发电量
{
	int i;
	float temp=0;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&('1'==inverter->flag)&&(12==strlen(inverter->inverterid)); i++){
		printf("curgenerarion: %f", inverter->curgeneration);
		temp = temp + inverter->curgeneration;
		inverter++;
	}
	
	printf("curgenerarion: %f", temp);
	
	return temp;
}*/
