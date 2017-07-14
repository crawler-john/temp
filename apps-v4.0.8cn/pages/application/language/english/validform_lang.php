<?php
/* 
 * 该文件保存表单验证的显示语言（英文版） 
 */
	
/* 参数配置 */
	//交流保护参数
	$lang['validform_under_voltage_fast'] = "Please enter an integer between 0-999";//外围电压下限
	$lang['validform_over_voltage_fast'] = "Please enter an integer between 0-999";//外围电压上限
	$lang['validform_under_voltage_slow'] = "Please enter an integer between 0-999";//内围电压下限
	$lang['validform_over_voltage_slow'] = "Please enter an integer between 0-999";//内围电压上限
	$lang['validform_under_frequency_fast'] = "Please enter a decimal number between 0-99(precision to 1st decimal)";//外围频率下限
	$lang['validform_over_frequency_fast'] = "Please enter a decimal number between 0-99(precision to 1st decimal)";//外围频率上限
	$lang['validform_under_frequency_slow'] = "Please enter a decimal number between 0-99(precision to 1st decimal)";//内围频率下限
	$lang['validform_over_frequency_slow'] = "Please enter a decimal number between 0-99(precision to 1st decimal)";//内围频率上限
	$lang['validform_voltage_triptime_fast'] = "Please enter a decimal number between 0-999(precision to 2nd decimal)";//外围过欠压延迟保护时间
	$lang['validform_voltage_triptime_slow'] = "Please enter a decimal number between 0-999(precision to 2nd decimal)";//内围过欠压延迟保护时间
	$lang['validform_frequency_triptime_fast'] = "Please enter a decimal number between 0-999(precision to 2nd decimal)";//外围过欠频延迟保护时间
	$lang['validform_frequency_triptime_slow'] = "Please enter a decimal number between 0-999(precision to 2nd decimal)";//内围过欠频延迟保护时间
	$lang['validform_grid_recovery_time'] = "Please enter an integer between 0-99999";//并网恢复时间
	$lang['validform_regulated_dc_working_point'] = "Please enter a decimal number between 0-99(precision to 1st decimal)";
	$lang['validform_under_voltage_stage_2'] = "Please enter an integer between 0-999";
	$lang['validform_voltage_3_clearance_time'] = "Please enter a decimal number between 0-999(precision to 2nd decimal)";
	$lang['validform_start_time'] = "Please enter an integer between 0-99999";
	
/* 系统管理 */
	//时间管理
	$lang['validform_datetime'] = "Format: YYYY/MM/DD hh:mm:ss";
	$lang['validform_null_datetime'] = "Please enter Date Time";
	
	//网络管理
	$lang['validform_ip_address'] = "The IP address format error";
	$lang['validform_subnet_mask'] = "The Subnet mask format error";
	$lang['validform_default_gateway'] = "The Default gateway format error";
	$lang['validform_preferred_dns_server'] = "The Preferred DNS server format error";
	$lang['validform_alternate_dns_server'] = "The Alternate DNS server format error";
	$lang['validform_null_ip_address'] = "Please enter IP address";
	$lang['validform_null_subnet_mask'] = "Please enter Subnet mask";
	$lang['validform_null_default_gateway'] = "Please enter Default gateway";
	$lang['validform_null_preferred_dns_server'] = "Please enter Preferred DNS server";
	
	//用户管理
	$lang['validform_username'] = "Please enter 4-18 bit arbitrary character";
	$lang['validform_old_password'] = "Please enter 5-18 bit arbitrary character";
	$lang['validform_new_password'] = "Please enter 5-18 bit arbitrary character";
	$lang['validform_confirm_password'] = "The passwords you entered do not match";
	$lang['validform_null_username'] = "Please enter the username";
	$lang['validform_null_password'] = "Please enter the password";
	$lang['validform_null_old_password'] = "Please enter the old password";
	$lang['validform_null_new_password'] = "Please enter the new password";
	$lang['validform_null_confirm_password'] = "Please confirm the new password";
	
	//无线网络管理
	  //AP
	$lang['validform_ap_ssid'] = "Please enter 4-18 bit arbitrary character";
	$lang['validform_ap_password_wep'] = "Please enter 5 bit or 13 bit digital";
	$lang['validform_ap_password_wpa'] = "Please enter 8-18 bit arbitrary character";
	$lang['validform_null_ap_ssid'] = "Please enter signal name";
	$lang['validform_null_ap_password'] = "Please enter password";
      //STA	
	  
/* 隐藏功能 */	
	//服务器地址与端口
	$lang['validform_domain'] = "The Domain format error";
	$lang['validform_ip_address'] = "The IP address format error";
	$lang['validform_port'] = "The Port format error";
	$lang['validform_null_domain'] = "Please enter Domain";
	$lang['validform_null_ip_address'] = "Please enter IP address";
	$lang['validform_null_port'] = "Please enter Port";
	$lang['validform_null_port1'] = "Please enter Port1";
	$lang['validform_null_port2'] = "Please enter Port2";
	
	//串口
	$lang['validform_ecu_address'] = "Please enter an integer between 0-128";
	$lang['validform_null_ecu_address'] = "Please enter ECU address";
	
	//加密
	$lang['validform_null_psw'] = "Please inter password";
	$lang['validform_psw'] = "Please inter 8 bit digital or letter";
	$lang['validform_timeout'] = "Please inter an integer between 10-600";
	
	//心跳周期时间设置
	$lang['validform_heart'] = "Please inter an integer between 60-300";
?>
