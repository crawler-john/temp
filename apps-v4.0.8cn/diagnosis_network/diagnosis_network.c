#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

//#define DEBUG
//#define DEBUGLOG

void printmsg(char *msg)		//打印字符串
{
#ifdef DEBUG
	printf("Diagnosis_network: %s!\n", msg);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/Diagnosis_network.log", "a");
	if(fp)
	{
		fprintf(fp, "Diagnosis_network: %s!\n", msg);
		fclose(fp);
	}
#endif
}

void print2msg(char *msg1, char *msg2)		//打印字符串
{
#ifdef DEBUG
	printf("Diagnosis_network: %s, %s!\n", msg1, msg2);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/Diagnosis_network.log", "a");
	if(fp)
	{
		fprintf(fp, "Diagnosis_network: %s, %s!\n", msg1, msg2);
		fclose(fp);
	}
#endif
}

void printdecmsg(char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	printf("Diagnosis_network: %s: %d!\n", msg, data);
#endif
#ifdef DEBUGLOG
	FILE *fp;

	fp = fopen("/home/Diagnosis_network.log", "a");
	if(fp)
	{
		fprintf(fp, "Diagnosis_network: %s: %d!\n", msg, data);
		fclose(fp);
	}
#endif
}

void printhexmsg(char *msg, char *data, int size)		//打印十六进制数据
{
#ifdef DEBUG
	int i;

	printf("Diagnosis_network: %s: ", msg);
	for(i=0; i<size; i++)
		printf("%X, ", data[i]);
	printf("\n");
#endif
#ifdef DEBUGLOG
	FILE *fp;
	int j;

	fp = fopen("/home/Diagnosis_network.log", "a");
	if(fp)
	{
		fprintf(fp, "Diagnosis_network: %s: ", msg);
		for(j=0; j<size; j++)
			fprintf(fp, "%X, ", data[j]);
		fprintf(fp, "\n");
		fclose(fp);
	}
#endif
}

//域名解析，成功说明网络正常
int domain_resolution()
{
	char domain[1024]={'\0'};		//EMA的域名
	char buff[1024] = {'\0'};
	char ip[20] = {'\0'};
	struct hostent * host;
	FILE *fp;

	fp = fopen("/etc/yuneng/datacenter.conf", "r");
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
				break;
			}
		}
		fclose(fp);
	}

	host = gethostbyname(domain);
	if(NULL == host)
	{
		printmsg("Resolve domain failure");
		return -1;
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		inet_ntop(AF_INET, *host->h_addr_list, ip, 32);
		printmsg(ip);
		return 0;
	}
}

int main(int argc, char *argv[])
{
	while(1){
		if(-1 == domain_resolution()){
			system("killall network.exe");
			sleep(1);
			system("/home/applications/network.exe");
		}

		sleep(300);
	}

    return 0;
}
