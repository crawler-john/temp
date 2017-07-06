#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"


extern ecu_info ecu;
extern sqlite3 *db;
extern int zbmodem;


int queryInverters(inverter_info *firstinverter)		//获取每个逆变器的数据
{
	int i, j,ret;
	inverter_info *curinverter = firstinverter;
	int count=0, syspower=0;
	float curenergy=0;
	FILE *fp;

	for(i=0;i<3;i++)
	{
		if(-1==zb_test_communication())
			zigbee_reset();
		else
			break;
	}

	for(j=0; j<5; j++)
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)			//每个逆变器最多要5次数据
		{
			process_all(firstinverter);
			if(0 == curinverter->dataflag)
			{
				if(1!=curinverter->bindflag)
				{
					if(1==zb_turnoff_limited_rptid(curinverter->shortaddr,curinverter))
					{
						curinverter->bindflag=1;			//绑定逆变器标志位置1
						update_inverter_bind_flag(curinverter);
					}
				}

				if((0==curinverter->model)&&(1==curinverter->bindflag))
				{
					if(1 == zb_query_inverter_info(curinverter))
						update_inverter_model_version(curinverter);
				}

				if((0!=curinverter->model)&&(1==curinverter->bindflag))
				{
					zb_query_data(curinverter);
					usleep(200000);
				}
			}
			curinverter++;
		}
	}

	ecu.polling_total_times++;				//ECU总轮询加1 ,ZK

	fp = fopen("/tmp/disconnect.txt", "w");
	if(fp)
	{
		curinverter = firstinverter;
		for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)						//统计当前一天逆变器与ECU没通信上的总次数， ZK
		{
			if((0 == curinverter->dataflag)&&(12 == strlen(curinverter->id)))
			{
				curinverter->disconnect_times++;
				fprintf(fp, "%s-%d-%d\n", curinverter->id,curinverter->disconnect_times,ecu.polling_total_times);
			}
			else if((1 == curinverter->dataflag)&&(12 == strlen(curinverter->id)))
			{
				fprintf(fp, "%s-%d-%d\n", curinverter->id,curinverter->disconnect_times,ecu.polling_total_times);
			}
		}
		fclose(fp);
	}

	curinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)		//统计连续没有获取到数据的逆变器 ZK，一旦接受到数据，此变量会清0
	{
		if((0 == curinverter->dataflag)&&(12 == strlen(curinverter->id)))
			curinverter->no_getdata_num++;
	}

	curinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++){							//统计当前多少个逆变器
		if((1 == curinverter->dataflag)&&(12 == strlen(curinverter->id)))
			count++;
	}
	ecu.count = count;

	curinverter = firstinverter;
	for(syspower=0, i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++){		//计算当前一轮系统功率
		if(1 == curinverter->dataflag){
			syspower += curinverter->op;
			syspower += curinverter->opb;
			syspower += curinverter->opc;
			syspower += curinverter->opd;
		}
	}
	ecu.system_power = syspower;

	curinverter = firstinverter;
	for(curenergy=0, i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++){		//计算当前一轮发电量
		if(1 == curinverter->dataflag){
			curenergy += curinverter->output_energy;
			curenergy += curinverter->output_energyb;
			curenergy += curinverter->output_energyc;
		}
	}
	ecu.current_energy = curenergy;

	update_tmpdb(firstinverter);

	fp=fopen("/tmp/id_without_bind.txt","w");								//为了统计显示有短地址但是没有绑定的逆变器ID
	if(fp)
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)
		{
			if((1!=curinverter->bindflag)&&(0!=curinverter->shortaddr))
			{
				fputs(curinverter->id,fp);
				fputs("\n",fp);
			}
		}
		fclose(fp);
	}

	fp = fopen("/tmp/signalstrength.txt", "w");
	if(fp)
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)	 //统计每台逆变器的信号强度， ZK
		{
			fprintf(fp, "%s%X\n", curinverter->id,curinverter->signalstrength);
		}
		fclose(fp);
	}

	write_gfdi_status(firstinverter);
	write_turn_on_off_status(firstinverter);
	save_turn_on_off_changed_result(firstinverter);
	save_gfdi_changed_result(firstinverter);

	return ecu.count;
}
