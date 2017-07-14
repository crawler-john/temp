<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Home_model extends CI_Model {

    public $pdo;

    public function __construct() 
    {
        parent::__construct();
        $dsn = 'sqlite:'.APPPATH.'../../../database.db';
        $this->pdo = new PDO($dsn);
    }

    public function get_data() 
    {
        /* 初始化数组 */
        $data = array(
            'ecuid' => '',
            'lifetimepower' => '0',
            'systemp' => '0',
            'todaypower' => '0',
            'datetime' => 'Never connected',
            'maxnum' => '0',
            'curnum' => '0',
            'version' => '',
            'timezone' => 'Asia/Shanghai',
            'eth0_mac' => '',
            'wlan0_mac' => '',
            'grid_quality' => '',
            'week_energy' => ''
        );
        
        /* 查询ECU_ID */
        $fp = @fopen("/etc/yuneng/ecuid.conf", 'r');
        if ($fp) {
            $data['ecuid'] = fgets($fp);
            fclose($fp);
        }

        /* 查询历史发电量 */
        $query = "select power from ltpower";
        $result = $this->pdo->query($query);
        $res = $result->fetch(PDO::FETCH_ASSOC);
        $data['lifetimepower'] = number_format($res['power'], 2);

        /* 查询最近一次系统功率 */
        $fp = @fopen("/tmp/system_p_display.conf", 'r');
        if ($fp) {
             $data['systemp'] = fgets($fp);
             fclose($fp);
        }
        
        //获取ECU当前设置的时区
        $fp = @fopen("/etc/yuneng/timezone.conf", 'r');
        if ($fp) {
            $data['timezone'] = fgets($fp);
            fclose($fp);
        }
        
        /* 查询系统当天累计发电量 */
        date_default_timezone_set($data['timezone']);
        $localtime_assoc = localtime(time(), true);
        $date = sprintf("%04d%02d%02d",
            $localtime_assoc['tm_year'] + 1900,
            $localtime_assoc['tm_mon'] + 1,
            $localtime_assoc['tm_mday']
        );
        $query = "select * from tdpower";
        $result = $this->pdo->query($query);
        $res = $result->fetch(PDO::FETCH_ASSOC); 
        if (!@strcmp($date, $res['date'])) {
            $data['todaypower'] = number_format($res['todaypower'], 2);
        }

        /* 查询最近一次连接服务器时间 */
        $fp = @fopen("/etc/yuneng/connect_time.conf", 'r');
        if ($fp) {
            $data['datetime'] = fgets($fp);
            fclose($fp);       
        }        

        /* 查询逆变器总台数 */
        $query = "select ID from id";
        $result = $this->pdo->query($query);  
        $res = $result->fetchAll();
        $data['maxnum'] = count($res);

        /* 查询最近一次逆变器连接台数 */
        $fp = @fopen("/tmp/current_number.conf", 'r');
        if ($fp) {
            $data['curnum'] = fgets($fp);
            fclose($fp);
        }

        /* 查询软件版本号(软件版本号+地区) */
        $fp = @fopen("/etc/yuneng/version.conf", 'r');
        if ($fp) {
            $data['version'] = fgets($fp);
            $offset = strpos($data['version'], '.', strpos($data['version'], '.') + 1);
            if ($offset) {
                $data['version'] = substr($data['version'], 0, $offset);
            }            
            $data['version'] = str_replace("\n", "", $data['version']);
            fclose($fp);
        }
        $fp = @fopen("/etc/yuneng/area.conf", 'r');
        if ($fp) {
            $data['version'] = $data['version'].fgets($fp);
            fclose($fp);
        }

        /* 查询ECU_有线网络Mac地址 */
        $fp = @fopen("/etc/yuneng/ecu_eth0_mac.conf", 'r');
        if ($fp) {
            $data['eth0_mac'] = fgets($fp);
            fclose($fp);
        }
        
        /* 查询ECU_无线网络Mac地址 */
        $fp = @fopen("/etc/yuneng/ecu_wlan0_mac.conf", 'r');
        if ($fp) {
            $data['wlan0_mac'] = fgets($fp);
            fclose($fp);
        }
        
        /* 查询逆变器通信信号强度 */
        $fp = @fopen("/etc/yuneng/plc_grid_quality.txt", 'r');
        if ($fp) {
            $data['grid_quality'] = fgets($fp);
            fclose($fp);
        }
        
        /* 查询一周能量(用于环保效益) */
        $dsn = 'sqlite:'.APPPATH.'../../../historical_data.db';
        $this->pdo = new PDO($dsn);
        $date_start = date("Ymd",time() - 518400);
        $date_end = date("Ymd",time());
        $query = "SELECT SUM(daily_energy) FROM daily_energy WHERE date BETWEEN $date_start AND $date_end";
        $result = $this->pdo->query($query);
        if(!empty($result)) {
            $res = $result->fetch(PDO::FETCH_NUM);
            $data['week_energy'] = $res[0];
        }
        
        return $data;
    }

}

/* End of file home_model.php */
/* Location: ./application/models/home_model.php */