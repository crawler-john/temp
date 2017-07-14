#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>

int set_eth0_mac()
{
    FILE *fp;
    char buff[18]={'\0'};
    char cmd[256]={'\0'};

    fp = fopen("/etc/yuneng/ecu_eth0_mac.conf","r");
    if(fp){
		fgets(buff,18,fp);
		fclose(fp);
		printf("%s,%d\n",buff,strlen(buff));

		system("ifconfig eth0 down");
		sprintf(cmd, "ifconfig eth0 hw ether %s", buff);
		system(cmd);
		sleep(1);
		system("ifconfig eth0 up");
    }

    return 0;
}

int read_wlan0_mac()
{
	FILE *fp;
    int i, sockfd;
    struct ifreq ifr;
    char mac[32] = {'\0'};

	fp = fopen("/etc/yuneng/ecu_wlan0_mac.conf","r");
	if(fp){
		fgets(mac, sizeof(mac), fp);
		fclose(fp);
	}
	if(0 == strlen(mac)){
		printf("ddd\n");
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd == -1){
			return -1;
	    }

		bzero(&ifr, sizeof(ifr));
		strncpy(ifr.ifr_name, "wlan0", sizeof(ifr.ifr_name));

		if((ioctl(sockfd,SIOCGIFHWADDR,&ifr)) < 0){
			return -1;
	    }

		fp = fopen("/etc/yuneng/ecu_wlan0_mac.conf","w");
		if(fp){
			fprintf(fp, "%02X:%02X:%02X:%02X:%02X:%02X", (unsigned char)ifr.ifr_hwaddr.sa_data[0], (unsigned char)ifr.ifr_hwaddr.sa_data[1], (unsigned char)ifr.ifr_hwaddr.sa_data[2], (unsigned char)ifr.ifr_hwaddr.sa_data[3], (unsigned char)ifr.ifr_hwaddr.sa_data[4], (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
			fclose(fp);
		}
	}

	return 0;
}

int main(void)
{
	set_eth0_mac();
	read_wlan0_mac();

	return 0;
}
