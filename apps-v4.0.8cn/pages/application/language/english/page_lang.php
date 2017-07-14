<?php
/* 
 * 该文件保存ECU页面的显示语言（英文版） 
 */
    /* 通用 */
	$lang['title'] = "Altenergy Power Control Software";
	$lang['title_ecu'] = "ENERGY COMMUNICATION UNIT";
	
	$lang['ecu_id'] = "ECU ID";
	$lang['device_id'] = "Inverter ID";
	
	$lang['button_save'] = "Save";
	$lang['button_update'] = "Update";
	$lang['button_ok'] = "OK";
	$lang['button_cancel'] = "Cancel";
	$lang['button_reset'] = "Reset";
	$lang['button_query'] = "Query";
	$lang['button_refresh'] = "Refresh";
	$lang['button_read'] = "Read";
	$lang['button_save_all'] = "Save All Inverters";
	$lang['button_read_all'] = "Read All Inverters";
	$lang['button_more'] = "More";
	
	$lang['message_success'] = "Success";
	$lang['message_warning'] = "Warning";
	$lang['message_failed'] = "Failed";	

	$lang['ecu_reboot_title'] = "ECU is rebooting ...";
	$lang['ecu_reboot'] = "Please do not refresh, if the page does not jump automatically, please enter the IP on LCD screen.";
	
    /* 主页 */
	$lang['home'] = "Home";
	$lang['home_lifetimepower'] = "Lifetime generation";
	$lang['home_systemp'] = "Last System Power";
	$lang['home_todaypower'] = "Generation of Current Day";
	$lang['home_datetime'] = "Last Connection to website";
	$lang['home_maxnum'] = "Number of Inverters";
	$lang['home_curnum'] = "Last Number of Inverters Online";
	$lang['home_version'] = "Current Software Version";
	$lang['home_file_size'] = "Database Size";
	$lang['home_timezone'] = "Current Time Zone";
	$lang['home_eth0_mac'] = "ECU Eth0 Mac Address";
	$lang['home_wlan0_mac'] = "ECU Wlan0 Mac Address";
	$lang['home_grid_quality'] = "Inverter Comm. Signal Level";
	$lang['home_environment_benefits'] = "ENVIRONMENTAL BENEFITS";
	$lang['home_equivalent'] = "CO<sub>2</sub> Offset Equivalent to";
	$lang['home_gallons'] = "GALLONS";
	$lang['home_trees'] = "TREES";
	$lang['home_kg'] = "KG";
	
/* 实时数据 */
	$lang['realtimedata'] = "Real Time Data";
	$lang['realtimedata_current_power'] = "Current Power";
	$lang['realtimedata_grid_frequency'] = "Grid Frequency";
	$lang['realtimedata_grid_voltage'] = "Grid Voltage";
	$lang['realtimedata_temperature'] = "Temperature";
	$lang['realtimedata_date'] = "Reporting Time";
	
	//图表属性
	$lang['graph_language'] = "lang:'en'";
	$lang['graph_title_power'] = "Trend of System Power ";
	$lang['graph_title_energy'] = "Power Generation Statistics";
	$lang['graph_y_label_power'] = "Power (W)";
	$lang['graph_y_label_energy'] = "Energy (kWh)";
	$lang['graph_value_power'] = "Power";
	$lang['graph_value_energy'] = "Energy";
	$lang['graph_daily_energy'] = "Solar Generated Today";
	$lang['graph_weekly_energy'] = "Solar Generated Current Week";
	$lang['graph_monthly_energy'] = "Solar Generated Current Month";
	$lang['graph_yearly_energy'] = "Solar Generated Current Year";
	$lang['contextButtonTitle'] = "Chart context menu";
	$lang['downloadJPEG'] = "Download JPEG image";
	$lang['downloadPDF'] = "Download PDF document";
	$lang['downloadPNG'] = "Download PNG image";
	$lang['downloadSVG'] = "Download SVG vector image";
	$lang['printChart'] = "Print chart";
		
	//功率曲线图
	$lang['power'] = "Power";
	
	//能量柱状图
	$lang['energy'] = "Energy";
	$lang['energy_weekly'] = "Current Week";
	$lang['energy_monthly'] = "Current Month";
	$lang['energy_yearly'] = "Current Year";
	
	//逆变器工作状态
	$lang['inverter_status'] = "Inverter Status";
	$lang['status_channel'] = "Channel";
	$lang['status_status'] = "Status";
	$lang['status_energy'] = "Energy";
	
/* 系统管理 */
    	$lang['management'] = "Administration";
	
	//ID管理
	$lang['id'] = "ID Management";
	$lang['id_clear_id'] = "Clear ID";
	$lang['id_total'] = "Total";
	$lang['id_correct'] = "Update success";
	$lang['id_error'] = "Format error";
	
	$lang['clear_id_result_0'] = "ID cleared successfully !";
	$lang['set_id_result_0'] = "ID updated successfully !";
	$lang['set_id_result_1'] = "Malformed ID(s): ";
	$lang['set_id_result_2'] = "Please enter an inverter ID !";
	$lang['set_id_result_num'] = "Total";
	
	//时间管理
	$lang['time'] = "Date,Time,Time Zone";
	$lang['time_datetime'] = "Date Time";
	$lang['time_timezone'] = "Time Zone";
	$lang['time_ntp'] = "NTP Server";

	$lang['datetime_result_0'] = "Datetime updated successfully !";
	$lang['datetime_result_1'] = "Datetime updated failed !";	
	$lang['timezone_result_0'] = "Time Zone updated successfully !";
	$lang['timezone_result_1'] = "Time Zone updated failed !";
	$lang['ntp_result_0'] = "Ntp_server updated successfully !";
	$lang['ntp_result_1'] = "Ntp_server updated failed !";
	
	//语言管理
	$lang['language'] = "Language";
	$lang['language_current_language'] = "Current Language";
	$lang['language_english'] = "English";
	$lang['language_chinese'] = "Chinese";
	$lang['language_result_0'] = "Change Language Success";
	$lang['language_result_1'] = "Language configuration file open failed";
	
	//网络管理
	$lang['network'] = "Network Connectivity";
	$lang['network_set_gprs'] = "GPRS Settings";
	$lang['network_use_gprs'] = "Use GPRS Module";
	$lang['network_set_ip'] = "IP Settings";
	$lang['network_use_dhcp'] = "Obtain an IP address automatically";
	$lang['network_use_static_ip'] = "Use the following IP address";
	$lang['network_ip_address'] = "IP address";
	$lang['network_subnet_mask'] = "Subnet mask";
	$lang['network_default_gateway'] = "Default gateway";
	$lang['network_preferred_dns_server'] = "Preferred DNS server";
	$lang['network_alternate_dns_server'] = "Alternate DNS server";
	
	$lang['gprs_result_0_1'] = "GPRS enabled successfully !";
	$lang['gprs_result_0_0'] = "GPRS disabled successfully !";
	$lang['gprs_result_1_1'] = "GPRS enabled failed !";
	$lang['gprs_result_1_0'] = "GPRS disabled failed !";
	$lang['network_result_success_dhcp'] = "Dynamic IP updated successfully !";
	$lang['network_result_success_static_ip'] = "Static IP updated successfully !";
	$lang['network_result_success'] = "Set IP successfully, please use the new IP to open pages";
		
	//无线网络管理
	$lang['wlan'] = "WLAN ";
	$lang['wlan_mode'] = "Mode";
	$lang['wlan_ssid'] = "SSID";
	$lang['wlan_ip_address'] = "IP address";
	$lang['wlan_state'] = "State";
	$lang['wlan_change_to_ap_mode'] = "Change to Local Wireless Access";
	$lang['wlan_change_to_sta_mode'] = "Connect to WLAN nearby";
	$lang['wlan_reboot'] = "WARNING : This operation will reboot ECU, to determine execution click on the ' OK' , otherwise click 'Cancel' .";
	  //AP
	$lang['wlan_mode_ap'] = "LWA";
	$lang['wlan_ifopen_1'] = "Opened";
	$lang['wlan_ifopen_0'] = "Closed";
	$lang['wlan_ap_setting'] = "Set Up Local Wireless Access";
	$lang['wlan_ap_ssid'] = "SSID";
	$lang['wlan_ap_channel'] = "Channel";
	$lang['wlan_ap_channel_auto'] = "Auto";	
	$lang['wlan_ap_method'] = "Safe Type";
	$lang['wlan_ap_password'] = "Password";
	$lang['wlan_ap_update'] = "Update";	
	  //STA	
	$lang['wlan_mode_sta'] = "Connected";
	$lang['wlan_ifconnect_1'] = "Connected";
	$lang['wlan_ifconnect_0'] = "Disconnected";
	$lang['wlan_sta_signals'] = "Available Networks";
	$lang['wlan_sta_ssid'] = "SSID";
	$lang['wlan_sta_quality'] = "Quality";
	$lang['wlan_sta_password'] = "Password";
	$lang['wlan_sta_connect'] = "Connect";
	$lang['wlan_sta_disconnect'] = "Disconnect";
	//wlan
	$lang['wlan_connecting'] = "Connecting...";
	$lang['wlan_disconnecting'] = "Disconnecting...";
	// hotspot
	$lang['hotspot_set'] = "Change Local Wireless Access settings";
	$lang['hotspot_set_info'] = "Are you sure you want to change Local Wireless Access settings? This will reboot ECU.";
		
	$lang['wlan_result_success_change_mode'] = "Setting success";
	$lang['wlan_result_success_set_ap'] = "Set Local Wireless Access parameter success";
	$lang['wlan_result_success_connect_sta'] = "Connect WIFI success";
	$lang['wlan_result_success_disconnect_sta'] = "Disconnect WIFI success";
	$lang['wlan_result_failed_connect_sta'] = "Connect WIFI failed";
	$lang['wlan_result_failed_wrong_password'] = "password error";
	
/* 参数配置 */
	$lang['configuration'] = "Configuration";
	
	//登录页面
	$lang['login'] = "Login";
	$lang['login_title'] = "Login";
	$lang['login_username'] = "Username";
	$lang['login_password'] = "Password";
	$lang['login_login'] = "Login";	
	$lang['login_result_0'] = "Login successful !";
	$lang['login_result_1'] = "Incorrect username or password !";
	
	//交流保护参数
	$lang['protection'] = "Parameters";
	$lang['protection2'] = "Parameters";
	$lang['protection_set'] = "Settings";
	$lang['protection_actual_value'] = "Actual value";	
	$lang['protection_select_inverter'] = "Select Inverter";
	$lang['protection_select_inverter_all'] = "All Inverters";
	$lang['protection_under_voltage_fast'] = "Under voltage (stage 1)";
	$lang['protection_over_voltage_fast'] = "Over voltage (stage 1)";
	$lang['protection_under_voltage_slow'] = "Under voltage (stage 3)";
	$lang['protection_over_voltage_slow'] = "Over voltage (stage 2)";
	$lang['protection_under_frequency_fast'] = "Under frequency (stage 1)";
	$lang['protection_over_frequency_fast'] = "Over frequency (stage 1)";
	$lang['protection_under_frequency_slow'] = "Under frequency (stage 2)";
	$lang['protection_over_frequency_slow'] = "Over frequency (stage 2)";
	$lang['protection_voltage_triptime_fast'] = "Voltage 1 clearance time";
	$lang['protection_voltage_triptime_slow'] = "Voltage 2 clearance time";
	$lang['protection_frequency_triptime_fast'] = "Frequency 1 clearance time";
	$lang['protection_frequency_triptime_slow'] = "Frequency 2 clearance time";
	$lang['protection_regulated_dc_working_point'] = "Regulated dc working point";
	$lang['protection_under_voltage_stage_2'] = "Under voltage (stage 2)";
	$lang['protection_voltage_3_clearance_time'] = "Voltage 3 clearance time";
	$lang['protection_start_time'] = "Start time";
	$lang['protection_grid_recovery_time'] = "Reconnection time";
	$lang['protection_read_parameters'] = "Read parameters";
	
	$lang['protection_result_0'] = "Parameters saved successfully ! (See the results 5 minutes later)";
	$lang['protection_result_1'] = "Please enter at least one protection parameters !";
	$lang['read_protection_result_0'] = "Parameter read successfully ! (See the results 5 minutes later)";
	$lang['read_protection_result_1'] = "Parameter read failed !";
	
	//GFDI设置
	$lang['gfdi'] = "GFDI";
	$lang['gfdi_state'] = "Status";
	$lang['gfdi_unlock'] = "Unlock GFDI";
	
	$lang['gfdi_result_0'] = "See the results 5 minutes later";
	$lang['gfdi_result_1'] = "Please select inverters need to be unlocked";
	
	//远程控制开关机
	$lang['switch'] = "Remote Control";
	$lang['switch_state'] = "Status";
	$lang['switch_turn_on'] = "Turn On";
	$lang['switch_turn_off'] = "Turn Off";
	$lang['switch_turn_on_off'] = "Turn On/Off";
	$lang['switch_turn_on_all'] = "Turn on all inverters";
	$lang['switch_turn_off_all'] = "Turn off all inverters";
	
	$lang['switch_result_0'] = "See the results 5 minutes later !";
	$lang['switch_result_1'] = "Please select inverters !";
	$lang['switch_result_2'] = "Settings saved failed !";
	
	//最大功率设置
	$lang['maxpower'] = "Power Setting";
	$lang['maxpower_maxpower'] = "Maximum Power (20-300W)";
	$lang['maxpower_actual_maxpower'] = "Actual Maximum Power";
	
	$lang['maxpower_result_0'] = "See the results 5 minutes later !";
	$lang['maxpower_result_1'] = "Please enter an integer between 20-300 !";
	$lang['maxpower_result_2'] = "Read maxpower failed !";

	//用户管理
	$lang['user_info'] = "User Management";
	$lang['user_info_username'] = "Username";
	$lang['user_info_new_username'] = "New Username";
	$lang['user_info_old_password'] = "Old Password";
	$lang['user_info_new_password'] = "New Password";
	$lang['user_info_confirm_password'] = "Confirm Password";
	$lang['user_info_change_password'] = "Change Password";
	
	$lang['user_info_result_0'] = "Password updated successfully !";
	$lang['user_info_result_1'] = "Incorrect username or old password !";
	$lang['user_info_result_2'] = "Please enter the new password !";
	$lang['user_info_result_3'] = "The passwords you entered do not match !";
	
/* 显示数据库数据 */
	//status
	$lang['status'] = "Status";
	$lang['status2'] = "Status2";
	$lang['display_status_event_id'] = "Event Id";
	$lang['display_status_event'] = "Event";
	$lang['display_status_date'] = "Date";
	$lang['display_status_event_0'] = "AC Frequency Under Range";
	$lang['display_status_event_1'] = "AC Frequency Exceeding Range";
	$lang['display_status_event_2'] = "AC Voltage Exceeding Range";
	$lang['display_status_event_3'] = "AC Voltage Under Range";
	$lang['display_status_event_7'] = "Over Critical Temperature";
	$lang['display_status_event_11'] = "GFDI Locked";
	$lang['display_status_event_12'] = "Turned Off";
	$lang['display_status_event_13'] = "Active Anti-island Protection";
	$lang['display_status_event_14'] = "CP Protection";
	$lang['display_status_event_15'] = "HV Protection";
	$lang['display_status_event_16'] = "Over Zero Protection";
	
	//status_zigbee
	$lang['status_zigbee'] = "Status(ZigBee)";
	$lang['display_status_zigbee_0'] = "AC Frequency Exceeding Range";
	$lang['display_status_zigbee_1'] = "AC Frequency Under Range";
	$lang['display_status_zigbee_2'] = "Channel A: AC Voltage Exceeding Range";
	$lang['display_status_zigbee_3'] = "Channel A: AC Voltage Under Range";
	$lang['display_status_zigbee_4'] = "Channel B: AC Voltage Exceeding Range";
	$lang['display_status_zigbee_5'] = "Channel B: AC Voltage Under Range";
	$lang['display_status_zigbee_6'] = "Channel C: AC Voltage Exceeding Range";
	$lang['display_status_zigbee_7'] = "Channel C: AC Voltage Under Range";
	$lang['display_status_zigbee_8'] = "Channel A: DC Voltage Too High";
	$lang['display_status_zigbee_9'] = "Channel A: DC Voltage Too Low";
	$lang['display_status_zigbee_10'] = "Channel B: DC Voltage Too High";
	$lang['display_status_zigbee_11'] = "Channel B: DC Voltage Too Low";
	$lang['display_status_zigbee_16'] = "Over Critical Temperature";
	$lang['display_status_zigbee_17'] = "GFDI Locked";
	$lang['display_status_zigbee_18'] = "Remote Shut";
	$lang['display_status_zigbee_19'] = "AC Disconnect";
	$lang['display_status_zigbee_21'] = "Active Anti-island Protection";
	$lang['display_status_zigbee_22'] = "CP Protection";

		//显示数据库
	$lang['database'] = "Database";
	$lang['historical_data'] = "Historical_data";
	$lang['record'] = "Record";
	
/* 隐藏功能 */
	$lang['hidden_index'] = "Hidden Pages List";
	
	//debug
	$lang['debug'] = "Debug";
	$lang['debug_command_input'] = "Please enter a custom command";
	$lang['debug_command_execute'] = "Execute";
	$lang['debug_command_success'] = "Command Success";
	$lang['debug_command_failed'] = "Command Failed";	
	$lang['debug_command_is_null'] = "Command cannot be empty";	
	
	//导数据
	$lang['export_file'] = "Export Historical Data";
	$lang['export_file_start_time'] = "Start Time";
	$lang['export_file_end_time'] = "End Time";
	$lang['export_file_export'] = "Export";
	
	//EMA服务器地址与端口
        $lang['datacenter'] = "Datacenter";
	$lang['datacenter_domain'] = "Domain Name";
	$lang['datacenter_ip'] = "IP Address";
	$lang['datacenter_port1'] = "Port 1";
	$lang['datacenter_port2'] = "Port 2";
	$lang['datacenter_update'] = "Update";
	
	$lang['datacenter_result_0'] = "Server address and port has been modified";
	
	//自动更新服务器地址与端口
	$lang['updatecenter'] = "Updatecenter";
	$lang['updatecenter_domain'] = "Domain Name";
	$lang['updatecenter_ip'] = "IP Address";
	$lang['updatecenter_port'] = "Port";
	$lang['updatecenter_update'] = "Update";
	
	$lang['updatecenter_result_0'] = "Server address and port has been modified";

	//初始化数据库
	$lang['initialize'] = "Initialize the database";
	$lang['initialize_clear_energy'] = "Clear Energy";
	$lang['initialize_success'] = "Clear energy success";
	$lang['initialize_failed'] = "Clear energy failed";
	
	//串口
	$lang['serial'] = "Serial";
	$lang['serial_switch'] = "Switch";
	$lang['serial_switch_on'] = "On";
	$lang['serial_switch_off'] = "Off";
	$lang['serial_baud_rate'] = "Serial port baud rate";
	$lang['serial_ecu_address'] = "ECU address";
	$lang['serial_update'] = "Update";
	
	$lang['serial_result_0'] = "Serial has been modified";
	$lang['serial_result_1'] = "Serial modification fails";
	
	//电网环境
	$lang['grid_environment'] = "Grid Environment";
	$lang['grid_environment_result'] = "Result";
	$lang['grid_environment_setting'] = "Grid Environment";
	$lang['grid_environment_select'] = "--Select Grid Environment--";

	$lang['grid_environment_result_0'] = "Set Grid Environment Success";
	$lang['grid_environment_result_1'] = "Please Select Grid Environment";
	$lang['grid_environment_result_2'] = "Set Grid Environment Failed";
	
	//IRD控制
	$lang['ird'] = "IRD";
	$lang['ird_result'] = "Result";
	$lang['ird_setting'] = "IRD Mode";
	$lang['ird_select'] = "--Select IRD Mode--";
	$lang['ird_select_1'] = "Turn off IRD";
	$lang['ird_select_2'] = "Turn on IRD with locked";
	$lang['ird_select_3'] = "Turn on IRD without locked";
	
	$lang['ird_result_0'] = "Set IRD Mode Success";
	$lang['ird_result_1'] = "Please Select a IRD Mode";
	$lang['ird_result_2'] = "Set IRD Mode Failed";
	
	//逆变器信号强度
	$lang['signal_level'] = "Inverter Comm. Signal Level";
	$lang['signal_level'] = "Inverter Comm. Signal Level";
	
	$lang['signal_level_result_0'] = "Read Signal Level Success";
	$lang['signal_level_result_1'] = "Read Signal Level Failed";
	
	//上传文件到ECU临时目录
	$lang['upload'] = "Upload";
	$lang['upload_filename'] = "Filename";
	$lang['upload_browse'] = "Browse";
	
	$lang['upload_result_0'] = "Your file was successfully uploaded !";
	$lang['upload_result_1'] = "Your file was uploaded failed !";
	$lang['upload_result_2'] = "Please upload file less than 8Mb !";
	$lang['upload_result_3'] = "Your file was already exists!";
	
	//本地升级ECU
	$lang['upgrade_ecu'] = "Firmware Update";
	$lang['upgrade_ecu_filename'] = "Upload Package";
	$lang['upgrade_ecu_browse'] = "Browse";
	
	$lang['upgrade_ecu_result_0'] = "Update Successfully !";
	$lang['upgrade_ecu_result_1'] = " File was not uploaded !";
	$lang['upgrade_ecu_result_2'] = "Please upload file less than 8Mb !";

	//远程更新逆变器
	$lang['all_inverter'] = "All Inverter";
	$lang['part_of_inverter'] = "Part Of Inverter";
	$lang['remoteupdate_flag'] = "Remote Update Flag";
	$lang['remote_update'] = "Remote Update";

	//信道设置
	$lang['channel_now'] = "ECU's Channel Now";
	$lang['old_channel'] = "Old Channel";
	$lang['new_channel'] = "New Channel";
	$lang['short_addr'] = "Short Address";
	$lang['bind_flag'] = "Bind Flag";

	//加密
	$lang['encryption'] = "Encryption";
	$lang['encrypted'] = "This system is encrypted！";
	$lang['psword'] = "Password";
	$lang['mod_time'] = "Modification Time";
	$lang['modi_time'] = "Time";
	$lang['encrypt_result_1'] = "Set Password failed!";
	$lang['encrypt_result_0'] = "Set Password success!";
	$lang['modi_time_result_0'] = "Set Time success!";
	$lang['modi_time_result_1'] = "Set Time failed!";
	$lang['modi_time_result_2'] = "Please input password and time!";
	
	//防盗心跳包时间设置
	$lang['heartbeat'] = "Heartbeat";
	$lang['set_heartbeat'] = "Set Heartbeat";
	$lang['set_heartbeat_result_0'] = "Set Heartbeat success!";
	$lang['set_heartbeat_result_1'] = "Please input time !";	
?>