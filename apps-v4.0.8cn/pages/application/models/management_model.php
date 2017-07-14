<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Management_model extends CI_Model {

    public $pdo;

    public function __construct() 
    {
        parent::__construct();
        $dsn = 'sqlite:'.APPPATH.'../../../database.db';
        $this->pdo = new PDO($dsn);          
    }

    /* 获取逆变器ID列表 */
    public function get_id() 
    {
        $temp = array();

        $query = "SELECT id FROM id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
            foreach ($res as $key => $value) {
            	$temp[$key] = $value[0];
            }
        }
        
        $data['ids'] = $temp;

        return $data;
    }

    /* 设置逆变器ID列表 */
    public function set_id() 
    {
        //用于保存逆变器ID的数组
        $results = array();
        $ids = array();
        $error_ids = array();
        $results["num"] = 0;
        
        $reset = $this->input->post('reset');
    	//若输入为空，则保存错误标志，退出该函数
    	if(!($this->input->post('ids'))) {
    	    $results["value"] = 2; // 输入为空
    	    return $results;
    	}
    	//将textarea中的每一行数据存入数组
		$temp = preg_split('/\n/', $this->input->post('ids'));
// 		if (strtoupper(substr(PHP_OS, 0, 3)) === 'WIN'){
// 		    $temp = preg_split('/\r\n/', $this->input->post('ids'));
// 		}

        //检验数据格式(是否为12位纯数字)
        foreach ($temp as $key => $id) 
        {
            if(ctype_digit($id) && (strlen($id)==12))
            {
                array_push($ids, $id);
            }
            else if(strlen($id))
            {
                array_push($error_ids, $id);
            }
        }

        //查询原来的ID
        $old_ids = array();
        $query = "SELECT id FROM id";
        $result = $this->pdo->prepare($query);
        if (!empty($result)) {
            $result->execute();
            $res = $result->fetchAll(PDO::FETCH_NUM);
            foreach ($res as $key => $value) {
                $old_ids[$key] = $value[0];
            }
        }

        //保存ID并删除重复的ID
        $results['ids'] = array_unique($ids);

        //保存格式错误的ID
        $results['error_ids'] = $error_ids;

        //筛选出要新增的ID
        $add_ids = array_diff($results['ids'], $old_ids);
        //筛选出要删除的ID
        $delete_ids = array_diff($old_ids, $results['ids']);

        //将正确的ID加入数据库
        if(count($results['ids']))
        {
            //重置绑定标志位
            $this->pdo->exec("UPDATE id SET 
                model=null, 
                software_version=null, 
                bind_zigbee_flag=null, 
                turned_off_rpt_flag=null");
            //新增
            $times = ceil(count($add_ids)/500);//计算插入次数(一次最多插入500条)
            for ($i=0; $i<$times; $i++) {
                $sql = "INSERT INTO id (id, flag) VALUES ";
                foreach (array_slice($add_ids, $i*500, 500) as $id) {
                    $sql.= " ('$id', 0),";                   
                }
                $this->pdo->exec(rtrim($sql, ","));
            }
            //删除
            $times = ceil(count($delete_ids)/500);
            for ($i=0; $i<$times; $i++) {
                $sql = "DELETE FROM id WHERE id='0' ";
                foreach (array_slice($delete_ids, $i*500, 500) as $id) {
                    $sql.= "OR id='$id' ";
                }
                $this->pdo->exec($sql);
            }
            

            //power表(注：最好能设置power表的主键为id,采用replace添加新的逆变器)
            $query = "SELECT item FROM power";
            $result = $this->pdo->prepare($query);            
            if(!empty($result)){
                $result->execute();
                $res = $result->fetchAll();
                $item = count($res);
            }
            foreach ($results['ids'] as $key => $id) {
                $query = "SELECT id FROM power WHERE id='$id'";
                $result = $this->pdo->prepare($query);
                if(!empty($result)){
                    $result->execute();
                    $res = $result->fetchAll();
                    if(!count($res)){
                        //注：flag为0说明初始时并不设置最大功率，是否先不写进power表，用join查找id与power表，并显示默认值250
                        //当需要设置时再写入power表？（提高inputid页面的速度）
                        $this->pdo->exec("INSERT INTO power VALUES($item, \"$id\", 258, '-', 258, '-', 0)");
                        $item++;
                    }
                }
            }
            system('killall main.exe');
            $fp = @fopen("/etc/yuneng/autoflag.conf", 'w');
            if($fp){
                fwrite($fp, "0");
                fclose($fp);
            }           
            $fp = @fopen("/etc/yuneng/reconfigtn.conf", 'w');
            if($fp){
                fwrite($fp, "1");
                fclose($fp);
            }
            $fp = @fopen("/etc/yuneng/presetdata.txt", 'w');
            if($fp){
                fclose($fp);
            }
            //新增
            $fp = @fopen("/etc/yuneng/limitedid.conf", 'w');
            if($fp){
                fwrite($fp, "1");
                fclose($fp);
            }
            if ($reset == 1) {
                $fp = @fopen("/etc/yuneng/process_reset.conf", 'w');
                if($fp){
                    fwrite($fp, "1");
                    fclose($fp);
                }
            }            
            
            if(!empty( $results['error_ids'])) {
                $results["value"] = 1; // 存在错误ID
            }
            else {
                $results["value"] = 0; // 保存逆变器成功,没有错误的ID
            }
            

        /* 将ECU本地页面变动数据存入数据库 */
            //创建表单
            $this->pdo->exec("CREATE TABLE IF NOT EXISTS process_result
                (item INTEGER, result VARCHAR, flag INTEGER, 
                primary key(item))");
            
            //ECU_id
            $ecuid = "000000000000";
            $fp = @fopen("/etc/yuneng/ecuid.conf",'r');
            if($fp)
            {
                $ecuid = fgets($fp);
                fclose($fp);
            }

            //ECU_version
            $version = "";        
            $fp = @fopen("/etc/yuneng/version.conf",'r');
            if($fp)
            {
                $version = trim(fgets($fp));
                fclose($fp);
            }
            $fp = @fopen("/etc/yuneng/area.conf",'r');
            if($fp)
            {
                $version .= trim(fgets($fp));
                fclose($fp);
            }

            //ECU_version_number
            $version_number = "";        
            $fp = @fopen("/etc/yuneng/version_number.conf",'r');
            if($fp)
            {
                $version_number = trim(fgets($fp));
                fclose($fp);
            }

            //当前逆变器数量
            $num = sprintf("%03d", count($results['ids']));

            //初始化消息体
            if(strlen($version_number))
            {
                $length = sprintf("%02d",strlen($version)+2+strlen($version_number));
                $record = "APS1300000A102AAA0".$ecuid.$length.$version."--".$version_number.$num."00000000000000END";
            }
            else
            {
                $length = sprintf("%02d",strlen($version));
                $record = "APS1300000A102AAA0".$ecuid.$length.$version.$num."00000000000000END";
            }

            //当前逆变器ID
            foreach ($results['ids'] as $value) {
                $record = $record.$value."00"."00000"."END";
            }

            //计算消息长度并加上回车符号
            $record_length = sprintf("%05d", strlen($record));
            $record = substr_replace($record, $record_length, 5, 5);
            $record = $record."\n";
            
            //将消息保存到数据库
            $sql = "REPLACE INTO process_result (item, result, flag) VALUES(102, '$record', 1)";
            $this->pdo->exec($sql);
        }
        else {
            $results["value"] = 1; // 存在错误ID
        }
        $results["num"] = 0;
        $sql = "SELECT COUNT(id) FROM id";
        $stmt = $this->pdo->prepare($sql);
        $stmt->execute();
        if (($res = $stmt->fetch()) !== false) {
            $results["num"] = $res[0];
        }

        return $results;
    }

     /* 清空逆变器列表 */
    public function set_id_clear()
    {
        $results = array();

        //清空逆变器列表 
        $this->pdo->exec("DELETE FROM id");
        system('killall main.exe');
        $fp = @fopen("/etc/yuneng/limitedid.conf", 'w');
        if($fp){
            fwrite($fp, "0");
            fclose($fp);
        }

        $results["value"] = 0;

        return $results;   
    }

    /* 获取时间信息 */
    public function get_datetime() 
    {        
        $data = array();

        //获取ECU当前设置的时区
        $data['timezone'] = "Asia/Shanghai";
        $fp = @fopen("/etc/yuneng/timezone.conf",'r');
        if ($fp)
        {
          $data['timezone'] = fgets($fp);
          fclose($fp);
        }
        date_default_timezone_set($data['timezone']);//设置默认时区

        //获取ECU当前设置的NTP服务器
        $data['ntp'] = "0.asia.pool.ntp.org";
        $fp = @fopen("/etc/yuneng/ntp_server.conf",'r');
        if ($fp)
        {
          $data['ntp'] = fgets($fp);
          fclose($fp);
        }

        return $data;
    }

    /* 设置日期时间 */
    public function set_datetime() 
    {
        $results = array();

        //读取当前时区
        $timezone = "Asia/Shanghai";
        $fp = @fopen("/etc/yuneng/timezone.conf",'r');
        if ($fp)
        {
          $timezone = fgets($fp);
          fclose($fp);
        }

        //读取输入日期时间
        $datetime = $this->input->post('datetime');

        //将日期转换成时间戳
        date_default_timezone_set($timezone);
        if($timestamp = strtotime($datetime))
        {
            //将时间保存至系统文件/dev/rtc2
            $datetime = getdate($timestamp);
            $save_datetime = sprintf("%04d%02d%02d%02d%02d%02d",
                    $datetime['year'], $datetime['mon'], $datetime['mday'],
                    $datetime['hours'], $datetime['minutes'], $datetime['seconds']);
            $fp = @fopen("/dev/rtc2", 'w');
            if($fp)
            {
                fwrite($fp, $save_datetime);
                fclose($fp);
            }

            //设置linux系统时间
            //date_default_timezone_set("UTC");
            $datetime = getdate($timestamp);
            $cmd = sprintf("date %02d%02d%02d%02d%04d.%02d",
                    $datetime['mon'], $datetime['mday'], $datetime['hours'],
                    $datetime['minutes'], $datetime['year'], $datetime['seconds']);
            exec($cmd);

            $results["value"] = 0;
            
            //重启主函数和客户端函数
            system("killall main.exe");
            system("killall client");
        }
        else
        {
            //格式错误，转换为时间戳失败
            $results["value"] = 1;
        }

        return $results;
    }

    /* 设置时区 */
    public function set_timezone() 
    {
        $results = array();

        //获取页面选择的时区 
        $timezone = $this->input->post('timezone');      

        //设置linux系统时区
        $cmd = "cp /usr/share/zoneinfo/$timezone /etc/localtime";
        system($cmd);

        //将时区保存到配置文件
        $fp = @fopen("/etc/yuneng/timezone.conf",'w');
        if($fp){
            fwrite($fp, $timezone);
            fclose($fp);
        }

        //计算ECU本地时间并存入时钟芯片
        date_default_timezone_set($timezone);
        $localtime_assoc = localtime(time(), true);
        $local_time = sprintf("%04d%02d%02d%02d%02d%02d",
                            $localtime_assoc['tm_year'] + 1900,
                            $localtime_assoc['tm_mon'] + 1,
                            $localtime_assoc['tm_mday'],
                            $localtime_assoc['tm_hour'],
                            $localtime_assoc['tm_min'],
                            $localtime_assoc['tm_sec']
                            );
        $fp = @fopen("/dev/rtc2", 'w');
        if ($fp) {
            fwrite($fp, $local_time);
            fclose($fp);
        }

        //重启主函数和客户端函数
        //system("/home/application/ntpapp.exe");
        system("killall main.exe");
        system("killall client");

        $results["value"] = 0;      

    /* 将ECU本地页面变动数据存入数据库 */
        //ECU_id
        $ecuid = "000000000000";
        $fp = @fopen("/etc/yuneng/ecuid.conf",'r');
        if($fp)
        {
            $ecuid = fgets($fp);
            fclose($fp);
        }

        //初始化消息体
        $record = "APS1300000A104AAA0".$ecuid.$local_time."00000000000000".$timezone."END";

        //计算消息长度并加上回车符号
        $record_length = sprintf("%05d", strlen($record));
        $record = substr_replace($record, $record_length, 5, 5);
        $record = $record."\n";

        //将消息保存到数据库
        $sql = "REPLACE INTO process_result (item, result, flag) VALUES(104, '$record', 1)";
        $this->pdo->exec($sql);

        return $results;
    }

    /* 设置NTP服务器 */
    public function set_ntp_server() 
    {        
        $results = array();

        //获取页面输入的NTP服务器地址
        $ntp = $this->input->post('ntp');

        //保存NTP服务器地址
        $fp = @fopen("/etc/yuneng/ntp_server.conf",'w');
        if($fp)
        {
            fwrite($fp, $ntp);
            fclose($fp);
            $results["value"] = 0;
        }
        else 
        {
            $results["value"] = 1;
        }

        return $results;
    }

    /* 获取语言信息 */
    public function get_language() 
    {        
        $data = array();

        $data['language'] = "english";
        $fp = @fopen("/etc/yuneng/language.conf",'r');
        if ($fp)
        {
          $data['language'] = fgets($fp);
          fclose($fp);
        }
        else if($this->session->userdata("language"))
        {
            $data['language'] = $this->session->userdata("language");
        }

        return $data;
    }

    /* 设置语言信息 */
    public function set_language()
    {
        $results = array();
               
        $language = $this->input->post('language');
        $this->session->set_userdata("language", $language);
        
        $fp = @fopen("/etc/yuneng/language.conf",'w');
        if ($fp){
          fwrite($fp, $language);
          fclose($fp);
          $results["value"] = 0;
        }
        else{
            //文件打开失败
            $results["value"] = 1;
        }       

        return $results;
    }

    /* 获取ECU网络配置 */
    public function get_network() 
    {        
        $data = array();

        //获取DHCP标志位
        $data['dhcp'] = 1;
        $fp = @fopen("/etc/yuneng/dhcp.conf",'r');
        if ($fp)
        {
           $data['dhcp'] = intval(fgets($fp));          
          fclose($fp);
        }

        //获取静态IP地址信息
        $ip = "";
        $mask = "";
        $gate = "";
        $dns1 = "";
        $dns2 = "";

        if ($data['dhcp'] == 0)
        {
            $fp = fopen("/etc/network/interfaces", 'r');
            if ($fp) 
            {
                while(!feof($fp))
                {
                    $temp = fgets($fp);
                    if(!strncmp($temp, "address", 7))
                    {
                        $ip = substr($temp, 8);
                    }
                    if(!strncmp($temp, "netmask", 7))
                    {
                        $mask = substr($temp, 8);
                    }
                    if(!strncmp($temp, "gateway", 7))
                    {
                        $gate = substr($temp, 8);
                    }
                }
                fclose($fp);
            }
            $fp = fopen("/etc/resolv.conf", 'r');
            if ($fp) 
            {
                $temp = fgets($fp);
                if(!strncmp($temp, "nameserver", 10))
                {
                    $dns1 = substr($temp, 11);
                }
                $temp = fgets($fp);
                if(!strncmp($temp, "nameserver", 10))
                {
                    $dns2 = substr($temp, 11);
                }
 
                fclose($fp); 
            }
        }
        $data['ip'] = $ip;
        $data['mask'] = $mask;
        $data['gate'] = $gate;
        $data['dns1'] = $dns1;
        $data['dns2'] = $dns2;

        return $data;
    }

    /* 设置IP */
    public function set_ip() 
    {
        $results = array();
        
        //获取DHCP设置选项
        $dhcp = $this->input->post('dhcp');
        
        if($dhcp == "1")
        {   
            //使用动态IP地址       
            //保存DHCP状态
            $fp = @fopen("/etc/yuneng/dhcp.conf",'w');
            if($fp){
                fwrite($fp, $dhcp);
                fclose($fp);
            }
            $fp = @fopen("/etc/network/interfaces", 'w');
            if($fp){
                fwrite($fp, "# /etc/network/interfaces -- configuration file for ifup(8), ifdown(8)\n
# The loopback interface\n
auto lo\n
iface lo inet loopback\n\n
# Wireless interfaces\n
iface wlan0 inet dhcp\n
wireless_mode managed\n
wireless_essid any\n
wpa-driver wext\n
wpa-conf /etc/wpa_supplicant.conf\n\n
iface tiwlan0 inet dhcp\n
wireless_mode managed\n
wireless_essid any\n\n
iface atml0 inet dhcp\n\n
# Wired or wireless interfaces\n
auto eth0\n
iface eth0 inet dhcp\n
        pre-up /bin/grep -v -e \"ip=[0-9]\\+\\.[0-9]\+\.[0-9]\+\.[0-9]\+\" /proc/cmdline > /dev/null\n\n
iface eth1 inet dhcp\n\n
# Ethernet/RNDIS gadget (g_ether)\n
# ... or on host side, usbnet and random hwaddr\n
iface usb0 inet dhcp\n\n
# Bluetooth networking\n
iface bnep0 inet dhcp");
                fclose($fp);
            }
            
            //重启网络
            system("/etc/rcS.d/S40networking restart");
            $results["value"] = 0;            
        }
        else if($dhcp == "0")
        {
            //使用静态IP地址
            //获取静态IP地址信息
            $ip = $this->input->post('ip');
            $mask = $this->input->post('mask');
            $gate = $this->input->post('gate');
            $dns1 = $this->input->post('dns1');
            $dns2 = $this->input->post('dns2');

            //将静态IP地址信息写入文件
            $fp = @fopen("/etc/network/interfaces", 'w');
            if($fp){
                fwrite($fp, "# /etc/network/interfaces -- configuration file for ifup(8), ifdown(8)\n
# The loopback interface\n
auto lo\n
iface lo inet loopback\n\n         
# Wireless interfaces\n
iface wlan0 inet dhcp\n
wireless_mode managed\n
wireless_essid any\n
wpa-driver wext\n
wpa-conf /etc/wpa_supplicant.conf\n\n
iface tiwlan0 inet dhcp\n
wireless_mode managed\n
wireless_essid any\n\n
iface atml0 inet dhcp\n\n
# Wired or wireless interfaces\n
auto eth0\n");
                fwrite($fp, "iface eth0 inet static\n");
                fwrite($fp, "address ".$ip."\n");
                fwrite($fp, "netmask ".$mask."\n");
                fwrite($fp, "gateway ".$gate."\n");
                fwrite($fp, "iface eth1 inet dhcp\n\n
# Ethernet/RNDIS gadget (g_ether)\n
# ... or on host side, usbnet and random hwaddr\n
iface usb0 inet dhcp\n\n
# Bluetooth networking\n
iface bnep0 inet dhcp");
                fclose($fp);
            }            

            $fp = @fopen("/etc/yuneng/resolv.conf", 'w');
            if($fp){
                fwrite($fp, "nameserver ".$dns1."\n");
                if (strlen($dns2)) {
                    fwrite($fp, "nameserver ".$dns2."\n");
                }
                fclose($fp);
            }
            system("cp /etc/yuneng/resolv.conf /etc/");

            //保存DHCP状态
            $fp = @fopen("/etc/yuneng/dhcp.conf",'w');
            if($fp){
                fwrite($fp, $dhcp);
                fclose($fp);
            }

            //重启网络
            system("/sbin/udhcpc -nq -i eth0 >/dev/null 2>&1");            
            $results["value"] = 0;
        }       
        return $results;
    }

    /* 获取无线局域网信息 */
    public function get_wlan()
    {
        $data['ap_info']['ssid'] = "";
        $data['ap_info']['channel'] = 0;
        $data['ap_info']['method'] = 0;
        $data['ap_info']['psk'] = "";
        $data['ap_info']['ip'] = "172.30.1.1";
        $data['ssid'] = "-";
        $data['ip'] = "-";
        $data['ifconnect'] = 0;
        $data['wifi_signals'] = "";
        $data['num'] = 0;

        //读取主机模式参数
        $fp = @fopen("/etc/yuneng/wifi_ap_info.conf", 'r');
        if($fp){
            while(!feof($fp)){
                $temp = fgets($fp);
                if(strncmp($temp, "#", 1))
                    break;
                if(!strncmp($temp, "#SSID", 5)){
                    $temp = substr($temp, 6);
                    sscanf($temp, "%[^\n]", $data['ap_info']['ssid']);
                }
                if(!strncmp($temp, "#channel", 8)){
                    $temp = substr($temp, 9);
                    $data['ap_info']['channel'] = intval($temp);
                }
                if(!strncmp($temp, "#method", 7)){
                    $temp = substr($temp, 8);
                    $data['ap_info']['method'] = intval($temp);
                }
                if(!strncmp($temp, "#psk", 4)){
                    $temp = substr($temp, 5);
                    sscanf($temp, "%[^\n]", $data['ap_info']['psk']);
                }
            }
        }

        //获取wifi信号名称
        exec("/usr/sbin/iwconfig wlan0 | grep -E 'wlan0' ", $output_ssid);
        if (!empty($output_ssid)) {
            $output_ssid[0] = strstr($output_ssid[0], "ESSID");
            if (!strncmp($output_ssid[0], "ESSID", 5)) {
                $output_ssid[0] = substr($output_ssid[0], 7);
                sscanf($output_ssid[0], "%[^\"]", $data['ssid']);
                $data['ifconnect'] = 1;
            }
        }
        //获取ip
        exec("/sbin/ifconfig wlan0 | grep -E 'inet addr' ", $output_ip);
        if (!empty($output_ip)) {
            $output_ip[0] = substr($output_ip[0], 20);
            sscanf($output_ip[0], "%[^ ]", $data['ip']);
        }

        //扫描无线信号，筛选出SSID、信号强度、加密方式并保存到临时文件
        system("/usr/sbin/iwlist scan | grep -E 'SSID|Quality|Encryption|Group' | sed 's/^ *//' >/tmp/wifi_temp.conf");
        //读取扫描到的wifi信号及信息
        $data['wifi_signals'] = "";
        $num = 0;
        $fp = @fopen("/tmp/wifi_temp.conf", 'r');
        if ($fp) {
            $num = -1;
            while(!feof($fp)){
                $temp = fgets($fp);
                if(!strncmp($temp, "ESSID", 5)){
                    $temp = substr($temp, 7);
                    sscanf($temp, "%[^\"]", $data['wifi_signals'][$num+1]['ssid']);
                    $data['wifi_signals'][$num+1]['quality'] = "";
                    $data['wifi_signals'][$num+1]['ifkey'] = "";
                    $data['wifi_signals'][$num+1]['group'] = "";
                    $num++;
                }
                if(!strncmp($temp, "Quality", 7)){
                    $temp = substr($temp, 8, 2);
                    $data['wifi_signals'][$num]['quality'] = intval($temp);
                }
                if(!strncmp($temp, "Encryption key", 14)){
                    $temp = substr($temp, 15);
                    sscanf($temp, "%[^\n]", $ifkey);
                    if(!strncmp($ifkey, "on", 2))
                        $data['wifi_signals'][$num]['ifkey'] = 1;
                    else
                        $data['wifi_signals'][$num]['ifkey'] = 0;  
                }
                if(!strncmp($temp, "Group Cipher", 12)){
                    $temp = substr($temp, 15);
                    sscanf($temp, "%[^\n]", $data['wifi_signals'][$num]['group']);
                }
            }
            $num++;
            fclose($fp);
        }
        $data['num'] = $num;
        return $data;
    }

    /* 设置主机模式参数 */
    public function set_wlan_ap()
    {
        $results =array();

        //获取主机模式参数
        $ssid = $this->input->post('SSID');
        $channel = intval($this->input->post('channel'));
        $method = intval($this->input->post('method'));
        if($method == 1)
            $psk = $this->input->post('psk_wep');
        else if($method == 2)
            $psk = $this->input->post('psk_wpa');

        if(strlen($ssid))
        {
            //保存主机模式参数到"/tmp/hostapd.conf"
            $fp = @fopen("/tmp/hostapd.conf", 'w');
            if($fp)
            {
                fwrite($fp, "#SSID=".$ssid."\n");
                fwrite($fp, "#channel=".$channel."\n");
                switch ($method) {
                    case '0':
                        fwrite($fp, "#method=0\n");
                        break;
                    case '1':
                        fwrite($fp, "#method=1\n");
                        fwrite($fp, "#psk=".$psk."\n");
                        break;
                    case '2':
                        fwrite($fp, "#method=2\n");
                        fwrite($fp, "#psk=".$psk."\n");
                        break;            
                    default:
                        # code...
                        break;
                }
                fwrite($fp, "interface=wlan1\n");
                fwrite($fp, "driver=ar6000\n");
                fwrite($fp, "ssid=".$ssid."\n");
                fwrite($fp, "channel=".$channel."\n");
                fwrite($fp, "ignore_broadcast_ssid=0\n");
                switch ($method) {
                    case '0':
                        # code...
                        break;
                    case '1':
                        //WEP
                        fwrite($fp, "auth_algs=1\n");
                        fwrite($fp, "wep_key0=\"".$psk."\"\n");
                        fwrite($fp, "wep_key1=1111111111\n");
                        fwrite($fp, "wep_key2=2222222222\n");
                        fwrite($fp, "wep_key3=3333333333\n");
                        fwrite($fp, "wep_default_key=0\n");
                        break;
                    case '2':
                        //WPA
                        fwrite($fp, "wpa=2\n");                
                        fwrite($fp, "wpa_key_mgmt=WPA-PSK\n");
                        fwrite($fp, "wpa_pairwise=CCMP TKIP\n");
                        fwrite($fp, "wpa_passphrase=".$psk."\n");
                        break;            
                    default:
                        # code...
                        break;
                }
                fclose($fp);
            }

            system("cp /tmp/hostapd.conf /etc/yuneng/wifi_ap_info.conf");
                        
            //重启ECU
            system("/sbin/reboot");
            $results["value"] = 0;
        }
        else {
            $results["value"] = 1;
        }
        return $results;
    }

    /* 设置从机模式参数 */
    public function set_wlan_sta()
    {
        //获得要连接的wifi信号的信息
        $ssid_id = $this->input->post('ssid_id');
        $ifconnect = $this->input->post("ifconnect$ssid_id");
        $ssid = $this->input->post("ssid$ssid_id");
        $ifkey = $this->input->post("ifkey$ssid_id");
        $psk = $this->input->post("psk$ssid_id");
        $group = $this->input->post("group$ssid_id");
        if (strlen($ssid) && !$ifconnect) {
            $fp = @fopen("/tmp/wpa_supplicant.conf", 'w');
            fwrite($fp, "ctrl_interface=/var/run/wpa_supplicant\n");
            if ($ifkey) {
                if (strlen($group)) {
                    //WPA/WPA2加密
                    fwrite($fp, "ctrl_interface_group=0\n");
                    fwrite($fp, "ap_scan=1\n");
                    fwrite($fp, "network={\n\tssid=\"".$ssid."\"\n");
                    fwrite($fp, "\tkey_mgmt=WPA-PSK\n");
                    fwrite($fp, "\tproto=WPA RSN\n");
                    fwrite($fp, "\tpairwise=TKIP CCMP\n");
                    fwrite($fp, "\tgroup=TKIP CCMP\n");
                    fwrite($fp, "\tpsk=\"".$psk."\"\n}\n");
                } else {
                    //WEP加密
                    fwrite($fp, "network={\n\tssid=\"".$ssid."\"\n");
                    fwrite($fp, "\tkey_mgmt=NONE\n");
                    fwrite($fp, "\twep_key0=\"".$psk."\"\n");
                    fwrite($fp, "\twep_tx_keyidx=0\n}\n");
                }
            } else {
                //无加密
                fwrite($fp, "network={\n\tssid=\"".$ssid."\"\n");
                fwrite($fp, "\tkey_mgmt=NONE\n}\n");
            }
            fclose($fp);
        }

        //设置从机模式参数
        $results['msg'] = "";
        $results['value'] = 0;
        if($ifconnect == 0)
        {
            system("killall wpa_supplicant");
            system("wpa_supplicant -Dwext -i wlan0 -c /tmp/wpa_supplicant.conf -B >/dev/null 2>&1");
            exec("/sbin/udhcpc -nq -i wlan0 >/dev/null 2>&1", $output, $res_val);
            if (!$res_val) {
                //连接成功
                system("cp /tmp/wpa_supplicant.conf /etc/yuneng/wifi_signal_info.conf");
                $results['msg'] = "success_connect_sta";
                $results['value'] = 1;
            } else {
                //连接失败
                system("killall wpa_supplicant");
                $results['msg'] = "failed_connect_sta";
                $results['value'] = 0;
            }
        }
        else if($ifconnect == 1)
        {
            system("killall wpa_supplicant");
            $results['msg'] = "success_disconnect_sta";
            $results['value'] = 2;
        }
        return $results;
    }

    /* 执行本地升级ECU */
    public function exec_upgrade_ecu()
    {
        $results = array();
        $res_array = array();

        exec("rm -rf /tmp/update_localweb/");
        if ($_FILES["file"]["error"] > 0)
        {
            array_push($res_array, "Return Code: " . $_FILES["file"]["error"] . "<br />");
            $results["value"] = 1;
        }
        else
        {
            array_push($res_array, "Upload: " . $_FILES["file"]["name"] . "<br />");
            array_push($res_array, "Type: " . $_FILES["file"]["type"] . "<br />");
            array_push($res_array, "Size: " . ($_FILES["file"]["size"] / 1024) . " Kb<br />");
            array_push($res_array, "Temp file: " . $_FILES["file"]["tmp_name"] . "<br />");        

            move_uploaded_file($_FILES["file"]["tmp_name"], "/tmp/" . $_FILES["file"]["name"]);
            array_push($res_array, "Stored in: " . "/tmp/" . $_FILES["file"]["name"]);
            exec("tar xjvf /tmp/".$_FILES["file"]["name"]." -C /tmp");
            exec("ls /tmp/update_localweb/assist", $temp, $value);
            exec("/tmp/update_localweb/assist &");
            $results["value"] = $value ? 1 : 0;
        }

        $results["result"] = implode("\n",$res_array);
        return $results;
    }

}


/* End of file management_model.php */
/* Location: ./application/models/management_model.php */