#ifndef __POWER_FACTOR_H__
#define __POWER_FACTOR_H__

int response_inverter_power_factor(const char *recvbuffer, char *sendbuffer);
int set_all_inverter_power_factor(const char *recvbuffer, char *sendbuffer);

#endif	/*__POWER_FACTOR_H__*/
