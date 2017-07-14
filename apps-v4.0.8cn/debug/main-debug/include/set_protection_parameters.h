extern char change_1_clearance_time_to_char(float time);		//外围过欠压、过欠频延保时间转成char型
extern char change_2_clearance_time_to_char(float time);		//内围过欠压、内围过欠频、内内围欠压延保时间转成char型
extern char change_start_time_to_char(float time);				//并网恢复时间、开机时间转成char型
extern int get_parameters_from_inverter(struct inverter_info_t *inverter);
extern int get_parameters_from_inverter_de(char *id);
extern int get_inverter_type(char *id);
