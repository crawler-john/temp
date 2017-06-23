extern int init_lost_data_info();
extern int calibration_time_broadcast(struct inverter_info_t *firstinverter, int time_linux);
extern int save_time_to_database(struct inverter_info_t *firstinverter,int time_linux);
extern int fill_up_data(struct inverter_info_t *firstinverter,int rest_time,int thistime);
