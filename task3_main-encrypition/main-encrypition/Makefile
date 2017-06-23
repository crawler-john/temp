CC = arm-linux-gnueabihf-gcc
SRC = 3501.c resolve.c access.c calculate.c database.c datetime.c display.c file.c gprs.c main.c plc.c protocal.c socket.c serial.c setpower.c debug.c save_historical_data.c process_protect_parameters.c check_data.c set_protection_parameters.c set_protection_parameters_inverter.c turn_on_off.c clear_gfdi.c set_grid_environment.c set_ird.c signal_strength_detection.c restore_inverter.c get_reply_from_serial.c update_inverter.c get_inverter_version.c check_inverters.c fill_up_data.c process_paras_changed.c  plcuid.c encrypition.c
OBJ = main.exe
#显示打印信息
DEFINES += -DDEBUG
#将日志写入到文件
DEFINES += -DDEBUGLOG
CFLAG = -I include/ -L lib/ -lsqlite3 -lpthread $(DEFINES)


$(OBJ) : $(SRC)
	$(CC) -o $(OBJ) $(SRC) $(CFLAG)

strip :
	arm-linux-gnueabihf-strip $(OBJ)

clean:
	rm $(OBJ)
