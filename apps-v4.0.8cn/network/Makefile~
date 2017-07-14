CC = arm-linux-gnueabihf-gcc
SRC = network.c
OBJ = network.exe

$(OBJ) : $(SRC)
	$(CC) -o $(OBJ) $(SRC)

strip:
	arm-linux-gnueabihf-strip $(OBJ)

clean:
	rm $(OBJ)
