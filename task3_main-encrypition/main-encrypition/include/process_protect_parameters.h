extern int determine_process_protect_parameters(struct inverter_info_t *inverter);
extern int read_reference_protect_parameters(struct inverter_info_t *firstinverter);
extern int compare_protect_parameters(char *protect_parameters);
extern int set_protect_parameters(struct inverter_info_t *inverter);
extern int process_protect_parameters(struct inverter_info_t *firstinverter);
