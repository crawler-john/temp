#define MAXINVERTERCOUNT 999	//最大的逆变器数
#define RECORDLENGTH 100	//子记录的长度
#define RECORDTAIL 100		//大记录的结尾，包括发电量、时间等信息
#define TNUIDLENGTH 7		//逆变器3501ID的长度
#define INVERTERIDLEN 13	//逆变器ID的长度
#define STATUSLENGTH 3		//逆变器当前状态的长度
#define DVLENGTH 5		//直流电压的长度，EMA通信协议中使用
#define DILENGTH 3		//直流电流的长度，EMA通信协议中使用
#define POWERLENGTH 5		//功率长度，EMA通信协议中使用
#define FREQUENCYLENGTH 5	//电网频率长度，EMA通信协议中使用
#define TEMPERATURELENGTH 3	//温度长度，EMA通信协议中使用
#define GRIDVOLTLENGTH 3	//电网电压长度，EMA通信协议中使用
#define CURGENLENGTH 6		//当前一轮发电量的长度，EMA通信协议中使用
#define SYSTEMPOWERLEN 10	//系统功率，EMA通信协议中使用
#define CURSYSGENLEN 10		//系统当前发电量，EMA通信协议中使用
#define LIFETIMEGENLEN 10	//历史发电量，EMA通信协议中使用

struct inverter_info_t{
	char tnuid[7];		//逆变器3501ID，逆变器ID的BCD编码
	char inverterid[13];	//逆变器的ID

	float predv;			//前一轮直流电压
	float predi;			//前一轮直流电流
	int preop;			//前一轮输出功率
	float prenp;			//前一轮电网频率
	int prenv;			//前一轮电网电压
	int preit;			//前一轮机内温度
	float predvb;			//前一轮B路直流电压
	float predib;			//前一轮B路直流电流
	int preopb;			//前一轮B路输出功率

	int lendtimes;			//向最近数据借用次数，最多3次
	
	float dv;			//直流电压
	float di;			//直流电流
	int op;			//输出功率
	float np;			//电网频率
	int nv;			//电网电压
	int it;			//机内温度

	int flagyc500;			//1，为YC500

	/********B路数据***********/
	float dvb;			//直流电压
	float dib;			//直流电流
	int opb;			//输出功率
	//float npb;			//电网频率
	//int nvb;			//电网电压
	//int itb;			//机内温度

	/*******预设值AB共用*******/
	int prevl1;		//读取的预设值， 电压下限1
	int prevu1;		//读取的预设值， 电压上限1
	int prevl2;		//读取的预设值， 电压下限2
	int prevu2;		//读取的预设值， 电压上限2
	int prefl;			//读取的预设值， 频率下限
	int prefu;			//读取的预设值， 频率上限
	int prert;			//读取的预设值， 恢复时间

	float curgeneration;		//逆变器当前一轮的电量
	float curgenerationb;		//B路当前一轮的电量
	float precurgeneration;		//逆变器当前一轮的电量
	float precurgenerationb;		//B路当前一轮的电量

	float preaccgen;		//A路上一轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float preaccgenb;		//B路上一轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float curaccgen;		//A路本轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float curaccgenb;		//B路本轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	int preacctime;		//上一轮返回的累计时间（逆变器启动后时间不停累计，直到其重启）
	int curacctime;		//本轮返回的累计时间（逆变器启动后时间不停累计，直到其重启）
	int duration;		//本论读取的累计时间减去上一轮读取的累计时间差值

	//char prestatus[3];		//前一轮逆变器状态
	char status_web[24];		//存入ECU本地数据库的状态，用于本地页面显示
	char status_ema[64];
	char status_send_flag;
	char status[12];		//逆变器状态
	char statusb[12];		//B路状态

	char prestatus[12];		//前一轮逆变器状态
	char prestatusb[12];		//前一轮B路状态

	char flag;		//正确获取到数据的标志位，‘0’：当前没有获取到数据；‘1’：当前已获取到数据
	char curflag;		//B路当前一轮有数据的标志

	int configflag;	//逆变器3501配置标志, '0':不用配置； ‘1’：需要配置
	char presetdataflag;		//不匹配标志,预设置的值与逆变器返回的预设值不匹配为‘A’，没返回为'B',匹配则为‘0’
	char lastreporttime[3];		//逆变器最近一次上报时间
	//char lastreporttimeb[3];	//B路逆变器最近一次上报时间

	int lasttime;			//上一次上传数据的时间
	int thistime;			//本次上传数据的时间
	//int lasttimeb;			//B路上一次上传数据的时间
	//int thistimeb;			//B路本次上传数据的时间
	int turnoffautorpidflag;			//禁止逆变器自动上报标志，‘1’为禁止，‘0’为不禁止
	char processed_protect_flag;	//

	char last_turn_on_off_flag;
	char turn_on_off_changed_flag;
	char last_gfdi_flag;
	char gfdi_changed_flag;

	char inverter_with_13_parameters;	//2表示逆变器有13项参数，1表示逆变器有5项参数，0表示不清楚逆变器有几项参数，13项参数的逆变器不用处理逆变器读取参数失败无法启动的问题
	int fill_up_data_flag;							//逆变器是否有补数据功能的标志位，1为有功能,2为没有功能，默认0为没有响应或者第一次

	char processed_paras_changed_flag;		//ECU每天对参数突变的逆变器处理，处理后标志为1，没有处理标志为0；
};

