CC = arm-linux-gnueabihf-gcc
CFLAGS = -lsqlite3 -ldl -lpthread -I include/ -L lib/

control_client_monitor : control_client_monitor.c
	$(CC) -o control_client_monitor control_client_monitor.c

strip:
	arm-linux-gnueabihf-strip control_client_monitor

clean:
	rm control_client_monitor
