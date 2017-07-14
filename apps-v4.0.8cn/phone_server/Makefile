CC=arm-linux-gnueabihf-gcc
target=phone_server_monitor
source=phone_server_monitor.c
CFLAGS = -lsqlite3 -ldl -lpthread -I include/ -L lib/

phone_server : phone_server.c
	$(CC) -o phone_server phone_server.c $(CFLAGS)
$(target):$(source)
	$(CC) -o $(target) $(source)

strip:
	arm-linux-gnueabihf-strip $(target) phone_server

clean:
	rm $(target) phone_server
