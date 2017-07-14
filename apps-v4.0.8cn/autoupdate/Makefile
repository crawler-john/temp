CC = arm-linux-gnueabihf-gcc
SRC = autoupdate.c
OBJ = autoupdate
CFLAG = -I include/

make :
	make autoupdate
	make updatemanager
	make autoupdate_restart

autoupdate : autoupdate.c
	$(CC) -o autoupdate autoupdate.c
updatemanager : updatemanager.c
	$(CC) -o updatemanager updatemanager.c
autoupdate_restart : autoupdate_restart.c
	$(CC) -o autoupdate_restart autoupdate_restart.c

strip :
	arm-linux-gnueabihf-strip autoupdate updatemanager autoupdate_restart

clean :
	rm autoupdate updatemanager

