#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mydatabase.h"
#include "mydebug.h"

/* 打开数据库 */
int open_db(const char *dbname, sqlite3 **ppDb)
{
	if(sqlite3_open(dbname, ppDb)){
		debug_msg("%s: %s", dbname, sqlite3_errmsg(*ppDb));
		sqlite3_close(*ppDb);
		return -1;
	}
	return 0;
}

/* 关闭数据库 */
int close_db(sqlite3 *db)
{
	if(sqlite3_close(db)){
		debug_msg("%s", sqlite3_errmsg(db));
		return -1;
	}
	return 0;
}

/* 创建表单 */
int create_table(sqlite3* db, const char *sql)
{
	int i;
	char *errmsg = 0;

	for(i=0; i<3; i++){
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg)){
			//创建失败，继续执行
			debug_msg("Create Table Failed: %s", errmsg);
			sleep(1);
		}
		else{
			//创建成功
			return 0;
		}
	}
	debug_msg("SQL:%s", sql);
	return -1;
}

/* 查询数据（不使用回调函数） */
int get_data(sqlite3 *db, const char *sql,
		char ***pazResult, int *pnRow, int *pnColumn)
{
	int i;
	char *errmsg = 0;

	for(i=0; i<3; i++){
		if(sqlite3_get_table(db, sql, pazResult, pnRow, pnColumn, &errmsg)){
			//查询失败，继续查询
			debug_msg("Query Failed: %s", errmsg);
			sqlite3_free_table(*pazResult);
			sleep(1);
		}
		else{
			//查询成功
			return 0;
		}
	}
	debug_msg("SQL:%s", sql);
	return -1;
}

/* 查询数据（使用回调函数） */
int query_data(sqlite3 *db, const char *sql,
		int (*callback)(void*,int,char**,char**), char *str)
{
	int i;
	char *errmsg = 0;

	for(i=0; i<3; i++){
		if(sqlite3_exec(db, sql, callback, str, &errmsg)){
			//查询失败，继续查询
			debug_msg("Query Failed: %s", errmsg);
			sleep(1);
		}
		else{
			//查询成功
			return 0;
		}
	}
	debug_msg("SQL:%s", sql);
	return -1;
}

/* 插入数据 */
int insert_data(sqlite3* db, const char *sql)
{
	int i;
	char *errmsg = 0;

	for(i=0; i<3; i++){
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg)){
			//插入失败，继续执行
			debug_msg("Insert Failed: %s", errmsg);
			sleep(1);
		}
		else{
			//插入成功
			return 0;
		}
	}
	debug_msg("SQL:%s", sql);
	return -1;
}

/* 删除数据 */
int delete_data(sqlite3* db, const char *sql)
{
	int i;
	char *errmsg = 0;

	for(i=0; i<3; i++){
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg)){
			//删除失败，继续执行
			debug_msg("Delete Failed: %s", errmsg);
			sleep(1);
		}
		else{
			//删除成功
			return 0;
		}
	}
	debug_msg("SQL:%s", sql);
	return -1;
}

/* 替换数据 */
int replace_data(sqlite3* db, const char *sql)
{
	int i;
	char *errmsg = 0;

	for(i=0; i<3; i++){
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg)){
			//替换失败，继续执行
			debug_msg("Replace Failed: %s", errmsg);
			sleep(1);
		}
		else{
			//替换成功
			return 0;
		}
	}
	debug_msg("SQL:%s", sql);
	return -1;
}

/* 更新数据 */
int update_data(sqlite3* db, const char *sql)
{
	int i;
	char *errmsg = 0;

	for(i=0; i<3; i++){
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg)){
			//更新失败，继续执行
			debug_msg("Update Failed: %s", errmsg);
			sleep(1);
		}
		else{
			//更新成功
			return 0;
		}
	}
	debug_msg("SQL:%s", sql);
	return -1;
}

/* 将信息保存到process_result表中 */
int save_to_process_result(int cmd_id, char *savebuffer)
{
	sqlite3 *db;
	char sql[1024] = {'\0'};
	char msg_length[6] = {'\0'};


	//计算消息长度并填入消息中
	sprintf(msg_length, "%05d", strlen(savebuffer));
	strncpy(&savebuffer[5], msg_length, 5);
	strcat(savebuffer, "\n");

	if(!open_db("/home/database.db", &db))
	{
		//创建process_result表单
		strcpy(sql, "CREATE TABLE IF NOT EXISTS process_result"
				"(item INTEGER, result VARCHAR, flag INTEGER, PRIMARY KEY(item))");
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "REPLACE INTO process_result (item, result, flag) "
				"VALUES (%d, '%s', 1)", cmd_id, savebuffer);
		if(!replace_data(db, sql)){
			savebuffer[strlen(savebuffer)-1] = '\0';
			debug_msg("Saved:%s", savebuffer);
			sleep(2);
		}

		close_db(db);
		return 0;
	}
	return -1;
}
