#ifndef __MYDEBUG_H__
#define __MYDEBUG_H__

char *system_time(void);
void debug_init(int flag, const char *name, const char *path);
void debug_msg(const char *fmt, ...);
void debug_err(const char *message);

#endif	/*__MYDEBUG_H__*/


