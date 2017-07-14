1、搜索wifi环境

2、对比前十行开头是否为
				(!strncmp(ss1,"ESSID",5))&&
				(!strncmp(ss2,"Quality",7))&&
				(!strncmp(ss3,"Signal",6))&&
				(!strncmp(ss4,"level",5))&&
				(!strncmp(ss5,"dBm",3))&&
				(!strncmp(ss6,"Noise",5))&&
				(!strncmp(ss7,"level",5))&&
				(!strncmp(ss8,"dBm",3))&&
				(!strncmp(ss9,"Encryption",10))&&
				(!strncmp(ss10,"key",3))

不是的话wifi模块初始化
