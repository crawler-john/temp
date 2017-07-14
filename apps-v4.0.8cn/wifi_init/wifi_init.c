/*
 * wifi_init.c
 * WiFi开机初始化程序
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void check_ap_conf();

int main(void)
{
	/* Enable AP Mode */
	check_ap_conf();
	system("/sbin/ifconfig wlan1 172.30.1.1"); //设置IP
	sleep(1);
	system("/usr/sbin/hostapd /etc/yuneng/wifi_ap_info.conf -B"); //建立AP
	sleep(1);
	system("/usr/sbin/udhcpd /etc/yuneng/udhcpd_wlan1.conf &"); //分配IP地址
	sleep(1);

	/* Connect WLAN */
	if (!access("/etc/yuneng/wifi_signal_info.conf", F_OK)) { //若已配置连接的路由器，则启动路由器连接功能
		system("/usr/sbin/wpa_supplicant -Dwext -i wlan0 -c /etc/yuneng/wifi_signal_info.conf -B"); //启动STA模式
		system("/sbin/udhcpc -nq -i wlan0"); //获取IP地址
	}
}

//获取ECU_id最后4位
int get_ecuid()
{
	FILE *fp;
	char ecuid[13] = {'\0'};

	if (NULL == (fp = fopen("/etc/yuneng/ecuid.conf", "r"))) {
		perror("ecuid.conf");
		return -1;
	}
	fgets(ecuid, 13, fp);
	fclose(fp);
	return atoi(ecuid+8);
}

//检查AP模式的配置文件
void check_ap_conf()
{
	FILE *fp;
	int serialNum = 0;
	int apReset = 0;
	char buff[32];

	//AP配置：wifi_ap_info.conf
	if (NULL != (fp = fopen("/etc/yuneng/wifi_ap_info.conf", "r"))) {
		fgets(buff, sizeof(buff), fp);
		fclose(fp);
		if (!strncmp(buff, "#SSID=ECU-WIFI_0000", 19))
			apReset = 1;
	} else {
		apReset = 1;
	}
	if (apReset) {
		fp = fopen("/etc/yuneng/wifi_ap_info.conf", "w");
		if (fp) {
			serialNum = get_ecuid()%10000;
			fprintf(fp,
					"#SSID=ECU-WIFI_%04d\n"
					"#method=0\n"
					"#channel=0\n"
					"interface=wlan1\n"
					"driver=ar6000\n"
					"ssid=ECU-WIFI_%04d\n"
					"channel=6\n"
					"ignore_broadcast_ssid=0\n",
					serialNum, serialNum);
			fclose(fp);
		}
	}

	//IP分配：udhcpd_wlan1.conf
	if (access("/etc/yuneng/udhcpd_wlan1.conf", F_OK)) {
		fp = fopen("/etc/yuneng/udhcpd_wlan1.conf", "w");
		if (fp) {
			fputs("start 172.30.1.100\n"
				  "end 172.30.1.254\n"
				  "interface wlan1\n"
				  "max_leases 99\n"
				  "leases_file /tmp/udhcpd_wlan1.leases\n"
				  "opt dns 172.30.1.1\n"
				  "option subnet 255.255.255.0\n"
				  "opt router 172.30.1.1", fp);
			fclose(fp);
		}
	}
}
