

typedef struct esensor_info_t{
	char id[13];		//esensor的ID
	unsigned short shortaddr;			//Zigbee的短地址
	char tnuid[8];				//逆变器3501ID（逆变器ID的BCD编码）
	int model;					//机型：1是YC250CN,2是YC250NA，3是YC500CN，4是YC500NA，5是YC900CN，6是YC900NA
	int version;				//软件版本号
	int dataflag;				//1表示读到当前数据；0表示读取数据失败
	int signalstrength;			//逆变器Zigbee信号强度
	int bindflag;				//逆变器绑定短地址标志，1表示绑定，0表示未绑定
	
	float dv;					//直流电压
	float di;					//直流电流
	float op;					//输出功率
	float curgeneration;		//esensor当次上单累积电量
	float energy;				//esensor历史电量
	float max_dv;
	float max_di;
	int curacctime;				//当次上电累计时间（逆变器启动后时间不停累计，直到其重启）
	int time;					//历史累计时间


	char last_report_time[16];	//发送给EMA时的日期和时间，格式：年月日时分秒
	int no_getdata_num;					//连续没有获取到逆变器数据的次数
	int disconnect_times;				//一天中没有与逆变器通信上的所有次数 ZK
	int zigbee_version;					//zigbee版本号ZK
	char processed_protect_flag;	//

	char last_turn_on_off_flag;
	char turn_on_off_changed_flag;
	char last_gfdi_flag;
	char gfdi_changed_flag;
}esensor_info;



