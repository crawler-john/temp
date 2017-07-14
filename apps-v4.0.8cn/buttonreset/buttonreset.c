#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

//#define DEBUGINFO 0

int main(void)
{
	int fd, lcdfd,ret,pressedflag=0;	//pressedflag:按下为1，没安下为0
	char buff[2]={'0'};
	struct timeval tpstart,tpend;
	tpstart.tv_sec=0;
	char lcdbuff[20] = "Reset successfully!";

	fd=open("/dev/buttonreset",O_RDONLY);
	while(1)
	{
		ret=read(fd,buff,1);
		if('1' == buff[0]){
			if(1==pressedflag){
				gettimeofday(&tpend,NULL);
				if(((tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_usec-tpstart.tv_usec)/1000000)>1.0){
					printf("Pressed!\n");
//					system("killall monitor.exe");
//					system("killall main.exe");
//					system("killall clientmonitor");
//					system("killall client");
//					system("killall idwriter");
//					system("killall ntpmanager");
//					system("killall resmonitor");
//					system("killall udhcpc");
//					system("killall updatemanager");
//					system("killall autoupdate");
//					system("abc");			//必须加上，不否不工作
//					system("rm -rf /home/local_web");
//					system("cp -rf /home/backup/local_web /home/");
//					printf("point1\n");
//					system("cp -rf /home/backup/applications/* /home/applications/");
					printf("point1\n");
					system("cp /home/backup/yuneng/* /etc/yuneng/");
					printf("point1\n");
//					system("/home/applications/manager &");
					printf("point1\n");
					lcdfd = open("/dev/lcd", O_WRONLY);
					ioctl(lcdfd,0x01,0);
					ioctl(lcdfd,0x80,0);
					write(lcdfd,lcdbuff,strlen(lcdbuff));
					close(lcdfd);
					sleep(1);
					system("reboot");
				}
				pressedflag = 0;
				tpstart.tv_sec=0;
			}
		}
		else{
			pressedflag=1;
			if(tpstart.tv_sec==0)
				gettimeofday(&tpstart,NULL);
		}

#ifdef DEBUGINFO
		if(buff[0] == '0')
			printf("Reset button has pressed!");
		printf("buff=%c,ret=%d\n",buff[0],ret);
#endif
		usleep(500000);
	}

	close(fd);

	return 0;
}
