CC = arm-linux-gnueabihf-gcc
SRC = button.c
OBJ = button

$(OBJ) : $(SRC)
	$(CC) -o $(OBJ) $(SRC)

strip:
	arm-linux-gnueabihf-strip $(OBJ)

clean:
	rm $(OBJ)
