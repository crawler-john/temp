CC = arm-linux-gnueabihf-gcc
src = idwriter.c channel.c debug.c
obj = idwriter
CFLAGS = -lsqlite3 -ldl -lpthread -I include/ -L lib/

$(obj):$(src)
	$(CC) -o $(obj) $(src) $(CFLAGS)

strip:
	arm-linux-gnueabihf-strip idwriter

clean:
	rm $(obj)
