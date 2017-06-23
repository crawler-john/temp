#define DEBUGINFO 0
extern char *get_current_time();
extern void printmsg(char *msg);
extern void print2msg(char *msg1, char *msg2);
extern void printdecmsg(char *msg, int data);
extern void printfloatmsg(char *msg, float data);
extern void printhexmsg(char *msg, char *data, int size);
