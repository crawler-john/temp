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
	char temp[256]={'\0'};

	fp = fopen("/etc/yuneng/version.conf", "r");
	if(fp)
	{
		fgets(temp, 512, fp);
		if('\n' == temp[strlen(temp)-1])
			temp[strlen(temp)-1] = '\0';
		fclose(fp);

		strcpy(ver, temp);
	}
	else
		err_exit("version.conf is not exist", -1);

	fp = fopen("/etc/yuneng/area.conf", "r");
	if(fp)
	{
		memset(temp, '\0', sizeof(temp));
		fgets(temp, 512, fp);
		if('\n' == temp[strlen(temp)-1])
			temp[strlen(temp)-1] = '\0';
		fclose(fp);

		strcat(ver, temp);
	}
	else
		err_exit("area.conf is not exist", -1);
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
	char *str;
	int i=0, res;
	FILE *fp;

	while(1)
	{
		chdir("/tmp");
		get_version(version);
		get_addr_port(ipaddr, port);
		sprintf(cmd, "ftpget -u ecuupdate -p aps -P %s %s updateinfo.txt /%s/updateinfo.txt", port, ipaddr, version);
		printmsg(cmd);
		i = 0;

		do
		{
			if(i++>2)
				break;//err_exit("Download updateinfo.txt more then 3 times", -1);
			res = system(cmd);
			if(res)
				sleep(60);
		}while(res);

		//printf("res: %d\n", res);
		fp = fopen("/tmp/updateinfo.txt", "r");
		if(fp)
		{
			while(1)
			{
				memset(buff, '\0', sizeof(buff));
				fgets(buff, sizeof(buff), fp);
				if(!strncmp(buff, "download ", 9))
				{
					str = strchr(buff, ' ');
					while(' ' == *str)
						str++;
					if((0x0a == str[strlen(str)-1])||('\n' == str[strlen(str)-1]))
						str[strlen(str)-1] = '\0';
					if((0x0d == str[strlen(str)-1])||('\n' == str[strlen(str)-1]))
						str[strlen(str)-1] = '\0';
					strcpy(fname, str);
					sprintf(cmd, "ftpget -u ecuupdate -p aps -P %s %s %s /%s/%s", port, ipaddr, fname, version, fname);
					printmsg(cmd);
					res = system(cmd);
					//printf("res: %d\n", res);
					chmod(fname, 0755);
				}
				if(!strncmp(buff, "command ", 8))
				{
					str = strchr(buff, ' ');
					while(' ' == *str)
						str++;
					if((0x0a == str[strlen(str)-1])||('\n' == str[strlen(str)-1]))
						str[strlen(str)-1] = '\0';
					if((0x0d == str[strlen(str)-1])||('\n' == str[strlen(str)-1]))
						str[strlen(str)-1] = '\0';
					strcpy(cmd, str);
					printmsg(cmd);
					system(cmd);
				}
				if(!strncmp(buff, "exit", 4))
					exit(0);
				if(!strlen(buff))
					break;
			}
			fclose(fp);
		}
		else
			printmsg("updateinfo.txt is not exist");

		sleep(172800);
	}

	return 0;
}
