CC=arm-linux-gnueabihf-gcc
target=monitor.exe
source=monitor.c

$(target):$(source)
	$(CC) -o $(target) $(source)

strip:
	arm-linux-gnueabihf-strip $(target)

clean:
	rm $(target)
