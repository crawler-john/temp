CC = arm-linux-gnueabihf-gcc
SRC = diagnosis_network.c
OBJ = diagnosis_network

$(OBJ) : $(SRC)
	$(CC) -o $(OBJ) $(SRC)

strip:
	arm-linux-gnueabihf-strip $(OBJ)

clean:
	rm $(OBJ)
