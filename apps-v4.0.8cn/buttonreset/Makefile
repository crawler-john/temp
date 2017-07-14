CC = arm-linux-gnueabihf-gcc
SRC = buttonreset.c
OBJ = buttonreset

$(OBJ) : $(SRC)
	$(CC) -o $(OBJ) $(SRC)

strip:
	arm-linux-gnueabihf-strip $(OBJ)

clean:
	rm $(OBJ)
