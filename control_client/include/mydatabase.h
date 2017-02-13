#ifndef __MYDATABASE_H__
#define __MYDATABASE_H__
#include "sqlite3.h"

int open_db(const char *dbname, sqlite3 **ppDb);
int close_db(sqlite3 *db);
int create_table(sqlite3* db, const char *sql);
int get_data(
	sqlite3 *db,
	const char *sql,
	char ***pazResult,
	int *pnRow,
	int *pnColumn
);
int query_data(
	sqlite3 *db,
	const char *sql,
	int (*callback)(void*,int,char**,char**),
	char *str
);
int insert_data(sqlite3* db, const char *sql);
int delete_data(sqlite3* db, const char *sql);
int replace_data(sqlite3* db, const char *sql);
int update_data(sqlite3* db, const char *sql);
int save_to_process_result(int cmd_id, char *savebuffer);

#endif	/*__MYDATABASE_H__*/
