CC=arm-linux-gnueabihf-gcc
target=wifind
source=wifind.c

$(target):$(source)
	$(CC) -o $(target) $(source)

strip:
	arm-linux-gnueabihf-strip $(target)

clean:
	rm $(target)
