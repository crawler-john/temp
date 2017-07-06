#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "variation.h"
#include "sqlite3.h"
#include <time.h>

#define DELAYTIME 90    //两台逆变器开启中间延时
int cc_fd;
int typeYC;
char data_ctset[20][8]={'\0'};   //CT设置值

char *Logger_time(void)
{
	static char record_time[20] = {'\0'};	//用于输出时间的静态缓存
	static struct tm *local_time;			//tm结构体指针
	static time_t now;						//时间类型变量

	time(&now);
	local_time = localtime(&now);
	sprintf(record_time, "%04d-%02d-%02d %02d:%02d:%02d",
			local_time->tm_year + 1900,
			local_time->tm_mon + 1,
			local_time->tm_mday,
			local_time->tm_hour,
			local_time->tm_min,
			local_time->tm_sec);

	return record_time;
}

int open_ccfd(void)		//打开串口
{
	int i;
	struct termios newtio;

	for(i=0;i<18;i++){                              //初始化ct设置参数
		memset(data_ctset[i],'0',sizeof(char));
		}

	cc_fd = open("/dev/ttyO3", O_RDWR | O_NOCTTY);		//打开串口
	if(cc_fd<0){
		perror("MODEMDEVICE");
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = B57600 | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 1;
	newtio.c_cc[VMIN] = 100;
	tcflush(cc_fd, TCIFLUSH);
	tcsetattr(cc_fd, TCSANOW, &newtio);
	return cc_fd;
}

int get_reply_from_ccfd(char *data)			//读取模块的返回帧
{
	int ret, size;
	fd_set rd;
	struct timeval timeout;

	FD_ZERO(&rd);
	FD_SET(cc_fd, &rd);
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;
	if(select(cc_fd+1, &rd, NULL , NULL, &timeout) <= 0)
	{
		//printmsg("Get reply time out~~");
		return -1;
	}
	else
	{
		size = read(cc_fd, data, 255);
//		printhexmsg("Reply", data, size);
		return size;
	}
}

int metergetdata(char *data,int chip)		//meter问询，返回存入data
{
	int res, i, j, sendtimes=0, recvbytes;
	struct timeval timeout;
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;

	sendbuff[0]  = 0xFB;
	sendbuff[1]  = 0xFB;
	sendbuff[2]  = 0x09;
	sendbuff[3]  = 0xE0;
	sendbuff[4]  = 0x00;
	sendbuff[5]  = 0x01;
	if(chip==1)
		sendbuff[6]  = 0x01;
	if(chip==2)
		sendbuff[6]  = 0x02;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	for(i=2;i<10;i++){
		check += sendbuff[i];
		}
	sendbuff[10]  = check >> 8;
	sendbuff[11]  = check;
	sendbuff[12] = 0xFE;
	sendbuff[13] = 0xFE;

//	printhexmsg("metergetdata",sendbuff,14);
	for(j=0; j<3; j++){              //发送3次
		write(cc_fd, sendbuff, 14);
		recvbytes=get_reply_from_ccfd(data);
//		printhexmsg("data", data,recvbytes);
		if ((70 == recvbytes)&& (0xFB == data[0])&& (0xFB == data[1])
			 &&(0x41 == data[2])
			 &&(0xD0 == data[3])
			 &&(0xFE == data[69]))
		{
			return 1;
		}
		else
		{
			usleep(100000);
			continue;
		}
	}
	return 0;
}

int getphasepower(float *data)
{
//	struct timeval timeout;
//	unsigned short check=0x00;
	char meter_data[255]={'\0'}; //meter数据
	float frequency,volA,volB,volC,varA,varB,varC,powerA,powerB,powerC;
	double energyA,energyB,energyC;
	int chip;

	memset(meter_data, '\0', sizeof(meter_data));
	for(chip=1;chip<3;chip++)   //Meter问询，分别问询芯片1和芯片2
	{
	memset(meter_data, '\0', sizeof(meter_data));
		if(1 == metergetdata(meter_data,chip))
		{
			frequency=(meter_data[8]*256.0+meter_data[9])/10;
			volA=(meter_data[10]*256.0+meter_data[11])/10;
			volB=(meter_data[12]*256.0+meter_data[13])/10;
			volC=(meter_data[14]*256.0+meter_data[15])/10;
			powerA=meter_data[16]*256.0+meter_data[17];
			powerB=meter_data[18]*256.0+meter_data[19];
			powerC=meter_data[20]*256.0+meter_data[21];
			varA=(meter_data[22]*256.0+meter_data[23]);
			varB=(meter_data[24]*256.0+meter_data[25]);
			varC=(meter_data[26]*256.0+meter_data[27]);
			energyA=(meter_data[28]*16777216.0+meter_data[29]*65536.0+meter_data[30]*256.0+meter_data[31])/3200*1000;
			energyB=(meter_data[32]*16777216.0+meter_data[33]*65536.0+meter_data[34]*256.0+meter_data[35])/3200*1000;
			energyC=(meter_data[36]*16777216.0+meter_data[37]*65536.0+meter_data[38]*256.0+meter_data[39])/3200*1000;

			if(0x01 == meter_data[6])  //通道1，数据存入文件
				{

				data[0]=frequency;
				data[1]=volA;
				data[2]=volB;
				data[3]=volC;
				data[7]=varA;
				data[8]=varB;
				data[9]=varC;
				data[10]=energyA;
				data[11]=energyB;
				data[12]=energyC;
						data[4]=powerA;
						data[5]=powerB;
						data[6]=powerC;

				}
			if(0x02 == meter_data[6])  //通道2，数据存入文件
				{
				data[13]=frequency;
				data[14]=volA;
				data[15]=volB;
				data[16]=volC;
				data[20]=varA;
				data[21]=varB;
				data[22]=varC;
				data[23]=energyA;
				data[24]=energyB;
				data[25]=energyC;
						data[17]=powerA;
						data[18]=powerB;
						data[19]=powerC;
				}
		}

	}
	return 0;
}

int read_min_phase_power(float *buff)    //读取最小相功率。正为正功率，负为负功率
{
//	float buff[8]={'\0'};
	int power_min;
	int i;
	int power[3]={0};

	getphasepower(buff);
	power[0]=buff[17]-buff[4];
	power[1]=buff[18]-buff[5];
	power[2]=buff[19]-buff[6];
//	printfloatmsg("buff17",buff[17]);
//	printfloatmsg("buff4",buff[4]);
	power_min=power[0];

	if(typeYC==5 || typeYC==6){       //YC1000
		for(i=1;i<3;i++)
		{
			if(power[i]<power_min)
				power_min=power[i];
		}
	}
	else if(typeYC==7){  //YC600
		power_min=power[0];
	}
//	printdecmsg("power_min",power_min);
	return power_min;
}

int read_ccflag(void)		//读取逆流标志，出现逆流返回1，没有逆流返回0, 读取失败返回-1
{
	int i = 3;
	int res;
	float data[40];

	do {
		if((res = read_min_phase_power(data)))
			break;
		usleep(200000);
	}while(--i);

	return res? (res < 0):-1;
}

int meterset(char data[20][8])              //设置 返回1设置成功 返回0设置失败
{
	int res, i, j, sendtimes=0, recvbytes,k;
	struct timeval timeout;
	unsigned char sendbuff[512]={'\0'};
	int meter_ctset[255]={'\0'};
	char recvdata[256]={'\0'};
	unsigned short check=0x00;

//	tcflush(cc_fd,TCIOFLUSH);
//	sleep(1);                 //发送指令前，先清空缓冲区
	sendbuff[0]  = 0xFB;
	sendbuff[1]  = 0xFB;
	sendbuff[2]  = 0x39;
	sendbuff[3]  = 0xE1;
	sendbuff[4]  = 0x00;
	sendbuff[5]  = 0x01;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	meter_ctset[0]  = atoi((data[6]));
	sendbuff[8]  = meter_ctset[0]/256;
	sendbuff[9]  = meter_ctset[0];
	meter_ctset[1]  = atoi((data[8]));
	sendbuff[10] = meter_ctset[1]/256;
	sendbuff[11] = meter_ctset[1];
	meter_ctset[2]  = atoi((data[0]));
	sendbuff[12] = meter_ctset[2];

	meter_ctset[3]  = atoi((data[10]));
	sendbuff[13] = meter_ctset[3]/256;
	sendbuff[14] = meter_ctset[3];
	meter_ctset[4]  = atoi((data[12]));
	sendbuff[15] = meter_ctset[4]/256;
	sendbuff[16] = meter_ctset[4];
	meter_ctset[5]  = atoi((data[2]));
	sendbuff[17] = meter_ctset[5];

	meter_ctset[6]  = atoi((data[14]));
	sendbuff[18] = meter_ctset[6]/256;
	sendbuff[19] = meter_ctset[6];
	meter_ctset[7]  = atoi((data[16]));
	sendbuff[20] = meter_ctset[7]/256;
	sendbuff[21] = meter_ctset[7];
	meter_ctset[8]  = atoi((data[4]));
	sendbuff[22] = meter_ctset[8];

	meter_ctset[9]  = atoi((data[7]));
	sendbuff[23] = meter_ctset[9]/256;
	sendbuff[24] = meter_ctset[9];
	meter_ctset[10]  = atoi((data[9]));
	sendbuff[25] = meter_ctset[10]/256;
	sendbuff[26] = meter_ctset[10];
	meter_ctset[11] = atoi((data[1]));
	sendbuff[27] = meter_ctset[11] ;

	meter_ctset[12]  = atoi((data[11]));
	sendbuff[28] = meter_ctset[12]/256;
	sendbuff[29] = meter_ctset[12];
	meter_ctset[13]  = atoi((data[13]));
	sendbuff[30] = meter_ctset[13]/256;
	sendbuff[31] = meter_ctset[13];
	meter_ctset[14]  = atoi((data[3]));
	sendbuff[32] = meter_ctset[14];

	meter_ctset[15]  = atoi((data[15]));
	sendbuff[33] = meter_ctset[15]/256;
	sendbuff[34] = meter_ctset[15];
	meter_ctset[16]  = atoi((data[17]));
	sendbuff[35] = meter_ctset[16]/256;
	sendbuff[36] = meter_ctset[16];
	meter_ctset[17]  = atoi((data[5]));
	sendbuff[37] =meter_ctset[17];
	for(k=38;k<58;k++){
		sendbuff[k] = 0x00;
	}
	for(i=2;i<58;i++){
		check += sendbuff[i];
		}
	sendbuff[58]  = check >> 8;
	sendbuff[59]  = check;
	sendbuff[60] = 0xFE;
	sendbuff[61] = 0xFE;

	printhexmsg("meterset",sendbuff,62);
	for(j=0; j<3; j++){              //发送3次
		write(cc_fd, sendbuff,62);
		recvbytes=get_reply_from_ccfd(recvdata);
		printhexmsg("recvdata", recvdata,recvbytes);
		if ((12 == recvbytes)
			&& (0xFB == recvdata[0])&& (0xFB == recvdata[1])&&(0x07 == recvdata[2])
			&&(0xD1 == recvdata[3])&&(0xFE == recvdata[11]))
				return 1;
		else
			continue;
	}
	return 0;
}

int get_ctset(void)
{
	int i,j;
	FILE *fp;
	char buff[255]={'\0'}; //设置值
	fp = fopen("/tmp/tmp_sensorset.conf","r");
		if(fp){                       //meter参数设置
			while(!feof(fp)){
				memset(buff, '\0', sizeof(buff));
				fgets(buff,sizeof(buff),fp);
				if(!strlen(buff)){
					break;
				}
				else{
//					printmsg("Enter ctset\n");
					for(i=0;i<18;i++){
						memset(data_ctset[i],'0',sizeof(char));
					}
				sscanf(buff,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s",
						data_ctset[0],data_ctset[1],data_ctset[2],data_ctset[3],data_ctset[4],data_ctset[5],
						data_ctset[6],data_ctset[7],data_ctset[8],data_ctset[9],data_ctset[10],data_ctset[11],
						data_ctset[12],data_ctset[13],data_ctset[14],data_ctset[15],data_ctset[16],data_ctset[17]
						);
				}
			}
			fclose(fp);
		}
		return 0;
}

int set_cc_ct(void)
{
	int i,j;
	FILE *fpset;
	char buff[255]={'\0'}; //设置值
	char data_set[20][8]={'\0'};   //CT设置值
	fpset = fopen("/tmp/sensor_set.conf","r");
		if(fpset){                       //meter参数设置
			while(!feof(fpset)){
//				flag=0;
				memset(buff, '\0', sizeof(buff));
				fgets(buff,sizeof(buff),fpset);
				if(!strlen(buff)){
					break;
				}
				else{
					printmsg("Enter ctset\n");
					for(i=0;i<18;i++){
						memset(data_set[i],'0',sizeof(char));
					}
				sscanf(buff,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s",
						data_set[0],data_set[1],data_set[2],data_set[3],data_set[4],data_set[5],
						data_set[6],data_set[7],data_set[8],data_set[9],data_set[10],data_set[11],
						data_set[12],data_set[13],data_set[14],data_set[15],data_set[16],data_set[17]
						);
				meterset(data_set);
				}
			}
			fclose(fpset);
			fpset = fopen("/tmp/sensor_set.conf","w");
			fclose(fpset);
		}
		return 0;
}

int meter_record(float *data,char ct_set[20][8])
{
	FILE *fp;
	fp = fopen("/tmp/sensor.txt","w");     //轮询数据
	if(fp){
						fprintf(fp,"freq_ch1=%.1f\n",data[0]);
						fprintf(fp,"volA_ch1=%.1f\n",data[1]);
						fprintf(fp,"volB_ch1=%.1f\n",data[2]);
						fprintf(fp,"volC_ch1=%.1f\n",data[3]);
						fprintf(fp,"powerA_ch1=%.0f\n",data[4]);
						fprintf(fp,"powerB_ch1=%.0f\n",data[5]);
						fprintf(fp,"powerC_ch1=%.0f\n",data[6]);
						fprintf(fp,"varA_ch1=%.1f\n",data[7]);
						fprintf(fp,"varB_ch1=%.1f\n",data[8]);
						fprintf(fp,"varC_ch1=%.1f\n",data[9]);
						fprintf(fp,"energyA_ch1=%.1f\n",data[10]);
						fprintf(fp,"energyB_ch1=%.1f\n",data[11]);
						fprintf(fp,"energyC_ch1=%.1f\n",data[12]);
						fprintf(fp,"type_ct1A=%s\n",ct_set[0]);
						fprintf(fp,"type_ct1B=%s\n",ct_set[2]);
						fprintf(fp,"type_ct1C=%s\n",ct_set[4]);
						fprintf(fp,"currentA_ch1=%s\n",ct_set[6]);
						fprintf(fp,"voltageA_ch1=%s\n",ct_set[8]);
						fprintf(fp,"currentB_ch1=%s\n",ct_set[10]);
						fprintf(fp,"voltageB_ch1=%s\n",ct_set[12]);
						fprintf(fp,"currentC_ch1=%s\n",ct_set[14]);
						fprintf(fp,"voltageC_ch1=%s\n",ct_set[16]);

	fprintf(fp,"freq_ch2=%.1f\n",data[13]);
	fprintf(fp,"volA_ch2=%.1f\n",data[14]);
	fprintf(fp,"volB_ch2=%.1f\n",data[15]);
	fprintf(fp,"volC_ch2=%.1f\n",data[16]);
	fprintf(fp,"powerA_ch2=%.0f\n",data[17]);
	fprintf(fp,"powerB_ch2=%.0f\n",data[18]);
	fprintf(fp,"powerC_ch2=%.0f\n",data[19]);
	fprintf(fp,"varA_ch2=%.1f\n",data[20]);
	fprintf(fp,"varB_ch2=%.1f\n",data[21]);
	fprintf(fp,"varC_ch2=%.1f\n",data[22]);
	fprintf(fp,"energyA_ch2=%.1f\n",data[23]);
	fprintf(fp,"energyB_ch2=%.1f\n",data[24]);
	fprintf(fp,"energyC_ch2=%.1f\n",data[25]);
	fprintf(fp,"type_ct2A=%s\n",ct_set[1]);
	fprintf(fp,"type_ct2B=%s\n",ct_set[3]);
	fprintf(fp,"type_ct2C=%s\n",ct_set[5]);
	fprintf(fp,"currentA_ch2=%s\n",ct_set[7]);
	fprintf(fp,"voltageA_ch2=%s\n",ct_set[9]);
	fprintf(fp,"currentB_ch2=%s\n",ct_set[11]);
	fprintf(fp,"voltageB_ch2=%s\n",ct_set[13]);
	fprintf(fp,"currentC_ch2=%s\n",ct_set[15]);
	fprintf(fp,"voltageC_ch2=%s\n",ct_set[17]);

		fclose(fp);
	  }
	return 0;
}

int turn_off_ct(void)     //断开继电器
{
	int relay;
	relay=open("/dev/relay",O_WRONLY);
	ioctl(relay,10);
	write(relay,"1",1);
	ioctl(relay,20);
	write(relay,"1",1);
	close(relay);
	return 0;
}

int turn_on_ct(void)    //闭合继电器
{
	int relay;
	relay=open("/dev/relay",O_WRONLY);
	ioctl(relay,10);
	write(relay,"0",1);
	ioctl(relay,20);
	write(relay,"0",1);
	close(relay);
	return 0;
}

int dalay_second(int dalay_time)
{
	while(dalay_time--)
	{
		if(1 == read_ccflag()){
			zb_shutdown_broadcast();		//防止广播关闭时有些逆变器没有收到，再发送一次
			sleep(1);
			break;
		}
		sleep(1);
	}
	return 0;
}
/*出现逆流后，先关闭所有逆变器，然后逐台开启，直到最小正功率低于300W，判断出现逆流，就关掉全部逆变器，否则继续轮询*/
int do_cc(struct inverter_info_t *firstinverter)
{
	int i;
	int j=0;
	float data[40];
	FILE *fp;
	char flag='0';
	struct inverter_info_t *curinverter = firstinverter;
	//YC1000 400W typeYC=0，YC600 600W typeYC=200
//	printmsg("Enter into cc\n");
	fp=fopen("/etc/yuneng/countercurrent.conf","r");
	if(fp){
		flag=fgetc(fp);
		fclose(fp);
		if(flag == '0'){
		return 0;
		}
	}
	else
		return 0;

	typeYC=curinverter->model;
	turn_on_ct();                   //检查逆流前，先闭合继电器
	if(1 == read_ccflag()){			//出现逆流
		printmsg("Enter ccflag\n");
		zb_shutdown_broadcast();	//关闭所有逆变器
		turn_off_ct();				//断开接触器
		sleep(1);
		if(1 == read_ccflag()){
			zb_shutdown_broadcast();		//防止广播关闭时有些逆变器没有收到，再发送一次
			sleep(1);
		}
		turn_on_ct();                   //检查逆流前，先闭合继电器
		if (0 == read_ccflag()){
//				sleep(150);                //等待逆变器母线电压下降
			dalay_second(150);
			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++){
				//开启一台逆变器
				zb_boot_single(curinverter);
				printdecmsg("i", i+1);
//				sleep(90);  //等待逆变器跑满功率
				j=0;
				while(DELAYTIME-j)                      //等待时间内继续检测逆流
				{
					if(1 == read_ccflag()){
						zb_shutdown_broadcast();		//防止广播关闭时有些逆变器没有收到，再发送一次
						dalay_second(150);
						break;
					}
					sleep(1);
					j++;
				}
				if(read_min_phase_power(data) <= 600){
					//电表功率小于等于600W，停止开启逆变器，并判断是否逆流
					if(1 == read_ccflag()){
							//关闭所有逆变器
							zb_shutdown_broadcast();
							sleep(1);
							zb_shutdown_broadcast();
							dalay_second(150);
					}
					//没有逆流，退出循环
					break;
				}
			}
		}
		return 1;
	}
	return 0;
}

/*只有出现过逆流现象才做：读取当前是否出现逆流，如果没有出现逆流，逐台开启逆变器，直到最小正功率低于400W，判断出现逆流，就关掉全部逆变器，否则继续轮询*/
int do_uncc(struct inverter_info_t *firstinverter)
{
	int i,power_min;
	int j=0;
	float data[40];
	FILE *fp;
	char flag='0';
	struct inverter_info_t *curinverter = firstinverter;
	//YC1000 300W，YC600 600W
//	printmsg("Enter into uncc\n");

	fp=fopen("/etc/yuneng/countercurrent.conf","r");
	if(fp){
		flag=fgetc(fp);
		fclose(fp);
		if(flag == '0'){
		return 0;
		}
	}
	else
		return 0;


	turn_on_ct();                   //先闭合继电器
	power_min=read_min_phase_power(data);
	get_ctset();
		meter_record(data,data_ctset);
	//当电表功率小于等于300W，则不做任何操作
	if(power_min <= 600){
		return 0;
	}
//	sleep(150);                //等待逆变器母线电压下降
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++){
		//开启一台逆变器
		if (0x30 == curinverter->status_web[18]) {
			//本来就是开启状态，跳过该台，判断下一台
			continue;
		}
		zb_boot_single(curinverter);
//		sleep(90);
		j=0;
		while(DELAYTIME-j)                      //等待时间内继续检测逆流
		{
			if(1 == read_ccflag()){
				zb_shutdown_broadcast();		//防止广播关闭时有些逆变器没有收到，再发送一次
				dalay_second(150);
				break;
			}
			sleep(1);
			j++;
		}
		if(read_min_phase_power(data) <= 600){
			//电表功率小于等于400W，停止开启逆变器，并判断是否逆流
			if(1 == read_ccflag()){
					//关闭所有逆变器
					zb_shutdown_broadcast();
					turn_off_ct();				//断开接触器
					dalay_second(150);
			}
			//没有逆流，退出循环
			break;
		}
	}
	return 0;
}
