#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#include "sqlite3.h"
#include "variation.h"
#include "debug.h"
#include "database.h"
extern unsigned char ccuid[7];
extern int plcmodem;

crc_check(char *data)
{
	int i;
	unsigned short crc=0x0000ffff;
	char da;
	unsigned int crc_ba[16]={0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
							0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef};

	for(i=19;i<25;i++)
	{
		da=((crc/256))/16;				//printf("\nda=%d  ",da);
		crc<<=4;						//printf("crc=%x  ",crc);
		crc^=crc_ba[da^(data[i]/16)];	//printf("crc=%x  ",crc);
		da=(crc/256)/16;				//printf("da=%d  ",da);
		crc<<=4;						//printf("crc=%x  ",crc);
		crc^=crc_ba[da^(data[i]&0x0f)];//printf("crc=%x  ",crc);
		//printf("crc16=%x\n",crc);
	}
	crc=0xffff-crc;
	data[25]=crc/256;
	data[26]=crc%256;
}

int check_change_no(char *buff,char *data)
{
	if((data[0]==0xfb)&&(data[1]==0xfb)&&(data[2]==0x11)&&(data[3]==0x00)&&(data[4]==0x0f)&&
		(data[5]==buff[5])&&(data[6]==buff[6])&&(data[7]==buff[7])&&(data[8]==buff[8])&&(data[9]==buff[9])&&(data[10]==buff[10])&&
		(data[11]==buff[19])&&(data[12]==buff[20])&&(data[13]==buff[21])&&(data[14]==buff[22])&&(data[15]==buff[23])&&(data[16]==buff[24])&&
		(data[17]==0x1d)&&(data[18]==0x01)&&(data[22]==0xfe)&&(data[23]==0xfe))
	{
		if(data[19]==0x00)printmsg("Did not open uid-configuration");
		else if(data[19]==0x01)printmsg("Error CRC-16");
		else printmsg("Unknow Error");
	}
	return -1;
}

int check_change_ok(char *buff,char *data)
{
	int i;int check=0;
	if((data[0]==0xfb)&&(data[1]==0xfb)&&(data[2]==0x11)&&(data[3]==0x00)&&(data[4]==0x15)&&
		(data[5]==buff[5])&&(data[6]==buff[6])&&(data[7]==buff[7])&&(data[8]==buff[8])&&(data[9]==buff[9])&&(data[10]==buff[10])&&
		(data[11]==buff[19])&&(data[12]==buff[20])&&(data[13]==buff[21])&&(data[14]==buff[22])&&(data[15]==buff[23])&&(data[16]==buff[24])&&
		(data[17]==0x1d)&&(data[18]==0x07)&&(data[28]==0xfe)&&(data[29]==0xfe)&&
		(data[19]==buff[11])&&(data[20]==buff[12])&&(data[21]==buff[13])&&(data[22]==buff[14])&&(data[23]==buff[15])&&(data[24]==buff[16]))
	{
		for(i=2;i<26;i++)
			check=check+data[i];
		if((data[26]==check/256)&&(data[27]==check%256))
		{
			printmsg("UID-config SUCCESS");
			return 1;
		}
	}
	return -1;
}

int uidstep1()
{
	int i,res=0;
	int check=0;
	fd_set rd;
	struct timeval timeout;
	char readbuff[255];
	char sendbuff[22]={0xFB,0xFB,0x1b,0x00,0x0d,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xaa,0xaa,0xfe,0xfe};
	for(i=5;i<11;i++)
		sendbuff[i]=ccuid[i-5];
	printhexmsg("ccuid",ccuid,6);
	for(i=2;i<18;i++)
	{
		check=check+sendbuff[i];
	}
	sendbuff[18]=check/256;
	sendbuff[19]=check%256;

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 22);printhexmsg("send",sendbuff,22);
	
	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		printmsg("Recv step1 failure");
		return -1;
	}
	else
	{
		res = 0;
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);printhexmsg("reply",readbuff,res);
		if(res!=22)
			return -1;		
		for(i=0;i<22;i++)
		{
			if(readbuff[i]!=sendbuff[i])
				return -1;
		}
		return 1;
	}
}

int uidstep2(struct inverter_info_t *firstinverter,char *badid,int *count)
{//badid[0]='A';
	struct inverter_info_t *curinverter = firstinverter;
	int i,res=0;
	int check=0;
	int setflag=1;
	int tmpcount=*count;
	fd_set rd;
	struct timeval timeout;
	char uid[6]={'\0'};
	char readbuff[255];
	char sendbuff[23]={0xFB,0xFB,0x1A,0x00,0x0e,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x02,0xaa,0xaa,0xfe,0xfe};
	//strncpy(&sendbuff[5],ccuid,6);
	for(i=5;i<11;i++)
		sendbuff[i]=ccuid[i-5];
	for(i=2;i<19;i++)
	{
		check=check+sendbuff[i];
	}
	sendbuff[19]=check/256;
	sendbuff[20]=check%256;

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 23);
	printhexmsg("send",sendbuff,23);
	while(1){
		res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
		if(res <= 0){
			if(*count==tmpcount){
			printmsg("Recv step2 failure");
			return -1;}
			else return 1;
		}
		else
		{
			check=0;
			res = 0;
			memset(readbuff, '\0', 255);
			res = read(plcmodem, readbuff, 255);
			printhexmsg("reply",readbuff,res);
			res+=26;

			int num=-1;
			while(1){
				setflag=1;
				num++;
				res-=26;
				check=0;
				if((res/26)==0)
					break;
				if((readbuff[num*26]==0xfb)&&(readbuff[num*26+1]==0xfb)&&(readbuff[num*26+2]==0x11)&&(readbuff[num*26+3]==0x00)&&
					(readbuff[num*26+4]==0x11)&&(readbuff[num*26+17]==0x09)&&(readbuff[num*26+24]==0xfe)&&(readbuff[num*26+25]==0xfe))
				{
					for(i=2;i<22;i++)
						check=check+readbuff[num*26+i];
					if((readbuff[num*26+22]==(check/256))&&(readbuff[num*26+23]==(check%256))){;}
					else continue;
				}
				else continue;
				curinverter = firstinverter;
				for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++){		//每个逆变器比对tnuid
//					if(!strncmp(curinverter->tnuid,&readbuff[num*26+11],6))
					if((curinverter->tnuid[0]==readbuff[num*26+11])&&(curinverter->tnuid[1]==readbuff[num*26+12])
							&&(curinverter->tnuid[2]==readbuff[num*26+13])&&(curinverter->tnuid[3]==readbuff[num*26+14])
							&&(curinverter->tnuid[4]==readbuff[num*26+15])&&(curinverter->tnuid[5]==readbuff[num*26+16]))

						{setflag=0;break;}
					curinverter++;
				}
				for(i=0; i<(*count); i++){		//每个逆变器比对badid
//					if(!strncmp(&badid[6*i],&readbuff[num*26+11],6))
					if((badid[6*i+0]==readbuff[num*26+11])&&(badid[6*i+1]==readbuff[num*26+12])
							&&(badid[6*i+2]==readbuff[num*26+13])&&(badid[6*i+3]==readbuff[num*26+14])
							&&(badid[6*i+4]==readbuff[num*26+15])&&(badid[6*i+5]==readbuff[num*26+16]))

						{setflag=0;break;}
				}
				if(setflag){
					(*count)++;
					for(i=0;i<6;i++)
						badid[(*count)*6-6+i]=readbuff[num*26+11+i];
					printhexmsg("**bad_id**",badid,6*(*count));printhexmsg("readbuff11",&readbuff[num*26+11],6*(*count));
				}
			}
		}
	}
	return 0;
}

int uidstep3(char *badid)
{printhexmsg("badid",badid,6);
	int i,res=0;
	int check=0;
	fd_set rd;
	struct timeval timeout;
	char readbuff[255];
	char sendbuff[22]={0xFB,0xFB,0x11,0x00,0x0d,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0xaa,0xaa,0xfe,0xfe};
//	strncpy(&sendbuff[5],ccuid,6);
	for(i=5;i<11;i++)
		sendbuff[i]=ccuid[i-5];
	sendbuff[11]=badid[0];sendbuff[12]=badid[1];sendbuff[13]=badid[2];
	sendbuff[14]=badid[3];sendbuff[15]=badid[4];sendbuff[16]=badid[5];//strncpy(&sendbuff[11],badid,6);
	for(i=2;i<18;i++)
	{
		check=check+sendbuff[i];
	}
	sendbuff[18]=check/256;
	sendbuff[19]=check%256;

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 22);
	printhexmsg("send",sendbuff,22);

	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		printmsg("Recv step3 failure");
		return -1;
	}
	else
	{
		res = 0;
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		printhexmsg("reply",readbuff,res);
		if(res!=22)
			return -1;		
		for(i=0;i<22;i++)
		{
			if(readbuff[i]!=sendbuff[i])
				return -1;
		}
		return 1;
	}
}

int uidstep4(char *badid,char *goodid)
{
	int i,res=0;
	int check=0;
	fd_set rd;
	struct timeval timeout;
	char readbuff[255];
	char sendbuff[31]={0xFB,0xFB,0x11,0x00,0x16,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0x1d,0x08,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xbb,0xbb,0xaa,0xaa,0xfe,0xfe};
//	strncpy(&sendbuff[5],ccuid,6);
	for(i=5;i<11;i++)
		sendbuff[i]=ccuid[i-5];
	for(i=0;i<6;i++){
	sendbuff[i+11]=badid[i];
	sendbuff[i+19]=goodid[i];}
	crc_check(sendbuff);			//获得crc校验码给sendbuff【25】【26】
//	char buff[8]={'\0'};buff[0]=sendbuff[19];buff[1]=sendbuff[20];buff[2]=sendbuff[21];buff[3]=sendbuff[22];
//	buff[4]=sendbuff[23];buff[5]=sendbuff[24];
//	add_crc(buff,6);printhexmsg("buff",buff,8);
//sendbuff[25]=0xd1;sendbuff[26]=0x4f;
	for(i=2;i<27;i++)
	{
		check=check+sendbuff[i];
	}
	sendbuff[27]=check/256;
	sendbuff[28]=check%256;

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 31);
	printhexmsg("send",sendbuff,31);
	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		printmsg("Recv step4 failure");
		return -1;
	}
	else
	{
		res = 0;
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		printhexmsg("reply",readbuff,res);
		if(res==30)
		{
			if(1==check_change_ok(sendbuff,readbuff))
				return 1;
		}
		else if(res==24)
		{
			check_change_no(sendbuff,readbuff);		//为了打印错误信息
			return -1;
		}
		else return -1;
	}
	return -1;
}

int uidstep5(char *goodid)
{
	int i,res=0;
	int check=0;
	fd_set rd;
	struct timeval timeout;
	char readbuff[255];
	char sendbuff[22]={0xFB,0xFB,0x1b,0x00,0x0d,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0x00,0x00,0x00,0x00,0x00,0x00,0x1e,0xaa,0xaa,0xfe,0xfe};
//	strncpy(&sendbuff[5],ccuid,6);
	for(i=5;i<11;i++)
		sendbuff[i]=ccuid[i-5];
//	strncpy(&sendbuff[11],goodid,6);
	for(i=11;i<17;i++)
		sendbuff[i]=goodid[i-11];
	for(i=2;i<18;i++)
	{
		check=check+sendbuff[i];
	}
	sendbuff[18]=check/256;
	sendbuff[19]=check%256;

	FD_ZERO(&rd);
	FD_SET(plcmodem, &rd);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	write(plcmodem, sendbuff, 22);
	
	res = select(plcmodem+1, &rd, NULL , NULL, &timeout);
	if(res <= 0){
		printmsg("Recv step5 failure");
		return -1;
	}
	else
	{
		res = 0;
		memset(readbuff, '\0', 255);
		res = read(plcmodem, readbuff, 255);
		if(res!=22)
			return -1;		
		for(i=0;i<22;i++)
		{
			if(readbuff[i]!=sendbuff[i])
				return -1;
		}
		return 1;
	}
}

int reduce_uid(struct inverter_info_t *firstinverter,char *inverterid,char *badinverterid)
{	print2msg("inveretrid",inverterid);
	char badid[6],goodid[6]={'\0'};
	int i,res,faile;
	for(i=0;i<6;i++)
	{
		badid[i] =((badinverterid[2*i]-0x30)<<4)+(badinverterid[2*i+1]-0x30);
		goodid[i]=((inverterid[2*i]-0x30)<<4)+(inverterid[2*i+1]-0x30);
	}
	printhexmsg("inverter_id",goodid,6);printhexmsg("wrong_id",badid,6);
	//step1
	for(i=0;i<3;i++)
	{
		printmsg("start step1");
		faile=1;
		if(1==uidstep1())
		{
			printmsg("step1 ok");
			faile=0;
			break;
		}
	}
	if(faile==1)
		return 0;
	
	//step2:特殊，会收到很多条回复
//	for(i=0;i<3;i++)
//	{
//		printmsg("start step2");
//		faile=1;
//		if(1==uidstep2(firstinverter,badinverterid))
//		{
//			printmsg("step2 ok");
//			faile=0;
//			break;
//		}
//	}
//	if(faile==1)
//		return 0;

	//step3
	for(i=0;i<3;i++)
	{
		printmsg("start step3");
		faile=1;
		if(1==uidstep3(badid))
		{
			printmsg("step3 ok");
			faile=0;
			break;
		}
	}
	if(faile==1)
		return 0;
	
	//step4
	for(i=0;i<3;i++)
	{
		printmsg("start step4");
		faile=1;
		if(1==uidstep4(badid,goodid))
		{
			printmsg("step4 ok");
			faile=0;
			break;
		}
	}
	if(faile==1)
		return 0;

	//step5
	for(i=0;i<3;i++)
	{
		printmsg("start step5");
		faile=1;
		if(1==uidstep5(goodid))
		{
			printmsg("step5 ok");
			faile=0;
			break;
		}
	}
	return 1;

}

int back3501uid(struct inverter_info_t *firstinverter)
{
	sleep(3);
	sqlite3 *db;
	sqlite3_open("/home/database.db", &db);
	if(NULL== db)
		exit(1);
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow,ncolumn;
	char protect_result[65535] = {'\0'};
	char inverter_result[64];
	char ecu_id[16];
	int count=0;
	char inverter_id[16];
	char wrong_id[16];
	FILE *fp;

	sprintf(sql,"SELECT DISTINCT correct_id FROM need_id WHERE set_flag=1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	printdecmsg("nrow",nrow);
	if(nrow>0)
	{
		strcpy(protect_result, "APS1300000A151AAA0");

		fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
		if(fp)
		{
			fgets(ecu_id, 13, fp);
			fclose(fp);
		}
		strcat(protect_result, ecu_id);					//ECU的ID
		strcat(protect_result, "0000");					//逆变器个数
		strcat(protect_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
		strcat(protect_result, "END");					//固定格式

		while(1)
		{
			memset(sql,'\0',1024);
			sprintf(sql,"SELECT correct_id,wrongid FROM need_id WHERE set_flag=1");
			sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
			if(nrow==0)
				break;
			strcpy(inverter_id,azResult[2]);
			strcpy(wrong_id,azResult[3]);
			memset(sql,'\0',1024);
			sprintf(sql,"UPDATE need_id  SET set_flag=0 WHERE correct_id='%s'",inverter_id);
			sqlite3_exec_3times(db, sql);
			if(1==reduce_uid(firstinverter,inverter_id,wrong_id))
			{
				strcat(protect_result, inverter_id);
				strcat(protect_result, "END");
				count++;
			}		
		}
		char temp[4];char tmp[5];
		sprintf(temp,"%04d",count);
		protect_result[30]=temp[0];
		protect_result[31]=temp[1];
		protect_result[32]=temp[2];
		protect_result[33]=temp[3];
		sprintf(tmp,"%05d",strlen(protect_result));
		strncpy(&protect_result[5],tmp,5);

		print2msg("res_A151",protect_result);
		//protect_result写入数据库


	memset(sql,'\0',1024);
	sprintf(sql,"DELETE FROM process_result WHERE item=151");
	sqlite3_exec_3times(db, sql);
	memset(sql,'\0',1024);
	sprintf(sql,"INSERT INTO process_result (item,result,flag) VALUES(151,'%s',1)",protect_result);
	sqlite3_exec_3times(db, sql);
	sqlite3_close(db);
	}
	return 0;
}

int protocal149(char *badid,int count)
{
	FILE *fp;
	int i;
	char ecu_id[13];char tep[5]={'\0'};char tmp[5];char uid[13]={'\0'};
	sprintf(tep,"%04d",count);
	char protocal_result[1024]={'\0'};
	strcpy(protocal_result,"APS1300000");
	strcat(protocal_result,"A149AAA0");
	fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}
	strcat(protocal_result,ecu_id);
	strcat(protocal_result,tep);
	strcat(protocal_result,"00000000000000");
	for(i=0;i<count;i++)
	{
		strcat(protocal_result,"END");
		sprintf(uid,"%02x%02x%02x%02x%02x%02x",badid[i*6],badid[i*6+1],badid[i*6+2],badid[i*6+3],badid[i*6+4],badid[i*6+5]);
		strcat(protocal_result,uid);
	}
	strcat(protocal_result,"END");
	if(count==0)
	{
		//strcat(protocal_result,"AAAAAAAAAAAAEND");
	}
	sprintf(tmp,"%05d",strlen(protocal_result));
	strncpy(&protocal_result[5],tmp,5);
	sqlite3 *db;
	sqlite3_open("/home/database.db",&db);
	if(NULL== db)
		exit(1);

	char sql[1024]={'\0'};
	sprintf(sql,"DELETE FROM process_result WHERE item=149");
	sqlite3_exec_3times(db, sql);
	memset(sql,'\0',1024);
	sprintf(sql,"INSERT INTO process_result (item,result,flag) VALUES(149,'%s',1)",protocal_result);
	sqlite3_exec_3times(db, sql);
	sqlite3_close(db);
	return 0;
}

int get_unnormal_id(struct inverter_info_t *firstinverter,int curcnt,int maxcnt)
{
	printmsg("Step2**Step2***Step2***Step2***Step2***Step2***Step2***Step2***Step2****");
	sleep(3);
	printmsg("Step2**Step2***Step2***Step2***Step2***Step2***Step2***Step2***Step2****");
	FILE *fp;
	int count=0;

	char  badid[1024]={'\0'};
	char flaga='0';
	fp = fopen("/etc/yuneng/get_bad_id.conf", "r");
	if(fp){
		flaga = fgetc(fp);
		fclose(fp);
	}
	fp = fopen("/etc/yuneng/get_bad_id.conf", "w");
	if(fp){
		fputs("0",fp);
		fclose(fp);
	}

	if('1' !=flaga){
		return 0;
	}
	if(1 != uidstep1())
	{
		printmsg("get_unnormal_id step1 faile");
		return 0;
	}

	int i,a;
	for(i=0;i<3;i++){
		if(1==uidstep2(firstinverter,badid,&count)){a=9;
			if(maxcnt==count+curcnt)
				break;
		}
	}
	if(a!=9){
		protocal149("AAAAAAAAAAAA",0);printhexmsg("bad_id0",badid,6);
	}
	else {
		protocal149(badid,count);printhexmsg("bad_id2",badid,6*count);
	}
	return 0;
}
