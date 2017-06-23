#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "variation.h"

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

int turn_on_off(struct inverter_info_t *firstinverter)
{
	int i, j;
	char id[256] = {'\0'};
	struct inverter_info_t *curinverter = firstinverter;
	int flag;

	while(1){
		curinverter = firstinverter;
		memset(id, '\0', 256);
		if(!get_turn_on_off_flag(id, &flag))
			break;

		clear_turn_on_off_flag(id);
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){
			if(!strncmp(id, curinverter->inverterid, 12)){
				j = 0;
				if(1 == flag){
					clear_turned_off_operation(id);
					while(j<1){
						if(1 == wakeup(curinverter))
							break;
						j++;
					}
				}
				if(2 == flag){
					save_turned_off_operation(id);
					while(j<1){
						if(1 == shutdown(curinverter))
							break;
						j++;
					}
				}

				break;
			}
			curinverter++;
		}
	}

	return 0;
}
