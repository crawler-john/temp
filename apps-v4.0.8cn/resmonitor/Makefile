CC = arm-linux-gnueabihf-gcc
src = resmonitor.c
TARGET = resmonitor
CFLAGS = -lsqlite3 -ldl -lpthread -I include/ -L lib/

$(TARGET):$(src)
	$(CC) $(src) -o $(TARGET) $(CFLAGS)

strip:
	arm-linux-gnueabihf-strip $(TARGET)

clean:
	rm $(TARGET)
