sqlite3 *database_init();
sqlite3 *create_tmpdb();
float get_lifetime_power(sqlite3 *db);
void set_lifetime_power(sqlite3 *db, float power);
void insert_record(sqlite3 *db, char *sendbuff, char flag);
void insert_event(sqlite3 *db, char inverter_id[], char status[]);
int gettnuid( sqlite3 *db , struct inverter_info_t *inverter);
void insert_uid( sqlite3 *db , char inverter_id[][13]);
void todaypower(sqlite3 *db, float currentpower);
void resendrecord(sqlite3 *db);
int get_protect_parameters(int *max_vol, int *min_vol, int *max_fre, int *min_fre, int *boot_t);
int save_process_result(int item, char *result);
int save_inverter_parameters_result(struct inverter_info_t *inverter, int item, char *inverter_result);
int save_inverter_parameters_result_id(char *id, int item, char *inverter_result);