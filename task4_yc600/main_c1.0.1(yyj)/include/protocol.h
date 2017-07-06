#ifndef PROTOCOL_H
#define PROTOCOL_H

int transdv(char *buff, float dv);
int transsyscurgen(char *buff, float curgen);
float calsystemgeneration(struct inverter_info_t *inverter);
int transltgen(char *buff, float ltgen);
int transdv(char *buff, float dv);
int transdi(char *buff, float di);
int transpower(char *buff, int power);
int transreactivepower(char *buff, int reactivepower);
int transactivepower(char *buff, int activepower);
int transfrequency(char *buff, float frequency);
int transtemperature(char *buff, int temperature);
int transgridvolt(char *buff, int voltage);
int transstatus(char *buff, char *status);
int transcurgen(char *buff, float gen);
int transsyspower(char *buff, int syspower);
int protocol(struct inverter_info_t *firstinverter, char *sendcommanddatetime);
int protocol_status(struct inverter_info_t *firstinverter, char *datetime);

#endif
