#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
#include "variation.h"

extern int plcmodem;		//PLC的文件描述符
extern unsigned char ccuid[7];		//ECU3501的ID

int update_inverter_version(char *id, char version)					//更新逆变器的机型码和软件版本号
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db=NULL;

	memset(sql, '\0', sizeof(sql));
	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))
		return -1;

	sprintf(sql, "UPDATE id SET item=%d WHERE id='%s' ", version, id);
	sqlite3_exec_3times(db, sql);

	sqlite3_close( db );

}

int clear_get_inverter_version_flag(char *id)
{
	char sql[1024] = {'\0'};
	sqlite3 *db;

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))		//create a database
		return 0;

	sprintf(sql, "UPDATE inverter_version SET get_flag=0 WHERE id='%s'", id);
	sqlite3_exec_3times(db, sql);

	sqlite3_close(db);
}

int query_inverter_version(char *id)		//请求逆变器的机型码
{
	int i=0, ret;
	char sendbuff[256];
	char readbuff[256];
	unsigned short check = 0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x01;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x11;
	sendbuff[5] = ccuid[0];
	sendbuff[6] = ccuid[1];
	sendbuff[7] = ccuid[2];
	sendbuff[8] = ccuid[3];
	sendbuff[9] = ccuid[4];
	sendbuff[10] = ccuid[5];
	sendbuff[11] = ((id[0]-0x30)<<4) + (id[1]-0x30);		//TNID
	sendbuff[12] = ((id[2]-0x30)<<4) + (id[3]-0x30);		//TNID
	sendbuff[13] = ((id[4]-0x30)<<4) + (id[5]-0x30);		//TNID
	sendbuff[14] = ((id[6]-0x30)<<4) + (id[7]-0x30);		//TNID
	sendbuff[15] = ((id[8]-0x30)<<4) + (id[9]-0x30);		//TNID
	sendbuff[16] = ((id[10]-0x30)<<4) + (id[11]-0x30);		//TNID
	sendbuff[17] = 0x4F;
	sendbuff[18] = 0x00;
	sendbuff[19] = 0x00;
	sendbuff[20] = 0xDC;
	sendbuff[21] = 0x11;

	for(i=2; i<22; i++)
		check = check + sendbuff[i];

	sendbuff[22] = check >> 8;		//CHK
	sendbuff[23] = check;		//CHK
	sendbuff[24] = 0xFE;
	sendbuff[25] = 0xFE;

	write(plcmodem, sendbuff, 26);
	ret = get_reply_from_serial(plcmodem, 5, 0, readbuff);


	if((37 == ret)&&(0xFB == readbuff[0])&&(0xFB == readbuff[1])&&(0x01 == readbuff[2])&&(0xFE == readbuff[35])&&(0xFE == readbuff[36]))
	{
		update_inverter_version(id, (readbuff[22]*1000+readbuff[25]*256+readbuff[26]));
		printdecmsg(id, readbuff[22]*1000+readbuff[25]*256+readbuff[26]);
		return readbuff[22]*1000+readbuff[25]*256+readbuff[26];
	}
	else
	{
		print2msg(id, "Failed to get inverter version");
		return 0;
	}
}

int get_inverter_version_single()
{
	char sql[1024] = {'\0'};
	char *zErrMsg=0;
	int nrow=0,ncolumn=0, i;
	char **azResult;
	sqlite3 *db;

	if(SQLITE_OK != sqlite3_open("/home/database.db", &db))		//create a database
		return 0;

	strcpy(sql, "SELECT id FROM inverter_version WHERE get_flag=1");
	if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
	{
		for(i=1; i<=nrow; i++)
		{
			clear_get_inverter_version_flag(azResult[i]);
			query_inverter_version(azResult[i]);
		}
	}
	sqlite3_free_table( azResult );

	sqlite3_close(db);

	return 0;
}

int get_inverter_version_all(struct inverter_info_t *firstinverter)
{
	int i;
	char buff[256]={'\0'};
	FILE *fp;
	struct inverter_info_t *curinverter = firstinverter;

	fp = fopen("/tmp/get_inverter_version.conf", "r");
	if(fp)
	{
		fgets(buff, sizeof(buff), fp);
		fclose(fp);
		
		if(strcmp("ALL", buff))
		{
			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
				query_inverter_version(curinverter->inverterid);
			
			fp = fopen("/tmp/get_inverter_version.conf", "w");
			fclose(fp);
		}
		
	}

	return 0;
}
