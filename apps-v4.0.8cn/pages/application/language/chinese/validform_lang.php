<?php
/* 
 * 该文件保存表单验证的显示语言（中文版） 
 */
	
/* 参数配置 */	
	//交流保护参数
	$lang['validform_under_voltage_fast'] = "请输入0-999之间的整数";//外围电压下限
	$lang['validform_over_voltage_fast'] = "请输入0-999之间的整数";//外围电压上限
	$lang['validform_under_voltage_slow'] = "请输入0-999之间的整数";//内围电压下限
	$lang['validform_over_voltage_slow'] = "请输入0-999之间的整数";//内围电压上限
	$lang['validform_under_frequency_fast'] = "请输入0-99之间的小数(精确到小数点后一位)";//外围频率下限
	$lang['validform_over_frequency_fast'] = "请输入0-99之间的小数(精确到小数点后一位)";//外围频率上限
	$lang['validform_under_frequency_slow'] = "请输入0-99之间的小数(精确到小数点后一位)";//内围频率下限
	$lang['validform_over_frequency_slow'] = "请输入0-99之间的小数(精确到小数点后一位)";//内围频率上限
	$lang['validform_voltage_triptime_fast'] = "请输入0-999之间的小数(精确到小数点后两位)";//外围过欠压延迟保护时间
	$lang['validform_voltage_triptime_slow'] = "请输入0-999之间的小数(精确到小数点后两位)";//内围过欠压延迟保护时间
	$lang['validform_frequency_triptime_fast'] = "请输入0-999之间的小数(精确到小数点后两位)";//外围过欠频延迟保护时间
	$lang['validform_frequency_triptime_slow'] = "请输入0-999之间的小数(精确到小数点后两位)";//内围过欠频延迟保护时间
	$lang['validform_grid_recovery_time'] = "请输入0-99999之间的整数";//并网恢复时间
	$lang['validform_regulated_dc_working_point'] = "请输入0-99之间的小数(精确到小数点后一位)";
	$lang['validform_under_voltage_stage_2'] = "请输入0-999之间的整数";
	$lang['validform_voltage_3_clearance_time'] = "请输入0-999之间的小数(精确到小数点后两位)";
	$lang['validform_start_time'] = "请输入0-99999之间的整数";

/* 系统管理 */
	//时间管理
	$lang['validform_datetime'] = "日期时间格式 YYYY/MM/DD hh:mm:ss";
	$lang['validform_null_datetime'] = "请输入日期时间";
	
	//网络管理
	$lang['validform_ip_address'] = "请输入格式正确的 IP地址";
	$lang['validform_subnet_mask'] = "请输入格式正确的 子网掩码";
	$lang['validform_default_gateway'] = "请输入格式正确的 默认网关";
	$lang['validform_preferred_dns_server'] = "请输入格式正确的 首选DNS服务器";
	$lang['validform_alternate_dns_server'] = "请输入格式正确的 备用DNS服务器";
	$lang['validform_null_ip_address'] = "请输入IP地址";
	$lang['validform_null_subnet_mask'] = "请输入子网掩码";
	$lang['validform_null_default_gateway'] = "请输入默认网关";
	$lang['validform_null_preferred_dns_server'] = "请输入首选DNS服务器";	
	
	//用户管理
	$lang['validform_username'] = "请输入4-18位任意字符";
	$lang['validform_old_password'] = "请输入5-18位任意字符";
	$lang['validform_new_password'] = "请输入5-18位任意字符";
	$lang['validform_confirm_password'] = "两次输入的密码不一致";
	$lang['validform_null_username'] = "请输入用户名";
	$lang['validform_null_password'] = "请输入密码";
	$lang['validform_null_old_password'] = "请输入旧密码";
	$lang['validform_null_new_password'] = "请输入新密码";
	$lang['validform_null_confirm_password'] = "请确认新密码";
	
	//无线网络管理
	  //AP
	$lang['validform_ap_ssid'] = "请输入4-18位任意字符";
	$lang['validform_ap_password_wep'] = "请输入5位或13位数字";
	$lang['validform_ap_password_wpa'] = "请输入8-18位任意字符";
	$lang['validform_null_ap_ssid'] = "请输入信号名称";
	$lang['validform_null_ap_password'] = "请输入密码";
      //STA	
	  
/* 隐藏功能 */	
	//服务器地址与端口
	$lang['validform_domain'] = "请输入正确格式的域名";
	$lang['validform_ip_address'] = "请输入正确格式的IP地址";
	$lang['validform_port'] = "请输入正确格式的端口";
	$lang['validform_null_domain'] = "请输入域名";
	$lang['validform_null_ip_address'] = "请输入IP地址";
	$lang['validform_null_port'] = "请输入端口";
	$lang['validform_null_port1'] = "请输入端口1";
	$lang['validform_null_port2'] = "请输入端口2";
	
	//串口
	$lang['validform_ecu_address'] = "请输入0-128的整数";
	$lang['validform_null_ecu_address'] = "请输入ECU地址";
	
	//加密
	$lang['validform_null_psw'] = "请输入加密密码";
	$lang['validform_psw'] = "请输入8位数字或字母";
	$lang['validform_timeout'] = "请输入10-600的整数";	
	
	//心跳周期时间设置
	$lang['validform_heart'] = "请输入60-300的整数";
	
?>
