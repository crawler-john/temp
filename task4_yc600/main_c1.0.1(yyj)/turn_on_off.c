#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "variation.h"
#include <string.h>



int get_turn_on_off_flag(char *id, int *flag)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;
	sqlite3 *db;

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "SELECT id,set_flag FROM turn_on_off LIMIT 0,1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(1 == nrow){
		strcpy(id, azResult[2]);
		*flag = atoi(azResult[3]);
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	return nrow;
}

int clear_turn_on_off_flag(char *id)
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	sqlite3 *db;

	sqlite3_open("/home/database.db", &db);
	sprintf(sql, "DELETE FROM turn_on_off WHERE id='%s'", id);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	sqlite3_close(db);

	return 0;
}

int turn_on_off(inverter_info *firstinverter)
{
	int i, j,k;
	char id[256] = {'\0'};
	inverter_info *curinverter = firstinverter;
	int flag;


	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;
	sqlite3 *db;

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "SELECT id,set_flag FROM turn_on_off");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(nrow > 0){
		memset(sql,'\0',1024);
		sprintf(sql, "DELETE FROM turn_on_off");
		sqlite3_exec_3times(db, sql);
		for(i=0;i<nrow;i++)
		{
			strcpy(id, azResult[2*i+2]);
			flag = atoi(azResult[2*i+3]);
			curinverter = firstinverter;
			for(j=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); j++,curinverter++)
			{
				if(!strncmp(id, curinverter->id, 12)){
					k = 0;
					if(1 == flag){
						while(k<3){
							if(1 == zb_boot_single(curinverter))
								break;
							k++;
						}
					}
					if(2 == flag){
						while(k<3){
							if(1 == zb_shutdown_single(curinverter))
								break;
							k++;
						}
					}

					break;
				}
			}
		}
	}
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	return nrow;


//	while(1){
//		curinverter = firstinverter;
//		memset(id, '\0', 256);
//		if(!get_turn_on_off_flag(id, &flag))
//			break;
//
//		clear_turn_on_off_flag(id);
//		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++){
//			if(!strncmp(id, curinverter->id, 12)){
//				j = 0;
//				if(1 == flag){
//					while(j<3){
//						if(1 == zb_boot_single(curinverter))
//							break;
//						j++;
//					}
//				}
//				if(2 == flag){
//					while(j<3){
//						if(1 == zb_shutdown_single(curinverter))
//							break;
//						j++;
//					}
//				}
//
//				break;
//			}
//			curinverter++;
//		}
//	}

	return 0;
}
