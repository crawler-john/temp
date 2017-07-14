CC = arm-linux-gnueabihf-gcc

manager:manager.c dhcp.c
	$(CC) manager.c dhcp.c -o manager

strip:
	arm-linux-gnueabihf-strip manager

clean:
	rm manager
