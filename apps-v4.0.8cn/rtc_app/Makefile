CC=arm-linux-gnueabihf-gcc
target=rtc_app
source=rtc_app.c

$(target):$(source)
	$(CC) -o $(target) $(source)

strip:
	arm-linux-gnueabihf-strip $(target)

clean:
	rm $(target)
