CC = arm-linux-gnueabihf-gcc
src = gprs.c
TARGET = gprs
CFLAGS = -lsqlite3 -ldl -lpthread -I include/ -L lib/

make :
	make gprs
	make gprsmonitor

$(TARGET):$(src)
	$(CC) $(src) -o $(TARGET) $(CFLAGS)
gprsmonitor : gprsmonitor.c
	$(CC) -o gprsmonitor gprsmonitor.c

strip:
	arm-linux-gnueabihf-strip $(TARGET) gprsmonitor

clean:
	rm $(TARGET) gprsmonitor
