/*
 * 程序用于处理逆变器启动时读取保护参数失败的问题。
 * Created by zhyf
 * Modify on 2013/12/24
 * Version 1.1
 */


#include <stdio.h>
#include <stdlib.h>

#include "variation.h"
#include "debug.h"

extern int plcmodem;
extern unsigned char ccuid[7];

char reference_protect_parameters[20] = {'\0'};
int get_parameters_flag=0;

//每次轮训逆变器数据时，判断累计时间和功率，如果累计时间小于30分钟时、功率小于10瓦，说明逆变器刚启动且有可能异常，标记设为1；如果功率大于10，说明没有异常，标志设为0。
//在解析数据函数里调用。
/*int determine_process_protect_parameters(struct inverter_info_t *inverter)
{
	if((inverter->op < 10) && (inverter->opb < 10))
	{
		if(inverter->curacctime < 1800)
		{
			inverter->process_protect_parameters = 1;
		}
	}
	else
	{
		inverter->process_protect_parameters = 0;
	}

	return 0;
}*/

//读取参照的保护参数，判断每一个逆变器的功率，如果大于10，说明可以正常工作，把它的保护参数读取出来，给工作异常的逆变器设置。在main函数里调用
int read_reference_protect_parameters(struct inverter_info_t *firstinverter)
{
	int i, j, res;
	struct inverter_info_t *curinverter = firstinverter;

	if(!get_parameters_flag)	//参考参数默认初始化时是0，判断如果是0，说明没有读取过，如果不是0，说明读取过
	{
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)	//都到第一个符合条件的逆变器参数作为参考参数，就直接返回
		{
			if(('1' == curinverter->flag) && (2 != curinverter->inverter_with_13_parameters)){		//如果ECU没有数据不处理
				if((curinverter->op >= 5) || (curinverter->opb >= 5))
				{
					for(j=0; j<3; j++)	//一个逆变器最多读取3次，失败则读取下一个逆变器
					{
						memset(reference_protect_parameters, '\0', sizeof(reference_protect_parameters));
						res = askpresetdatacmd(curinverter, reference_protect_parameters, 0x11);
						if(1 == res){
							get_parameters_flag = 1;
							return 0;
						}
						if(2 == res)
							break;
					}
				}
			}
		}
	}

	return -1;
}

//逆变器的参数和参照参数对比，相同返回0，不同返回-1
int compare_protect_parameters(char *protect_parameters)
{
	int i;

	for(i=4; i<12; i++)		//有一个参数不同就直接返回-1
	{
		if(protect_parameters[i] != reference_protect_parameters[i])
			return -1;
	}

	return 0;
}

//设置逆变器参数，并且读取，比对后，相同返回0，不同返回-1。
int set_protect_parameters(struct inverter_info_t *inverter)
{
	char protect_parameters[20] = {'\0'};

	sendpresetdata(plcmodem, ccuid, inverter->tnuid, reference_protect_parameters);		//设置逆变器参数
	askpresetdatacmd(inverter, protect_parameters, 0x11);		//读取逆变器参数
	if(!compare_protect_parameters(protect_parameters))									//比对逆变器参数
		return 0;
	else
		return -1;
}

//对每一个逆变器处理。在main函数里调用
int process_protect_parameters(struct inverter_info_t *firstinverter)
{
	int i, j, k;
	char protect_parameters[20] = {'\0'};
	struct inverter_info_t *curinverter = firstinverter;

	if(get_parameters_flag)		//有参考参数时才做处理
	{
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->inverterid)); i++, curinverter++)
		{
			//printf("inverter: %s, process_protect_flag: %d, curinverter->curacctime: %d\n", curinverter->inverterid, curinverter->processed_protect_flag, curinverter->curacctime);
			if((!curinverter->processed_protect_flag) && ('1' == curinverter->flag) && (2 != curinverter->inverter_with_13_parameters)){		//如果ECU没有数据不处理
				if((curinverter->op < 5) && (curinverter->opb < 5))
				{
					for(j=0; j<1; j++)		//一个逆变器最多读取3次
					{
						if(1 == askpresetdatacmd(curinverter, protect_parameters, 0x11))
						{
							if(compare_protect_parameters(protect_parameters))		//如果参数与参照参数不同则设置，相同就不处理标记为0
							{
								for(k=0; k<1; k++)		//最多设置3次，有一次成功标记为0，后面不再处理，如果3此失败，等下一个大的轮询
								{
									if(!set_protect_parameters(curinverter))
									{
										curinverter->processed_protect_flag = 1;
										break;
									}
								}
							}
							else
							{
								curinverter->processed_protect_flag = 1;
							}
							break;
						}
					}
				}
				else
					curinverter->processed_protect_flag = 1;
			}
		}
	}
}
