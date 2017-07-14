#ifndef __ENCRYPTION_H__
#define __ENCRYPTION_H__
int process_encrypition(struct inverter_info_t *firstinverter);
int initEncryption(struct inverter_info_t *firstinverter);
int encryption_heartbeat(void);
int process_encryption_alarm(struct inverter_info_t *firstinverter);

#endif /*__ENCRYPTION_H__*/
