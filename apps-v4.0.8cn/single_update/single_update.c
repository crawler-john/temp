#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

//#define DEBUG
//#define DEBUGLOG

void debug(char *msg)
{
#ifdef DEBUG
	printf("%s\n", msg);
#endif
}

void err_exit(char *err_msg, int err_code)
{
#ifdef DEBUG
	printf("%s\n", err_msg);
#endif
	exit(err_code);
}

void printmsg(char *msg)		//打印字符串
{
#ifdef DEBUG
	printf("autoupdate: %s!\n", msg);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/autoupdate.log", "a");
	if(fp)
	{
		fprintf(fp, "autoupdate: %s!\n", msg);
		fclose(fp);
	}
#endif
}

void print2msg(char *msg1, char *msg2)		//打印字符串
{
#ifdef DEBUG
	printf("autoupdate: %s, %s!\n", msg1, msg2);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/autoupdate.log", "a");
	if(fp)
	{
		fprintf(fp, "autoupdate: %s, %s!\n", msg1, msg2);
		fclose(fp);
	}
#endif
}

void printdecmsg(char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	printf("autoupdate: %s: %d!\n", msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/autoupdate.log", "a");
	if(fp)
	{
		fprintf(fp, "autoupdate: %s: %d!\n", msg, data);
		fclose(fp);
	}
#endif
}

void printhexmsg(char *msg, char *data, int size)		//打印十六进制数据
{
#ifdef DEBUG
	int i;

	printf("autoupdate: %s: ", msg);
	for(i=0; i<size; i++)
		printf("%X, ", data[i]);
	printf("\n");
#endif
#ifdef DEBUGLOG
	FILE *fp;
	int j;

	fp = fopen("/home/autoupdate.log", "a");
	if(fp)
	{
		fprintf(fp, "autoupdate: %s: ", msg);
		for(j=0; j<size; j++)
			fprintf(fp, "%X, ", data[j]);
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif
}

void get_version(char * ver)
{
	FILE *fp;

	fp = fopen("/etc/yuneng/version.conf", "r");
	if(fp)
	{
		fgets(ver, 512, fp);
		if('\n' == ver[strlen(ver)-1])
			ver[strlen(ver)-1] = '\0';
		fclose(fp);
	}
	else
		err_exit("version.conf is not exist", -1);
}

void get_addr_port(char *addr, char *port)
{
	FILE *fp;
	char buff[1024];
	char domain[1024] = {'\0'};
	char ip[32] = {'\0'};
	struct hostent * host;

	fp = fopen("/etc/yuneng/updatecenter.conf", "r");
	if(fp)
	{
		while(1)
		{
			memset(buff, '\0', sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if(!strlen(buff))
				break;
			if(!strncmp(buff, "Domain", 6))
			{
				strcpy(domain, &buff[7]);
				if('\n' == domain[strlen(domain)-1])
					domain[strlen(domain)-1] = '\0';
			}
			if(!strncmp(buff, "IP", 2))
			{
				strcpy(ip, &buff[3]);
				if('\n' == ip[strlen(ip)-1])
					ip[strlen(ip)-1] = '\0';
			}
			if(!strncmp(buff, "Port", 4))
			{
				strcpy(port, &buff[5]);
				if('\n' == port[strlen(port)-1])
					port[strlen(port)-1] = '\0';
			}
		}
		fclose(fp);
	}
	print2msg("Domain", domain);
	print2msg("IP", ip);
	print2msg("Port", port);

	if(!strlen(domain))
		strcpy(domain, "ecu.apsema.com");
	if(!strlen(ip))
		strcpy(ip, "60.190.131.190");
	if(!strlen(port))
		strcpy(port, "21");

	host = gethostbyname(domain);
	if(NULL != host)
	{
		memset(ip, '\0', sizeof(ip));
		inet_ntop(AF_INET, *host->h_addr_list, ip, 32);
		print2msg("Resolved IP", ip);
	}
	strcpy(addr, ip);
}

int main(int argc, char *argv[])
{
	char version[512] = {'\0'};
	char ipaddr[512] = {'\0'};
	char port[512] = {'\0'};
	char cmd[1024] = {'\0'};
	char buff[512] = {'\0'};
	char fname[512] = {'\0'};
	char ecuid[512] = {'\0'};
	char *str;
	int i=0, res;
	FILE *fp;

	while(1)
	{
		remove("/tmp/update_single");
		remove("/tmp/update_single.tar.bz2");

		fp = fopen("/etc/yuneng/ecuid.conf", "r");
		if(fp)
		{
			fgets(ecuid, 13, fp);
			fclose(fp);
		}
		else
			printmsg("ecuid.conf is not exist");
		chdir("/tmp");
		get_version(version);
		get_addr_port(ipaddr, port);
		sprintf(cmd, "ftpget -u ecuupdate -p aps -P %s %s /tmp/update_single.tar.bz2 /single_ecu/%s/update_single.tar.bz2", port, ipaddr, ecuid);
		printmsg(cmd);
		i = 0;

		do
		{
			if(i++>0)
				break;//err_exit("Download updateinfo.txt more then 3 times", -1);
			res = system(cmd);
			if(res)
				sleep(60);
		}while(res);

		if(!res)
		{
			if(!system("tar xjvf update_single.tar.bz2"))
				system("/tmp/update_single/assist");

			remove("/tmp/update_single");
			remove("/tmp/update_single.tar.bz2");
		}

		sleep(86400);
	}

	return 0;
}
