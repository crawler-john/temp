CC = arm-linux-gnueabihf-gcc
src = ntpapp.c ntpcomm.c
TARGET = ntpapp.exe

$(TARGET):$(src)
	$(CC) $(src) -o $(TARGET)
ntpmanager : ntpmanager.c
	$(CC) ntpmanager.c -o ntpmanager

strip:
	arm-linux-gnueabihf-strip $(TARGET) ntpmanager

clean:
	rm ntpapp.exe ntpmanager
