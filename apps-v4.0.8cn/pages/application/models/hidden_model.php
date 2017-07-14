<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Hidden_model extends CI_Model {

    public $pdo;

    public function __construct() 
    {
        parent::__construct();
        $dsn = 'sqlite:'.APPPATH.'../../../database.db';
        $this->pdo = new PDO($dsn);          
    }

    /* 执行DEBUG操作 */
    public function exec_command() 
    {
        $results = array();

        $cmd = $this->input->post('command');

        if(strlen($cmd))
        {
            //将外部程序原始输出存入res_array
            //将外部程序返回值存入value
            exec($cmd, $results["res_array"], $results["value"]);
        }
        else
            $results["value"] = -1;        
      
        return $results;
    }

    /* 获取导出数据的起止时间 */
    public function get_export_time()
    {
        $data = array();
        
        $timezone = "Asia/Shanghai";
        //读取当前时区
        $fp = @fopen("/etc/yuneng/timezone.conf",'r');
        if ($fp)
        {
          $timezone = fgets($fp);
          fclose($fp);
        }
        date_default_timezone_set($timezone);
        $data['start_time'] = date("Y-m-d H:i:s",time()-3600*24)."\n";
        $data['end_time'] = date("Y-m-d H:i:s",time());

        return $data;
    }

    /* 执行导出数据操作 */
    public function exec_export_file()
    {
        $data = array();
        $temp = array();

        //获取起止时间
        $start_time = $this->input->post('start_time');
        $end_time = $this->input->post('end_time');
        sscanf($start_time, "%d-%d-%d %d:%d:%d", $year, $month, $day, $hour, $minute, $second);
        $start = sprintf("%04d%02d%02d%02d%02d%02d", $year, $month, $day, $hour, $minute, $second);
        sscanf($end_time, "%d-%d-%d %d:%d:%d", $year, $month, $day, $hour, $minute, $second);
        $end = sprintf("%04d%02d%02d%02d%02d%02d", $year, $month, $day, $hour, $minute, $second);       

        $title = array(
                    'Inverter ID',
                    'Channel',
                    'DC voltage',
                    'DC current',
                    'Power',
                    'Grid frequency',
                    'Temperature',
                    'Grid voltage',
                    'Energy',
                    'Report date and time'
                );

        //获取数据
        $dsn = 'sqlite:'.APPPATH.'../../../record.db';
        $this->pdo = new PDO($dsn);
        $query = "SELECT record FROM data WHERE date_time BETWEEN $start AND $end";
        $result = $this->pdo->prepare($query);
        $flag_of_fillupdata = 0; //0：不补，1：500的补，2：1000的补，3：APS18的补（1000，还有没600补数据）

        $count = 0;
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll(PDO::FETCH_ASSOC);
            
            //限制文件大小
            foreach($res as $k => $v) {$temp[$k] = $v['record'];}
            //$length = strlen(implode(' ', $temp));
            if(strlen(implode(' ', $temp)) > 1000000) { 
                $this->load->view('hidden/export_file_error');
                return 1;
            }
            
            $count = 0;
            foreach ($res as  $value) {
                //支持版本:APS11,ASP12,APS13
                if(!strncmp($value['record'], "APS11", 5) || !strncmp($value['record'], "APS12", 5) || !strncmp($value['record'], "APS13", 5))
                {
                    $num = (strlen($value['record']) - 87) / 57;
                    for ($i=0; $i<$num; $i++) {
                        $temp = substr($value['record'], 86+57*$i, 57);//每段数据为57个字符
                        $data[$count]['inverter_id'] = ' '.strval(substr($temp, 0, 12));
                        $data[$count]['channel'] = substr($temp, 12, 1);                        
                        $temp = str_replace("A", "0", $temp);//将A替换为0
                        $data[$count]['dc_voltage'] = number_format(floatval(substr($temp, 13, 5)/10), 1);
                        $data[$count]['dc_current'] = number_format(floatval(substr($temp, 18, 3)/10), 1);
                        $data[$count]['power'] = number_format(floatval(substr($temp, 21, 5)/100), 2);
                        $data[$count]['grid_frequency'] = number_format(floatval(substr($temp, 26, 5)/10), 1);

                        if(!strncmp(substr($temp, 31, 1), "B", 1))
                            $data[$count]['temperature'] = "-".intval(substr($temp, 32, 2));
                        else
                            $data[$count]['temperature'] = intval(substr($temp, 31, 3));

                        $data[$count]['grid_voltage'] = intval(substr($temp, 34, 3));
                        $data[$count]['energy'] = number_format(floatval(substr($temp, 48, 6)/1000000), 6);
                        $data[$count]['datetime'] = substr($value['record'], 60, 4)."/".
                                                    substr($value['record'], 64, 2)."/".
                                                    substr($value['record'], 66, 2)." ".
                                                    substr($value['record'], 68, 2).":".
                                                    substr($value['record'], 70, 2).":".
                                                    substr($value['record'], 72, 2);
                        $count++;
                    }      
                }
                
                //支持版本:APS14
                if(!strncmp($value['record'], "APS14", 5))
                {//（当YC1000与500混合时，此方法不行，需要用END分割字符串）   V4版plc版ECU都是APS15开头                
                    //YC1000版本
                    if(!strncmp(substr($value['record'], 98, 2), "04", 2)) {
                        $num = intval(substr($value['record'], 74, 3));
                        for ($i=0; $i<$num; $i++) {
                            $temp = substr($value['record'], 86+180*$i, 180);//每段数据为180个字符
                            $temp = str_replace("A", "0", $temp);//将A替换为0
                            for($ch_i=0; $ch_i<4; $ch_i++) { //4个通道
                                //逆变器ID
                                $data[$count]['inverter_id'] = substr($temp, 0, 12);
                                //通道
                                $data[$count]['channel'] = substr($temp, 27+45*$ch_i, 1);
                                //直流电压                     
                                $data[$count]['dc_voltage'] = number_format(floatval(substr($temp, 14, 5)/10), 1);
                                //直流电流
                                $data[$count]['dc_current'] = number_format(floatval(substr($temp, 28+45*$ch_i, 3)/10), 1);
                                //ECU计算的本轮功率
                                if($ch_i < 3)
                                    $data[$count]['power'] = number_format(floatval(substr($temp, 56+45*$ch_i, 5)/100), 2);
                                else 
                                    $data[$count]['power'] = number_format(floatval(substr($temp, 172, 5)/100), 2);
                                //电网频率
                                $data[$count]['grid_frequency'] = number_format(floatval(substr($temp, 19, 5)/10), 1);
                                //机内温度                        
                                if(!strncmp(substr($temp, 25, 1), "B", 1))
									$data[$count]['temperature'] = "-".intval(substr($temp, 26, 1));
								else if(!strncmp(substr($temp, 24, 1), "B", 1))
                                    $data[$count]['temperature'] = "-".intval(substr($temp, 25, 2));
                                else
                                    $data[$count]['temperature'] = intval(substr($temp, 24, 3));
                                //电网电压
                                if($ch_i < 3)
                                    $data[$count]['grid_voltage'] = intval(substr($temp, 31+45*$ch_i, 3));
                                else
                                    $data[$count]['grid_voltage'] = "-";
                                //本轮累计电量
                                if($ch_i < 3)
                                    $data[$count]['energy'] = number_format(floatval(substr($temp, 50+45*$ch_i, 6)/1000000), 6);
                                else
                                    $data[$count]['energy'] = number_format(floatval(substr($temp, 166, 6)/1000000), 6);
                                //时间
                                $data[$count]['datetime'] = substr($value['record'], 60, 4)."/".
                                    substr($value['record'], 64, 2)."/".
                                    substr($value['record'], 66, 2)." ".
                                    substr($value['record'], 68, 2).":".
                                    substr($value['record'], 70, 2).":".
                                    substr($value['record'], 72, 2);
                                $count++;
                            }
                        }
                    }
                }
                
                //支持版本:APS15
                if(!strncmp($value['record'], "APS15", 5))
                {
                    $num = intval(substr($value['record'], 74, 3));
                    if(!strncmp(substr($value['record'], 98, 1), "A", 1)) {
							$num=$num*2;
                    	$flag_of_fillupdata = 1;
	                    for ($i=0; $i<$num; $i++) {
	                        $temp = substr($value['record'], 86+50*$i, 50);//每段数据为50个字符
	                        $data[$count]['inverter_id'] = ' '.strval(substr($temp, 0, 12));
	                        $data[$count]['channel'] = substr($temp, 12, 1);
	                        $temp = str_replace("A", "0", $temp);//将A替换为0
	                        $data[$count]['dc_voltage'] = number_format(floatval(substr($temp, 13, 5)/10), 1);
	                        $data[$count]['dc_current'] = number_format(floatval(substr($temp, 18, 3)/10), 1);
	                        $data[$count]['power'] = number_format(floatval(substr($temp, 21, 5)/100), 2);
	                        $data[$count]['grid_frequency'] = number_format(floatval(substr($temp, 26, 5)/10), 1);
	                        $data[$count]['temperature'] = intval(substr($temp, 31, 3)) - 100;                    
	                        $data[$count]['grid_voltage'] = intval(substr($temp, 34, 3));
	                        $data[$count]['energy'] = number_format(floatval(substr($temp, 37, 10)/1000000), 6);
	                        $data[$count]['datetime'] = substr($value['record'], 60, 4)."/".
	                            substr($value['record'], 64, 2)."/".
	                            substr($value['record'], 66, 2)." ".
	                            substr($value['record'], 68, 2).":".
	                            substr($value['record'], 70, 2).":".
	                            substr($value['record'], 72, 2);
	                        $count++;
	                    }
                    }
                    if(!strncmp(substr($value['record'], 98, 2), "04", 2)) {
                    	$flag_of_fillupdata = 2;
                        for ($i=0; $i<$num; $i++) {
                            $temp = substr($value['record'], 86+180*$i, 180);//每段数据为180个字符
                            $temp = str_replace("A", "0", $temp);//将A替换为0
                            for($ch_i=0; $ch_i<4; $ch_i++) { //4个通道
                                //逆变器ID
                                $data[$count]['inverter_id'] = substr($temp, 0, 12);
                                //通道
                                $data[$count]['channel'] = substr($temp, 27+45*$ch_i, 1);
                                //直流电压                     
                                $data[$count]['dc_voltage'] = number_format(floatval(substr($temp, 14, 5)/10), 1);
                                //直流电流
                                $data[$count]['dc_current'] = number_format(floatval(substr($temp, 28+45*$ch_i, 3)/10), 1);
                                //ECU计算的本轮功率
                                if($ch_i < 3)
                                    $data[$count]['power'] = number_format(floatval(substr($temp, 56+45*$ch_i, 5)/100), 2);
                                else 
                                    $data[$count]['power'] = number_format(floatval(substr($temp, 172, 5)/100), 2);
                                //电网频率
                                $data[$count]['grid_frequency'] = number_format(floatval(substr($temp, 19, 5)/10), 1);
                                //机内温度                        
                                if(!strncmp(substr($temp, 25, 1), "B", 1))
									$data[$count]['temperature'] = "-".intval(substr($temp, 26, 1));
								else if(!strncmp(substr($temp, 24, 1), "B", 1))
                                    $data[$count]['temperature'] = "-".intval(substr($temp, 25, 2));
                                else
                                    $data[$count]['temperature'] = intval(substr($temp, 24, 3));
                                //电网电压
                                if($ch_i < 3)
                                    $data[$count]['grid_voltage'] = intval(substr($temp, 31+45*$ch_i, 3));
                                else
                                    $data[$count]['grid_voltage'] = "-";
                                //本轮累计电量
                                if($ch_i < 3)
                                    $data[$count]['energy'] = number_format(floatval(substr($temp, 50+45*$ch_i, 6)/1000000), 6);
                                else
                                    $data[$count]['energy'] = number_format(floatval(substr($temp, 166, 6)/1000000), 6);
                                //时间
                                $data[$count]['datetime'] = substr($value['record'], 60, 4)."/".
                                    substr($value['record'], 64, 2)."/".
                                    substr($value['record'], 66, 2)." ".
                                    substr($value['record'], 68, 2).":".
                                    substr($value['record'], 70, 2).":".
                                    substr($value['record'], 72, 2);
                                $count++;
                            }
                        }
                    }
                    
                }
                
                //支持版本:APS18
                if(!strncmp($value['record'], "APS18", 5))
                {	//（当YC1000与500混合时，此方法不行，需要用END分割字符串）                        
                    //YC1000版本
                    $c1=0;$c2=0;
//                    for ($nn=0;$nn<99;$nn++)
//                    {
                    $num = intval(substr($value['record'], 74, 3));
                    for($i=0;$i<num;$i++){
	                    if(!strncmp(substr($value['record'], 98+$c1*180+$c2*88, 2), "04", 2)) {
	                    	$c1++;
                        //for ($i=0; $i<$num; $i++) {
                            $temp = substr($value['record'], 86+$c1*180+$c2*88, 180);//每段数据为180个字符
                            $temp = str_replace("A", "0", $temp);//将A替换为0
                            for($ch_i=0; $ch_i<4; $ch_i++) { //4个通道
                                //逆变器ID
                                $data[$count]['inverter_id'] = substr($temp, 0, 12);
                                //通道
                                $data[$count]['channel'] = substr($temp, 27+45*$ch_i, 1);
                                //直流电压                     
                                $data[$count]['dc_voltage'] = number_format(floatval(substr($temp, 14, 5)/10), 1);
                                //直流电流
                                $data[$count]['dc_current'] = number_format(floatval(substr($temp, 28+45*$ch_i, 3)/10), 1);
                                //ECU计算的本轮功率
                                if($ch_i < 3)
                                    $data[$count]['power'] = number_format(floatval(substr($temp, 56+45*$ch_i, 5)/100), 2);
                                else 
                                    $data[$count]['power'] = number_format(floatval(substr($temp, 172, 5)/100), 2);
                                //电网频率
                                $data[$count]['grid_frequency'] = number_format(floatval(substr($temp, 19, 5)/10), 1);
                                //机内温度                        
                                if(!strncmp(substr($temp, 25, 1), "B", 1))
									$data[$count]['temperature'] = "-".intval(substr($temp, 26, 1)-100);
								else if(!strncmp(substr($temp, 24, 1), "B", 1))
                                    $data[$count]['temperature'] = "-".intval(substr($temp, 25, 2)-100);
                                else
                                    $data[$count]['temperature'] = intval(substr($temp, 24, 3)-100);
                                //电网电压
                                if($ch_i < 3)
                                    $data[$count]['grid_voltage'] = intval(substr($temp, 31+45*$ch_i, 3));
                                else
                                    $data[$count]['grid_voltage'] = "-";
                                //本轮累计电量
                                if($ch_i < 3)
                                    $data[$count]['energy'] = number_format(floatval(substr($temp, 50+45*$ch_i, 6)/1000000), 6);
                                else
                                    $data[$count]['energy'] = number_format(floatval(substr($temp, 166, 6)/1000000), 6);
                                //时间
                                $data[$count]['datetime'] = substr($value['record'], 60, 4)."/".
                                    substr($value['record'], 64, 2)."/".
                                    substr($value['record'], 66, 2)." ".
                                    substr($value['record'], 68, 2).":".
                                    substr($value['record'], 70, 2).":".
                                    substr($value['record'], 72, 2);
                                $count++;
                            }
                        //}
	                    }
	                    if(!strncmp(substr($value['record'], 98+$c1*180+$c2*88, 2), "07", 2)) {
	                	//YC600版本
	                	$c2++;
                	//for ($i=0; $i<$num; $i++) {
                		$temp = substr($value['record'], 86+$c1*180+$c2*88, 88);//每段数据为83个字符
                		for($ch_n=0;$ch_n<2;$ch_n++){
                		//逆变器ID
                		$data[$count]['inverter_id'] = ' '.strval(substr($temp, 0, 12));
                		//逆变器型号
//                		if(strncmp($data[$count]['type'] = substr($temp, 12, 2),"04",2))
                		$temp = str_replace("A", "0", $temp);//将A替换为0,去除填充
                		//电网电压
                		$data[$count]['grid_voltage'] = intval(substr($temp, 14, 3));
                		//电网频率
                		$data[$count]['grid_frequency'] = number_format(floatval(substr($temp, 17, 5)/10), 1);
                		//温度
                		if(!strncmp(substr($temp, 23, 1), "B", 1))
                			$data[$count]['temperature'] = "-".intval(substr($temp, 24, 1)-100);
                		else if(!strncmp(substr($temp, 22, 1), "B", 1))
                			$data[$count]['temperature'] = "-".intval(substr($temp, 23, 2)-100);
                		else	
                			$data[$count]['temperature'] = intval(substr($temp, 22, 3)-100);
                		//无功功率
//                		$data[$count]['reactive_power'] = number_format(floatval(substr($temp, 25, 5)/100), 2);
                		//有功功率
//                		$data[$count]['active_power'] = number_format(floatval(substr($temp, 30, 5)/100), 2);
                		//本轮输出电量
                		$data[$count]['energy'] = number_format(floatval(substr($temp, 35, 10)/1000000), 6);
                		
                		//通道
                		$data[$count]['channel'] = substr($temp, 45+20*$ch_n, 1);     		
                		//直流电压
                		$data[$count]['dc_voltage'] = number_format(floatval(substr($temp, 46+20*$ch_n, 5)/10), 1);
                		//直流电流
                		$data[$count]['dc_current'] = number_format(floatval(substr($temp, 51+20*$ch_n, 3)/10), 1);
                		//本轮累积电量
//                		$data[$count]['current_energy'] = number_format(floatval(substr($temp, 54+20*$ch_n, 6)/1000000), 6);
                		//ECU计算的本轮功率
                		$data[$count]['power'] = number_format(floatval(substr($temp, 60+20*$ch_n, 5)/100), 2);                        
                		$data[$count]['datetime'] = substr($value['record'], 60, 4)."/".
                				substr($value['record'], 64, 2)."/".
                				substr($value['record'], 66, 2)." ".
                				substr($value['record'], 68, 2).":".
                				substr($value['record'], 70, 2).":".
                				substr($value['record'], 72, 2);
                		$count++;
                		}
                	//}
	                  }
                    }
//                 }
                }
            }
        }
        
        if($flag_of_fillupdata>0)
        {
        	$query2 = "SELECT data,lost_date_time FROM fill_up_data WHERE lost_date_time BETWEEN $start AND $end";
        	$result2 = $this->pdo->prepare($query2);
        	if(!empty($result2))
        	{
        		$result2->execute();
        		$res2 = $result2->fetchAll(PDO::FETCH_ASSOC);
        		
        		//限制文件大小
        		foreach($res2 as $k2 => $v2) {$temp2[$k2] = $v2['data'];}
        		//$length = strlen(implode(' ', $temp));
        		if(strlen(implode(' ', $temp2)) > 1000000) {
        			$this->load->view('hidden/export_file_error');
        			return 1;
        		}
        		foreach ($res2 as  $value2)
        		{
        			if($flag_of_fillupdata==1){
	        			$temp = substr($value2['data'], 0, 47);
	        			$data[$count]['inverter_id'] = ' '.strval(substr($temp, 0, 12));
	        			$data[$count]['channel'] = substr($temp, 12, 1);
	        			$data[$count]['dc_voltage'] = number_format(floatval(substr($temp, 13, 5)/10), 1);
	        			$data[$count]['dc_current'] = number_format(floatval(substr($temp, 18, 3)/10), 1);
	        			$data[$count]['power'] = number_format(floatval(substr($temp, 21, 5)/100), 2);
	        			$data[$count]['grid_frequency'] = number_format(floatval(substr($temp, 26, 5)/10), 1);
	        			$data[$count]['temperature'] = intval(substr($temp, 31, 3))-100;
	        			$data[$count]['grid_voltage'] = intval(substr($temp, 34, 3));
	        			$data[$count]['energy'] = number_format(floatval(substr($temp, 37, 10)/1000000), 6);
	        			$data[$count]['datetime'] = substr($value2['lost_date_time'], 0, 4)."/".
	        					substr($value2['lost_date_time'], 4, 2)."/".
	        					substr($value2['lost_date_time'], 6, 2)." ".
	        					substr($value2['lost_date_time'], 8, 2).":".
	        					substr($value2['lost_date_time'], 10, 2).":".
	        					substr($value2['lost_date_time'], 12, 2);	        			 
	        			$count++;  
        			}  
        			if($flag_of_fillupdata>1){
        				$inverter_id = ' '.strval(substr($value2['data'], 0, 12));
        				if(!strncmp("04",substr($value2['data'], 12, 2),2))
        				{
        					$temp = str_replace("A", "0", $value2['data']);//将A替换为0
        					       
        					$dc_voltage = number_format(floatval(substr($temp, 14, 5)/10), 1);
        					$grid_frequency = number_format(floatval(substr($temp, 19, 5)/10), 1);
        					if(!strncmp(substr($temp, 25, 1), "B", 1))
        						$temperature = "-".intval(substr($temp, 26, 1));
        					else if(!strncmp(substr($temp, 24, 1), "B", 1))
        						$temperature = "-".intval(substr($temp, 25, 2));
        					else
        						$temperature = intval(substr($temp, 24, 3));
        					 
        					for($i=0;$i<3;$i++){
        						$data[$count]['inverter_id'] = $inverter_id;
        						$data[$count]['dc_voltage'] = $dc_voltage;
        						$data[$count]['grid_frequency'] = $grid_frequency;
        						$data[$count]['temperature'] = $temperature;
        						$temp = substr($value2['data'], 27+$i*45, 45);
        						$temp = str_replace("A", "0", $temp);//将A替换为0,去除填充
        						$data[$count]['channel'] = substr($temp, 0, 1);
        						$data[$count]['dc_current'] = number_format(floatval(substr($temp, 1, 3)/10), 1);
        						$data[$count]['grid_voltage'] = intval(substr($temp, 4, 3));
        						$data[$count]['power'] = number_format(floatval(substr($temp, 29, 5)/100), 2);
        						$data[$count]['energy'] = number_format(floatval(substr($temp, 23, 6)/1000000), 6);
        						$data[$count]['datetime'] = substr($value2['lost_date_time'], 0, 4)."/".
        								substr($value2['lost_date_time'], 4, 2)."/".
        								substr($value2['lost_date_time'], 6, 2)." ".
        								substr($value2['lost_date_time'], 8, 2).":".
        								substr($value2['lost_date_time'], 10, 2).":".
        								substr($value2['lost_date_time'], 12, 2);
        						$count++;        						
        					}
        					if(1){
        						$data[$count]['inverter_id'] = $inverter_id;
        						$data[$count]['dc_voltage'] = $dc_voltage;
        						$data[$count]['grid_frequency'] = $grid_frequency;
        						$data[$count]['temperature'] = $temperature;
        						$temp = substr($value2['data'], 27+$i*45, 15);
        						$temp = str_replace("A", "0", $temp);//将A替换为0,去除填充
        						$data[$count]['channel'] = substr($temp, 0, 1);
        						$data[$count]['dc_current'] = number_format(floatval(substr($temp, 1, 3)/10), 1);
        						$data[$count]['grid_voltage'] = "-";
        						$data[$count]['power'] = number_format(floatval(substr($temp, 10, 5)/100), 2);
        						$data[$count]['energy'] = number_format(floatval(substr($temp, 4, 6)/1000000), 6);
        						$data[$count]['datetime'] = substr($value2['lost_date_time'], 0, 4)."/".
        								substr($value2['lost_date_time'], 4, 2)."/".
        								substr($value2['lost_date_time'], 6, 2)." ".
        								substr($value2['lost_date_time'], 8, 2).":".
        								substr($value2['lost_date_time'], 10, 2).":".
        								substr($value2['lost_date_time'], 12, 2);
        						$count++;
        					}
        					 
        				}
        			} 			 
        		}
        		
        	}
        	
        }

        /* 导出为csv文件 */
//         header("Content-Type: application/octet-stream");
//         header("Content-Type: text/csv");
//         header("Content-Disposition: attachment; filename=historical_data.csv");
//         header('Cache-Control:must-revalidate,post-check=0,pre-check=0');
//         header('Expires:0');
//         header('Pragma:public');        
//         foreach ($title as $value) {
//             echo $value.",";
//         }
//         echo "\n";
//         foreach ($data as $record) {
//             foreach ($record as $value) {
//                 echo $value.",";
//             }
//             echo "\n";
//         }

        /* 导出为xls文件 */
        header("Content-Type: application/vnd.ms-execl");
        header("Content-Disposition: attachment; filename=historical_data($start~$end).xls");
        header("Pragma: no-cache");
        header("Expires: 0");
        echo "<table border=1>";
        echo "<tr height=50>";
        echo " <td bgcolor=#ff9a00>Inverter ID</td>";  
        echo " <td bgcolor=#ff9a00>Channel</td>"; 
        echo " <td bgcolor=#ff9a00>DC Voltage(V)</td>"; 
        echo " <td bgcolor=#ff9a00>DC Current(A)</td>"; 
        echo " <td bgcolor=#ff9a00>Power(W)</td>";
        echo " <td bgcolor=#ff9a00>Grid Frequency(Hz)</td>";
        echo " <td bgcolor=#ff9a00>Temperature(<sup>o</sup>C)</td>";
        echo " <td bgcolor=#ff9a00>Grid Voltage(V)</td>";
        echo " <td bgcolor=#ff9a00>Energy(kWh)</td>";
        echo " <td bgcolor=#ff9a00>Report Date and Time</td>";
        echo "</tr>";
        foreach ($data as $record) {
            echo "<tr>";
            echo "<td align=center style=\"vnd.ms-excel.numberformat:@\">".$record['inverter_id']."</td>";
            echo "<td align=center>".$record['channel']."</td>";
            echo "<td align=center>".$record['dc_voltage']."</td>";
            echo "<td align=center>".$record['dc_current']."</td>";
            echo "<td align=center>".$record['power']."</td>";
            echo "<td align=center>".$record['grid_frequency']."</td>";
            echo "<td align=center >".$record['temperature']."</td>";
            echo "<td align=center>".$record['grid_voltage']."</td>";
            echo "<td align=center>".$record['energy']."</td>";
            echo "<td align=center style=\"vnd.ms-excel.numberformat:@\">".$record['datetime']."</td>";
            echo "</tr>";
        }        
        echo "</table>";
        //echo $length;
    }

    /* 显示自动更新的服务器的地址和端口 */
    public function get_updatecenter()
    {
        $data = array();
        $data['domain'] = "";
        $data['ip'] = "";
        $data['port'] = "";

        $fp = @fopen("/etc/yuneng/updatecenter.conf", 'r');
        if($fp)
        {
            while(!feof($fp))
            {
                $temp = fgets($fp);
                if(!strncmp($temp, "Domain", 6))
                    $data['domain'] = substr($temp, 7);
                if(!strncmp($temp, "IP", 2))
                    $data['ip'] = substr($temp, 3);
                if(!strncmp($temp, "Port", 4))
                    $data['port'] = substr($temp, 5);
            }
            fclose($fp);
        }

        return $data;
    }

    /* 设置自动更新的服务器的地址和端口 */
    public function set_updatecenter()
    {
        $results = array();

        //获取Updatecenter的信息
        $domain = $this->input->post('domain');
        $ip = $this->input->post('ip');
        $port = $this->input->post('port');
        

        $fp = @fopen("/etc/yuneng/updatecenter.conf", 'w');
        if($fp){
            fwrite($fp, "Domain=".$domain."\n");
            fwrite($fp, "IP=".$ip."\n");
            fwrite($fp, "Port=".$port."\n");
            fclose($fp);
        }
        system("killall autoupdate");
        system("killall single_update");

        $results["value"] = 0;

        return $results;
    }
        

    /* 显示EMA的地址和端口 */
    public function get_datacenter()
    {
        $data = array();

        $data['domain'] = "";
        $data['ip'] = "";
        $data['port1'] = "";
        $data['port2'] = "";

        $fp = @fopen("/etc/yuneng/datacenter.conf", 'r');
        if($fp)
        {
            while(!feof($fp))
            {
                $temp = fgets($fp);
                if(!strncmp($temp, "Domain", 6))
                    $data['domain'] = substr($temp, 7);
                if(!strncmp($temp, "IP", 2))
                    $data['ip'] = substr($temp, 3);
                if(!strncmp($temp, "Port1", 5))
                    $data['port1'] = substr($temp, 6);
                if(!strncmp($temp, "Port2", 5))
                    $data['port2'] = substr($temp, 6);
            }
            fclose($fp);
        }

        return $data;
    }

    /* 设置EMA的地址和端口 */
    public function set_datacenter()
    {
        $results = array();

        //获取Datacenter的信息
        $domain = $this->input->post('domain');
        $ip = $this->input->post('ip');
        $port1 = $this->input->post('port1');
        $port2 = $this->input->post('port2');

        $fp = @fopen("/etc/yuneng/datacenter.conf", 'w');
        if($fp)
        {
            fwrite($fp, "Domain=".$domain."\n");
            fwrite($fp, "IP=".$ip."\n");
            fwrite($fp, "Port1=".$port1."\n");
            fwrite($fp, "Port2=".$port2."\n");
            fclose($fp);
        }

        system("killall client");

        $results["value"] = 0;

        return $results;
    }


    /* 执行初始化操作 */
    public function exec_initialize()
    {
        $result = 0;

        system("killall main.exe");
        if(FALSE === $this->pdo->exec("UPDATE ltpower SET power=0.0 WHERE item =1"))$result = 1;
        if(FALSE === $this->pdo->exec("DELETE FROM tdpower"))$result = 1;
        system("rm -f /home/historical_data.db");       

        $data['result'] = $result;
      
        return $data;
    }

    /* 显示串口信息 */
    public function get_serial()
    {
        //初始化串口信息
        $data['serial_switch'] = "off";
        $data['baud_rate'] = "9600";
        $data['ecu_address'] = "8";
        $fp = @fopen("/etc/yuneng/serial.conf", 'r');
        if($fp)
        {
            $data['serial_switch'] = fgets($fp);
            $data['baud_rate'] = fgets($fp);
            $data['ecu_address'] = fgets($fp);
            fclose($fp);
        }

        return $data;
    }

    /* 设置串口信息 */
    public function set_serial()
    {
        $results = array();

        //获取页面输入的串口信息
        $serial_switch = $this->input->post('serial_switch');
        $baud_rate = $this->input->post('baud_rate');
        $ecu_address = intval($this->input->post('ecu_address'));
        if($ecu_address>0 && $ecu_address<128)
        {
            $fp = @fopen("/etc/yuneng/serial.conf", 'w');
            if($fp)
            {
                fwrite($fp, $serial_switch."\n");
                fwrite($fp, $baud_rate."\n");
                fwrite($fp, $ecu_address);
                fclose($fp);

                $results["value"] = 0;
            }
            else
            {
                $results["value"] = 1;
            }
        }

        return $results;
    }
    
    /* 显示电网环境信息 */
    public function get_grid_environment()
    {
        $data = array();
        
        //若数据表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS grid_environment
            (id VARCHAR(256), result INTEGER, set_value INTEGER, set_flag INTEGER, primary key (id))");
        
        $query = "SELECT id.ID, grid_environment.result 
            FROM id LEFT JOIN grid_environment ON id.ID=grid_environment.id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
        }
        $data['ids'] = $res;
        
        return $data;
    }
    
    /* 设置电网环境 */
    public function set_grid_environment()
    {
        $results = array();
    
        //获取页面输入的电网环境信息
        $id = $this->input->post('id');
        $grid_environment = $this->input->post('grid_environment');
        if($grid_environment == -1){
            //未选择选项
            $results["value"] = 1;
        }
        else{
            if(strlen($id) == 12)
            {
                //设置单个逆变器
                //$this->pdo->exec("REPLACE INTO grid_environment (id, result, set_value, set_flag) VALUES ('$id', 0, $grid_environment, 1)");
                $this->pdo->exec("REPLACE INTO grid_environment (id, result, set_value, set_flag) VALUES ('$id', (SELECT result FROM grid_environment WHERE id='$id'), $grid_environment, 1)");
                $results["value"] = 0;
            }
            else{
                //设置所有逆变器
                $fp = @fopen("/tmp/set_grid_environment.conf", "w");
                if($fp)
                {
                    fwrite($fp, "ALL,".$grid_environment);
                    fclose($fp);
                    $results["value"] = 0;
                }
                else{
                    $results["value"] = 2;
                }                
            }
        }
        return $results;
    }
    
    /* 读取电网环境 */
    public function read_grid_environment()
    {
        $results = array();
    
        $fp = @fopen("/tmp/get_grid_environment.conf", "w");
        if($fp)
        {
            fwrite($fp, "ALL");
            fclose($fp);
            $results["value"] = 0;
        }
        else{
            $results["value"] = 3;
        }
        return $results;
    }
    
    /* 显示IRD控制 */
    public function get_ird()
    {
        $data = array();
    
        //若数据表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS ird
            (id VARCHAR(256), result INTEGER, set_value INTEGER, set_flag INTEGER, primary key (id))");
    
        $query = "SELECT id.ID, ird.result
            FROM id LEFT JOIN ird ON id.ID=ird.id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
        }
        $data['ids'] = $res;
    
        return $data;
    }
    
    /* 设置IRD控制 */
    public function set_ird()
    {
        $results = array();
    
        //获取页面输入的IRD信息
        $id = $this->input->post('id');
        $ird = $this->input->post('ird');
        if($ird == -1){
            //未选择选项
            $results["value"] = 1;
        }
        else{
            if(strlen($id) == 12)
            {
                //设置单个逆变器
                //$this->pdo->exec("REPLACE INTO ird (id, result, set_value, set_flag) VALUES ('$id', 3, $ird, 1)");
                $this->pdo->exec("REPLACE INTO ird (id, result, set_value, set_flag) VALUES ('$id', (SELECT result FROM ird WHERE id='$id'), $ird, 1)");
                $results["value"] = 0;
            }
            else{
                //设置所有逆变器
                $fp = @fopen("/tmp/set_ird.conf", "w");
                if($fp)
                {
                    fwrite($fp, "ALL,".$ird);
                    fclose($fp);
                    $results["value"] = 0;
                }
                else{
                    $results["value"] = 2;
                }
            }
        }
        return $results;
    }
    
    /* 读取IRD控制 */
    public function read_ird()
    {
        $results = array();
          
        $fp = @fopen("/tmp/get_ird.conf", "w");
        if($fp)
        {
            fwrite($fp, "ALL");
            fclose($fp);
            $results["value"] = 0;
        }
        else{
            $results["value"] = 3;
        }           
        return $results;
    }
    
    /* 显示逆变器信号强度 */
    public function get_signal_level()
    {
        $data = array();
    
        //若数据表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS signal_strength
            (id VARCHAR(256), signal_strength INTEGER, set_flag INTEGER, primary key (id))");
    
        $query = "SELECT id.ID, signal_strength.signal_strength
            FROM id LEFT JOIN signal_strength ON id.ID=signal_strength.id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
        }
        $data['ids'] = $res;
    
        return $data;
    }
    
    /* 读取逆变器信号强度 */
    public function read_signal_level()
    {
        $results = array();

        //获取页面输入的IRD信息
        $id = $this->input->post('id');
        
        if(!strncmp($id, "ALL", 3))
        {
            //读取所有逆变器的信号强度
            $fp = @fopen("/tmp/read_all_signal_strength.conf", "w");
            if($fp)
            {
                fwrite($fp, "ALL");
                fclose($fp);
                $results["value"] = 0;
            }
            else{
                $results["value"] = 1;
            }
            
        }
        else if(strlen($id) == 12)
        {
            //读取单个逆变器
            $this->pdo->exec("REPLACE INTO signal_strength (id,  set_flag) VALUES ('$id', 1)");
            $results["value"] = 0;
        }
       
        return $results;
    }
    
    /* 上传文件 */
    public function do_upload()
    {
        $results = array();
        $res_array = array();
        
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
            $results["value"] = 0;
        }

        $results["result"] = implode("\n",$res_array);
        return $results;
     }
     
    /* 读取信道 */
    public function get_channel()
    {
        /* 初始化 */
        $data = array(
            'channel' => 'unknow',
            'old_channel' => 16,
            'new_channel' => 16,
            'ids' => array(),
        );

        /* 读取ECU信道 */
        $fp = @fopen("/etc/yuneng/channel.conf", 'r');
        if ($fp) {
            $data['channel'] = fgets($fp);
            fclose($fp);
        }
         
        /* 读取原信道与新信道 */
        $fp = @fopen("/tmp/old_channel.conf", 'r');
        if ($fp) {
            $data['old_channel'] = fgets($fp);
            fclose($fp);
        }
        $fp = @fopen("/tmp/new_channel.conf", 'r');
        if ($fp) {
            $data['new_channel'] = fgets($fp);
            fclose($fp);
        }

        /*读取逆变器信息*/
        $query = "SELECT id,short_address,bind_zigbee_flag,flag FROM id ";
        $result = $this->pdo->prepare($query);
        if (!empty($result)) {
            $result->execute();
            $data['ids'] = $result->fetchAll(PDO::FETCH_NUM);
        }   
         
        return $data;
    }
     
    /* 设置信道 */
    public function set_channel()
    {
        $results = array();
        $last_channel = 0;
        $old_channel = $this->input->post('old_channel');
        $new_channel = $this->input->post('new_channel');
        $ids = $this->input->post('id');

        //保存原信道
        $fp = @fopen("/tmp/old_channel.conf", 'w');
        if ($fp) {
            fwrite($fp, $old_channel);
            fclose($fp);
        }

        /* 读取上一次修改的信道 */
        $fp = @fopen("/tmp/new_channel.conf", 'r');
        if ($fp) {
            $last_channel = fgets($fp);
            fclose($fp);          
        }
        if ($new_channel != $last_channel) {
            //和上一次修改的信道不同,需要清空绑定标志位
            $this->pdo->exec("UPDATE id SET 
                model=null, 
                software_version=null, 
                bind_zigbee_flag=null, 
                turned_off_rpt_flag=null ");
            $fp = @fopen("/tmp/new_channel.conf", 'w');
            if ($fp) {
                fwrite($fp, $new_channel);
                fclose($fp);
            } 
        }

        //UPDATE Flag
        if ($ids) {
            $times = ceil(count($ids)/500);
            for ($i=0; $i<$times; $i++) {
                $sql = "UPDATE id SET flag=1 WHERE id='0' ";
                foreach (array_slice($ids, $i*500, 500) as $id) {
                    $sql.= "OR id='$id' ";               
                }
                $this->pdo->exec($sql);
            }
        }            
        
        //保存改信道标志
        $fp = @fopen("/tmp/change_channel.conf", 'w');
        if ($fp) {
            fwrite($fp, '1');
            fclose($fp);
        }

        //保存重新组网标志
        $fp = @fopen("/etc/yuneng/limitedid.conf", 'w');
        if($fp){
           fwrite($fp, "1");
           fclose($fp);
        }

        //重启主函数
        exec("killall main.exe");   
        $results["value"] = 0;
        return $results;
    }

    /* 重置信道 */
    public function reset_channel()
    {
        $results = array();

        //保存重置信道标志
        $fp = @fopen("/etc/yuneng/process_reset.conf", 'w');
        if ($fp) {
            fwrite($fp, '1');
            fclose($fp);
        }

        //保存重新组网标志
        $fp = @fopen("/etc/yuneng/limitedid.conf", 'w');
        if($fp){
           fwrite($fp, "1");
           fclose($fp);
        }
        exec("killall main.exe");   //重启主函数

        $results["value"] = 0;
        return $results;
    }
    
    /* 显示DC设置 */
    public function get_dc()
    {
        $data = array();
    
        //若数据表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS dc
            (id VARCHAR(256), result REAL, set_value REAL, set_flag INTEGER, primary key (id))");
    
        $query = "SELECT id.id, dc.result FROM id LEFT JOIN dc ON id.id=dc.id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
        }
        $data['ids'] = $res;
    
        return $data;
    }
    
    /* 设置DC */
    public function set_dc()
    {
        $results = array();
    
        //获取页面输入的IRD信息
        $id = $this->input->post('id');
        $dc = $this->input->post('dc');
        if($dc == -1){
            //未选择选项
            $results["value"] = 1;
        }
        else{
            if(strlen($id) == 12)
            {
                //设置单个逆变器
                //$this->pdo->exec("REPLACE INTO ird (id, result, set_value, set_flag) VALUES ('$id', 3, $ird, 1)");
                $this->pdo->exec("REPLACE INTO dc (id, result, set_value, set_flag) VALUES ('$id', (SELECT result FROM dc WHERE id='$id'), $dc, 1)");
                $results["value"] = 0;
            }
            else{
                //设置所有逆变器
                $fp = @fopen("/tmp/set_ird.conf", "w");
                if($fp)
                {
                    fwrite($fp, "ALL,".$ird);
                    fclose($fp);
                    $results["value"] = 0;
                }
                else{
                    $results["value"] = 2;
                }
            }
        }
        return $results;
    }
    
    /* 读取DC */
    public function read_dc()
    {
        $results = array();
    
        $fp = @fopen("/tmp/get_dc.conf", "w");
        if($fp)
        {
            fwrite($fp, "ALL");
            fclose($fp);
            $results["value"] = 0;
        }
        else{
            $results["value"] = 3;
        }
        return $results;
    }
    
 /* 读取远程更新信息 */
    public function get_remoteupdate()
    {
         //若数据表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS update_inverter
            (id VARCHAR(256), update_result INTEGER, 
            update_time VARCHAR(256), update_flag INTEGER, 
            primary key (id))");

        $data = array();
        $query = "SELECT id.id, id.software_version, update_inverter.update_flag FROM id 
            LEFT JOIN update_inverter ON id.id=update_inverter.id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
        }
else{
$query1 = "SELECT id.id,update_inverter.update_result,update_inverter.update_flag FROM id 
            LEFT JOIN update_inverter ON id.id=update_inverter.id";
 $result1 = $this->pdo->prepare($query1);
 if(!empty($result1))
 {
           $result1->execute();
            	$res1 = $result1->fetchAll();
		$res=$res1;
 }

}
        $data['ids'] = $res;
        
        return $data;
    }
    
    /* 设置远程更新信息 */
    public function set_remoteupdate()
    {
        $results = array();
    
        $ids = $this->input->post('ids'); //获取选中的ID号
	    $scope = $this->input->post('scope'); //获取设置范围(0:设置全部逆变器; 1:设置部分逆变器)
	    if($scope == '0') {
	        //设置所有逆变器
            //$this->pdo->exec("UPDATE id SET set_update_flag=1 ");
	    }
	    else if ($scope == '1') {
	        //设置部分逆变器
	        foreach($ids as $id) {
                //$this->pdo->exec("UPDATE id SET set_update_flag=1 WHERE id='$id' ");
	            $this->pdo->exec("REPLACE into update_inverter (id, update_flag) VALUES ('$id', 1)");
	        }
	    }
	    else 
	        return FALSE;	        

        return TRUE;
    }

     /* 读取隐藏版本号 */
    public function get_version()
    {
        /* 初始化 */
        $data['version'] = "unknow";
         
        $fp = @fopen("/etc/yuneng/version.conf", 'r');
        if ($fp) {
            $data['version'] = fgets($fp);
            $data['version'] = str_replace("\n", "", $data['version']);
            fclose($fp);
        }
        $fp = @fopen("/etc/yuneng/area.conf", 'r');
        if ($fp) {
            $data['version'] = $data['version'].fgets($fp);
            fclose($fp);
        }
        return $data;
    }

     /* 读取GPRS标志位 */
    public function get_gprs()
    {
        //获取GPRS标志位
        $data['gprs'] = 0;
        $fp = @fopen("/etc/yuneng/gprs.conf",'r');
        if ($fp)
        {
          $data['gprs'] = intval(fgets($fp)); 
          fclose($fp);
        }

        return $data;
    }

    /* 设置GPRS */
    public function set_gprs() 
    {        
        $results = array();

        //获取GPRS设置选项
        $gprs = $this->input->post('gprs');
        if($gprs == "true")
            $gprs = "1";
        else
            $gprs = "0";
        $results["gprs"] = $gprs;

        //将标志位写入文件
        $fp = @fopen("/etc/yuneng/gprs.conf",'w');
        if ($fp)
        {
            fwrite($fp, $gprs);
            fclose($fp);
            $results["value"] = 0;
        }
        else{
            $results["value"] = 1;
        }
        //重启GPRS
        system("killall gprs");      
        return $results;
    }
    
}


/* End of file hidden_model.php */
/* Location: ./application/models/hidden_model.php */