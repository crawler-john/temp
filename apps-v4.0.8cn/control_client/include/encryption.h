#ifndef __ENCRYPTION_H__
#define __ENCRYPTION_H__

int response_inverter_encryption(const char *recvbuffer, char *sendbuffer);
int set_encryption(const char *recvbuffer, char *sendbuffer);
int clear_encryption(const char *recvbuffer, char *sendbuffer);
int response_inverter_encryption_time(const char *recvbuffer, char *sendbuffer);
int set_encryption_time(const char *recvbuffer, char *sendbuffer);

#endif	/*__ENCRYPTION_H__*/
