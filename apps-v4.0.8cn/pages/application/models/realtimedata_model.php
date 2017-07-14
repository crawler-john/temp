<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Realtimedata_model extends CI_Model {

    public $pdo;

    public function __construct() 
    {
        parent::__construct();
        $dsn = 'sqlite:'.APPPATH.'../../../database.db';
        $this->pdo = new PDO($dsn);          
    }

    public function get_data()
    {
        $realdata = array();
        $curdata = array();
        $num = 0;
        $fp = @fopen("/tmp/parameters.conf",'r');
        if ($fp)
        {
            while(!feof($fp))
            {
                $record = fgets($fp);
                if(strlen($record) > 1)
                {
                    sscanf($record, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",
                        $curdata[$num][0], 
                        $curdata[$num][1], 
                        $curdata[$num][2], 
                        $curdata[$num][3], 
                        $curdata[$num][4], 
                        $curdata[$num][5]);
                    $num++;
                }
            }
            fclose($fp);
        }
        $num = -1;
        $temp = '';
        foreach ($curdata as $key => $value) {            
            if (strncmp($temp, $value[0], 12)) {
                $temp = $value[0];
                $num++;
            }
            $realdata[$num][0][] = $value[0];
            $realdata[$num][1][] = strlen(trim($value[1]))? $value[1].' W' : '-';
            $realdata[$num][2]   = trim($value[2])? $value[2].' Hz' : '-';
            $realdata[$num][3][] = (strlen(trim($value[3])) && strcmp('-', trim($value[3])))? $value[3].' V' : '-';
            $realdata[$num][4]   = strlen(trim($value[4]))? $value[4].' &#176;C' : '-';
            $realdata[$num][5]   = $value[5];
        }
        $data['curdata'] = $realdata;
        return $data;
    }

    //获取当前功率数据
    public function get_power_graph() 
    {
        /* 初始化 */
        $data = array(
            'power' => array(),
            'today_energy' => '0',
            'subtitle' => 'No Data',
        );

        //读取当前时区
        $timezone = "Asia/Shanghai";
        $fp = @fopen("/etc/yuneng/timezone.conf",'r');
        if ($fp){
            $timezone = fgets($fp);
            fclose($fp);
        }
        date_default_timezone_set($timezone);
        
        //获取当前日期
        $date = $this->input->post('date');
        if(!$date){$date = date("Y-m-d",time());}
        sscanf($date, "%d-%d-%d", $year, $month, $day);
        $query_date = sprintf("%04d%02d%02d", $year, $month, $day);
        
        //获取数据
        $dsn = 'sqlite:'.APPPATH.'../../../historical_data.db';
        $this->pdo = new PDO($dsn);
        $query = "SELECT time,each_system_power FROM each_system_power WHERE date=$query_date";
        $result = $this->pdo->query($query);
        if(empty($result))return $data;//没有数据库或SQL语句错误
        $res = $result->fetchAll(PDO::FETCH_ASSOC);
        if(count($res))$data['subtitle'] = ''; //存在数据
        foreach ($res as $key => $value) {
            $data['power'][$key]['time'] = strtotime($query_date.$value['time'])*1000;
            $data['power'][$key]['each_system_power'] = intval($value['each_system_power']);
        }
        
        $query = "SELECT daily_energy FROM daily_energy WHERE date=$query_date";
        $result = $this->pdo->query($query);
        if(empty($result))return $data;//没有数据库或SQL语句错误
        $res = $result->fetch(PDO::FETCH_ASSOC);
        if(!empty($res))
            $data['today_energy'] = number_format(floatval($res['daily_energy']), 2); //保留两位小数

        return $data;
    }

    //获取发电能量
    public function get_energy_graph() 
    {
        /* 初始化 */
        $data = array(
            'energy' => array(),
            'total_energy' => '0',
            'subtitle' => 'No Data',
        );

        //读取当前时区
        $timezone = "Asia/Shanghai";
        $fp = @fopen("/etc/yuneng/timezone.conf",'r');
        if ($fp){
            $timezone = fgets($fp);
            fclose($fp);
        }
        date_default_timezone_set($timezone);

        //获取选择日期（默认为今天）
        $date = $this->input->post('date');
        if(!$date){$date = date("Y-m-d",time());}

        //获取周期（默认为一周）
        $period = $this->input->post('period');
        if(!$period){$period = "weekly";}

        //按周期获取数据
        if(!strncmp($period, "weekly", 6))
        {
            //计算起止时间
            $data_start = date("Y-m-d",strtotime($date)-3600*24*6)."\n";
            sscanf($data_start, "%d-%d-%d", $year, $month, $day);
            $query_date_start = sprintf("%04d%02d%02d", $year, $month, $day);//开始时间
            sscanf($date, "%d-%d-%d", $year, $month, $day);
            $query_date_end = sprintf("%04d%02d%02d", $year, $month, $day);//结束时间

            //获取数据
            $dsn = 'sqlite:'.APPPATH.'../../../historical_data.db';
            $this->pdo = new PDO($dsn);
            $query = "SELECT date,daily_energy FROM daily_energy 
                WHERE date BETWEEN $query_date_start AND $query_date_end ORDER BY date";
            $result = $this->pdo->query($query);
            if(empty($result))return $data;//没有数据库或SQL语句错误            
            $res = $result->fetchAll(PDO::FETCH_ASSOC);
            
            //格式化数据
            $flag_start = 0;
            $flag_end = count($res);
            if($flag_end){ //存在数据
                $data['subtitle'] = '';                 
                for ($i=6; $i >= 0; $i--){ //初始化数据(先赋值为0)
                    $data['energy'][6-$i]['date'] = date("Y-m-d", strtotime($date) - $i*3600*24);
                    sscanf($data['energy'][6-$i]['date'], "%d-%d-%d", $year, $month, $day);
                    $data['energy'][6-$i]['date'] = sprintf("%04d%02d%02d", $year, $month, $day);
                    $data['energy'][6-$i]['energy'] = 0;
                }            
                foreach ($data['energy'] as $key => $value){
                    for ($i=$flag_start; $i<$flag_end ; $i++){
                        if(!strncmp($value['date'], $res[$i]['date'], 8)){
                            $data['energy'][$key]['energy'] = floatval(number_format($res[$i]['daily_energy'], 2));
                            $flag_start++;
                            break;
                        }
                    }
                    sscanf($data['energy'][$key]['date'], "%04d%02d%02d", $year, $month, $day);
                    $data['energy'][$key]['date'] = sprintf("%02d/%02d", $month, $day);
                }
            }
        }
        else if(!strncmp($period, "monthly", 7))
        {
            //计算起止时间
            $data_start = date("Y-m-d",strtotime($date)-3600*24*29)."\n";
            sscanf($data_start, "%d-%d-%d", $year, $month, $day);
            $query_date_start = sprintf("%04d%02d%02d", $year, $month, $day);
            sscanf($date, "%d-%d-%d", $year, $month, $day);
            $query_date_end = sprintf("%04d%02d%02d", $year, $month, $day);

            //获取数据
            $dsn = 'sqlite:'.APPPATH.'../../../historical_data.db';
            $this->pdo = new PDO($dsn);
            $query = "SELECT date,daily_energy FROM daily_energy 
                WHERE date BETWEEN $query_date_start AND $query_date_end  ORDER BY date";
            $result = $this->pdo->query($query);
            if(empty($result))return $data;//没有数据库或SQL语句错误
            $res = $result->fetchAll(PDO::FETCH_ASSOC);
            
            //格式化数据
            $flag_start = 0;
            $flag_end = count($res);
            if($flag_end){ //存在数据
                $data['subtitle'] = '';
                for ($i=29; $i >= 0; $i--){
                    $data['energy'][29-$i]['date'] = date("Y-m-d", strtotime($date)-$i*3600*24);
                    sscanf($data['energy'][29-$i]['date'], "%d-%d-%d", $year, $month, $day);
                    $data['energy'][29-$i]['date'] = sprintf("%02d%02d", $month, $day);
                    $data['energy'][29-$i]['energy'] = 0;
                }      
                foreach ($data['energy'] as $key => $value){
                    for ($i=$flag_start; $i<$flag_end ; $i++){
                        if(!strncmp($value['date'], substr($res[$i]['date'], 4), 4)){
                            $data['energy'][$key]['energy'] = floatval(number_format($res[$i]['daily_energy'], 2));
                            $flag_start++;
                            break;
                        }
                    }
                    sscanf($data['energy'][$key]['date'], "%02d%02d", $month, $day);
                    $data['energy'][$key]['date'] = sprintf("%02d/%02d", $month, $day);
                }
            }
        }
        else if(!strncmp($period, "yearly", 6))
        {
            //计算起止月份
            sscanf($date, "%d-%d-%d", $year, $month, $day);
            $query_date_end = sprintf("%04d%02d", $year, $month);
            if($month == 12)
                $query_date_start = sprintf("%04d01", $year);
            else
                $query_date_start = sprintf("%04d%02d", $year-1, $month+1);

            //获取数据
            $dsn = 'sqlite:'.APPPATH.'../../../historical_data.db';
            $this->pdo = new PDO($dsn);
            $query = "SELECT date,monthly_energy FROM monthly_energy 
                WHERE date BETWEEN $query_date_start AND $query_date_end  ORDER BY date";           
            $result = $this->pdo->query($query);
            if(empty($result))return $data;//没有数据库或SQL语句错误
            $res = $result->fetchAll(PDO::FETCH_ASSOC);
            
            //格式化数据
            $flag_start = 0;
            $flag_end = count($res);
            if($flag_end){ //存在数据
                $data['subtitle'] = '';
                for ($i=0; $i < 12; $i++){
                    if($month == 0){
                       $year--;
                       $month = 12;
                    }                                  
                    $data['energy'][$i]['date'] = sprintf("%04d%02d", $year, $month--);
                    $data['energy'][$i]['energy'] = 0;
                }
                $data['energy'] = array_reverse($data['energy']);//将数组倒序                
                
                foreach ($data['energy'] as $key => $value){
                    for ($i=$flag_start; $i < $flag_end; $i++){
                        if(!strncmp($value['date'], $res[$i]['date'], 6))
                            {
                            $data['energy'][$key]['energy'] = floatval(number_format($res[$i]['monthly_energy'], 2));
                            $flag_start++;
                            break;
                        }
                    }
                    sscanf($data['energy'][$key]['date'], "%04d%02d", $year, $month);
                    $data['energy'][$key]['date'] = sprintf("%04d/%02d", $year, $month);
                }
            }
        }

        //求一个周期内的总发电量
        foreach ($data['energy'] as $key => $value) {
              $data['total_energy'] = $data['total_energy'] + $value['energy'];
        }
        return $data;
    }

    //获取逆变器工作状态
    public function get_inverter_status()
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
}


/* End of file realtimedata_model.php */
/* Location: ./application/models/realtimedata_model.php */