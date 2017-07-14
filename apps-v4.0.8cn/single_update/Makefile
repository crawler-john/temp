CC = arm-linux-gnueabihf-gcc
SRC = single_update.c
OBJ = single_update
CFLAG = -I include/

make :
	make single_update
	make single_update_monitor

single_update : single_update.c
	$(CC) -o single_update single_update.c
single_update_monitor : single_update_monitor.c
	$(CC) -o single_update_monitor single_update_monitor.c

strip :
	arm-linux-gnueabihf-strip single_update single_update_monitor

clean :
	rm single_update single_update_monitor

