CC = arm-linux-gnueabihf-gcc
TARGET = macapp
SRC = macapp.c

$(TARGET):$(SRC)
	$(CC) -o $(TARGET) $(SRC)

strip:
	arm-linux-gnueabihf-strip $(TARGET)

clean:
	rm $(TARGET)
