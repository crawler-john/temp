#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "variation.h"

#define DEBUG
#define DEBUGLOG

#ifdef DEBUGLOG
#define	FILE_NAME	"/home/main.log"
#define NEW_FILE_NAME	"/home/main2.log"

char date_time[20];

char *get_current_time()		//发给EMA记录时获取的时间，格式：年月日时分秒，如20120902142835
{
	time_t tm;
	struct tm record_time;    //记录时间

	time(&tm);
	memcpy(&record_time, localtime(&tm), sizeof(record_time));

	sprintf(date_time, "%04d-%02d-%02d %02d:%02d:%02d", record_time.tm_year+1900, record_time.tm_mon+1, record_time.tm_mday,
			record_time.tm_hour, record_time.tm_min, record_time.tm_sec);

	return date_time;
}

int get_file_size(char *file_name)			//获取数据库的文件大小，如果数据库超过30M后，每插入一条新的数据，就删除数据库中最早的数据
{
	struct stat file_info;
	int file_size=0;

	stat(file_name, &file_info);
	file_size = file_info.st_size;

	return file_size;
}

int file_rename()
{
	if(get_file_size(FILE_NAME)>=30000000) {
		rename(FILE_NAME, NEW_FILE_NAME);
	}

	return 0;
}
#endif

void printmsg(char *msg)		//打印字符串
{
#ifdef DEBUG
	printf("main==>%s!\n", msg);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s!\n", get_current_time(), msg);
		fclose(fp);
	}
	file_rename();
#endif
}

void print2msg(char *msg1, char *msg2)		//打印字符串
{
#ifdef DEBUG
	printf("main==>%s: %s!\n", msg1, msg2);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: %s!\n", get_current_time(), msg1, msg2);
		fclose(fp);
	}
	file_rename();
#endif
}

void printdecmsg(char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	printf("main==>%s: %d!\n", msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: %d!\n", get_current_time(), msg, data);
		fclose(fp);
	}
	file_rename();
#endif
}

void printfloatmsg(char *msg, float data)		//打印实数
{
#ifdef DEBUG
	printf("main==>%s: %f!\n", msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: %f!\n", get_current_time(), msg, data);
		fclose(fp);
	}
	file_rename();
#endif
}

void printhexmsg(char *msg, char *data, int size)		//打印十六进制数据
{
#ifdef DEBUG
	int i;

	printf("main==>%s: ", msg);
	for(i=0; i<size; i++)
		printf("%X, ", data[i]);
	printf("\n");
#endif
#ifdef DEBUGLOG
	FILE *fp;
	int j;

	fp = fopen(FILE_NAME, "a");
	if(fp)
	{
		fprintf(fp, "[%s] %s: ", get_current_time(), msg);
		for(j=0; j<size; j++)
			fprintf(fp, "%X, ", data[j]);
		fprintf(fp, "\n");
		fclose(fp);
	}
	file_rename();
#endif
}

void printecuinfo(ecu_info *ecu)
{
	print2msg("ECU ID", ecu->id);
	printdecmsg("PANID", ecu->panid);
	printdecmsg("Channel", ecu->channel);
	print2msg("IP", ecu->ip);
	printfloatmsg("Lifetime energy", ecu->life_energy);
	printfloatmsg("Current energy", ecu->current_energy);
	printdecmsg("System power", ecu->system_power);
	printdecmsg("Total", ecu->total);
	printdecmsg("Current count", ecu->count);
	printdecmsg("Type", ecu->type);
	printdecmsg("Zoneflag", ecu->zoneflag);
}

void printinverterinfo(inverter_info *inverter)
{
	printdecmsg("Inverter it", inverter->it);
	printfloatmsg("Inverter gf", inverter->gf);
	printdecmsg("Inverter curacctimet", inverter->curacctime);
	printfloatmsg("Inverter dv", inverter->dv);
	printfloatmsg("Inverter dvb", inverter->dvb);
	printfloatmsg("Inverter di", inverter->di);
	printfloatmsg("Inverter dib", inverter->dib);
	printfloatmsg("Inverter dic", inverter->dic);
	printfloatmsg("Inverter did", inverter->did);
	printdecmsg("Inverter gv", inverter->gv);
	printdecmsg("Inverter gvb", inverter->gvb);
	printdecmsg("Inverter gvc", inverter->gvc);
	printfloatmsg("Inverter curaccgen", inverter->curaccgen);
	printfloatmsg("Inverter curaccgenb", inverter->curaccgenb);
	printfloatmsg("Inverter curaccgenc", inverter->curaccgenc);
	printfloatmsg("Inverter curaccgend", inverter->curaccgend);
	printfloatmsg("Inverter reactive_power", inverter->reactive_power);
	printfloatmsg("Inverter reactive_powerb", inverter->reactive_powerb);
	printfloatmsg("Inverter reactive_powerc", inverter->reactive_powerc);
	printfloatmsg("Inverter active_power", inverter->active_power);
	printfloatmsg("Inverter active_powerb", inverter->active_powerb);
	printfloatmsg("Inverter active_powerc", inverter->active_powerc);
	printfloatmsg("Inverter output_energy", inverter->output_energy);
	printfloatmsg("Inverter output_energyb", inverter->cur_output_energyb);
	printfloatmsg("Inverter output_energyc", inverter->cur_output_energyc);
	printmsg("=================================================");
	printfloatmsg("preaccgen", inverter->preaccgen);
	printfloatmsg("preaccgenb", inverter->preaccgenb);
	printfloatmsg("preaccgenc", inverter->preaccgenc);
	printfloatmsg("preaccgend", inverter->preaccgend);
	printfloatmsg("pre_output_energy", inverter->pre_output_energy);
	printfloatmsg("pre_output_energyb", inverter->pre_output_energyb);
	printfloatmsg("pre_output_energyc", inverter->pre_output_energyc);
	printdecmsg("preacctime", inverter->preacctime);
	print2msg("last_report_time",inverter->last_report_time);
	printdecmsg("Inverter op", inverter->op);
	printdecmsg("Inverter opb", inverter->opb);
	printdecmsg("Inverter opc", inverter->opc);
	printmsg("=================================================");
}
