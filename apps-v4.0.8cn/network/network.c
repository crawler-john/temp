#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * 检测无线网卡wlan0状态
 * 返回 0：无线未连接(默认)
 * 返回 1：无线已连接
 */
int check_wlan0_connection(void)
{
	FILE *fp;
	char flag = '0';

    fp = fopen("/sys/class/net/wlan0/carrier", "r");
    if (fp) {
    	if ((flag = fgetc(fp)) == EOF)
    		flag = '0';
    	fclose(fp);
    }
    return flag - '0';
}

/*
 * 检测有线网卡eth0状态
 * 返回 0：网线未连接
 * 返回 1：网线已连接(默认)
 */
int check_eth0_connection(void)
{
	FILE *fp;
	char flag = '1';

    fp = fopen("/sys/class/net/eth0/carrier", "r");
    if (fp) {
    	if ((flag = fgetc(fp)) == EOF)
    		flag = '1';
    	fclose(fp);
    }
    return flag - '0';
}

/*
 * 获取IP地址取得方式
 * 返回 0:获取静态IP
 * 返回 1:获取动态IP(默认)
 */
int get_dhcp_flag()
{
	FILE *fp;
	char flag = '1';

    fp = fopen("/etc/yuneng/dhcp.conf", "r");
    if (fp) {
    	if ((flag = fgetc(fp)) == EOF)
    		flag = '1';
    	fclose(fp);
    }
    return flag - '0';
}

/*
 * 获取有线IP地址
 */
int get_ip_of_eth0()
{
	//静态IP不再自动获取IP
	if (get_dhcp_flag() == 0)
		return 0;

	//网线未连接时设置默认IP
	if (check_eth0_connection() == 0) {
		system("ifconfig eth0 192.168.131.228");
		return 0;
	}

	//重新获取有线IP地址
	if (system("/sbin/udhcpc -nq -i eth0") != 0) {
		system("ifconfig eth0 192.168.131.228");
	}
	return 0;
}

/*
 * 获取无线IP地址
 */
int get_ip_of_wlan0()
{
	//无线未连接时重启wpa_supplicant
	if (check_wlan0_connection() == 0) {
		system("killall wpa_supplicant");
		if (!access("/etc/yuneng/wifi_signal_info.conf", F_OK))
			system("/usr/sbin/wpa_supplicant -Dwext -i wlan0 -c /etc/yuneng/wifi_signal_info.conf -B"); //启动STA模式
	}

	//重新获取无线IP地址
	system("/sbin/udhcpc -nq -i wlan0");
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	char wifi_val[5]={'\0'};
	int flag_wifi=1;

	fp = fopen("/etc/yuneng/wifi.conf","r");
    if (fp) {
    	fgets(wifi_val,5,fp);
    	fclose(fp);
      	if(!strncmp(wifi_val,"0",1))
       		flag_wifi=0;
    }
    while(1) {
    	get_ip_of_eth0();
    	if(flag_wifi==1)
    	get_ip_of_wlan0();
    	sleep(300);
    }
    return 0;
}
