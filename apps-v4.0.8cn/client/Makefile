CC = arm-linux-gnueabihf-gcc
src = client.c debug.c
TARGET = client
CFLAGS = -lsqlite3 -ldl -lpthread -I include/ -L lib/

make :
	make client
	make clientmonitor

$(TARGET):$(src)
	$(CC) $(src) -o $(TARGET) $(CFLAGS)
clientmonitor : clientmonitor.c
	$(CC) -o clientmonitor clientmonitor.c

strip:
	arm-linux-gnueabihf-strip $(TARGET) clientmonitor

clean:
	rm $(TARGET) clientmonitor
