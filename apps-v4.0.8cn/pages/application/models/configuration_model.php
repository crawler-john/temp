<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Configuration_model extends CI_Model {

    public $pdo;
    public $pdoen;

    public function __construct() 
    {
        parent::__construct();
        $dsn = 'sqlite:'.APPPATH.'../../../database.db';
        $this->pdo = new PDO($dsn);    
        $dsnen = 'sqlite:'.APPPATH.'../../../encryption.db';
        $this->pdoen = new PDO($dsnen);
    }

    /* 登陆验证 */
    public function check_login() 
    {
        $results = array();

        //获取用户名与密码
        $data["username"] = "admin";
        $data["password"] = "admin";
        $fp=@fopen("/etc/yuneng/userinfo.conf",'r');
        if ($fp)
        {
          $data['username'] = fgets($fp);
          $data['username'] = str_replace("\n", "", $data['username']);
          $data['password'] = fgets($fp);
          $data['password'] = str_replace("\n", "", $data['password']);
          fclose($fp);
        }

        $username = $this->input->post('username');
        $password = $this->input->post('password');
        if ($username == $data['username'] && $password == $data['password']){
            $this->session->set_userdata('logged_in',TRUE);
            $results["value"] = 0;
        }
        else
            $results["value"] = 1;

        return $results;
    }

    /* 获取5项交流保护参数 */
    public function get_protection() 
    {
        //赋初始为NULL
        $ids = array();
        $parameters = array();
        $data = array(
                        'under_voltage_slow' => '',
                        'over_voltage_slow' => '',
                        'under_frequency_slow' => '',
                        'over_frequency_slow' => '',
                        'grid_recovery_time' => ''
                    );

        //获取逆变器ID
        $query = "SELECT id FROM id ORDER BY id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
            foreach ($res as $key => $value) {
                $ids[$key] = $value[0];
            }
        }

        //如果set_protection_parameters表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS set_protection_parameters 
            (parameter_name VARCHAR(256), parameter_value REAL, set_flag INTEGER,
                primary key(parameter_name))");

        //获取5项交流保护参数的值
        $query = "SELECT * FROM set_protection_parameters";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll(PDO::FETCH_ASSOC);
            foreach ($res as $value) {
                if( $value['parameter_name'] == "under_frequency_slow" ||
                    $value['parameter_name'] == "over_frequency_slow" )
                {
                    //保留一位小数
                    $data[$value['parameter_name']] = number_format($value['parameter_value'], 1);
                }
                else if( $value['parameter_name'] == "under_voltage_slow" ||
                         $value['parameter_name'] == "over_voltage_slow" ||
                         $value['parameter_name'] == "grid_recovery_time")
                {
                    //取整
                    $data[$value['parameter_name']] = intval($value['parameter_value']);
                }            
            }
        }

        //获取逆变器交流保护参数的值
        $query = "SELECT id.id, protection_parameters.* FROM id LEFT OUTER JOIN protection_parameters ON (id.ID = protection_parameters.id) ORDER BY ID";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
        
            foreach ($res as $key => $value) {

                //逆变器ID
                $parameters[$key]['inverter_id'] = strval($value[0]);
                
                //内围电压下限
                if(strlen($value['under_voltage_slow']))
                    $parameters[$key]['under_voltage_slow'] = intval($value['under_voltage_slow']);
                else
                    $parameters[$key]['under_voltage_slow'] = "N/A";

                //内围电压上限
                if(strlen($value['over_voltage_slow']))
                    $parameters[$key]['over_voltage_slow'] = intval($value['over_voltage_slow']);
                else
                    $parameters[$key]['over_voltage_slow'] = "N/A";

                //内围频率下限
                if(strlen($value['under_frequency_slow']))
                    $parameters[$key]['under_frequency_slow'] = number_format($value['under_frequency_slow'], 1);
                else
                    $parameters[$key]['under_frequency_slow'] = "N/A";

                //内围频率上限
                if(strlen($value['over_frequency_slow']))
                    $parameters[$key]['over_frequency_slow'] = number_format($value['over_frequency_slow'], 1);
                else
                    $parameters[$key]['over_frequency_slow'] = "N/A";

                //并网恢复时间
                 if(strlen($value['grid_recovery_time']))
                    $parameters[$key]['grid_recovery_time'] = intval($value['grid_recovery_time']);
                else
                    $parameters[$key]['grid_recovery_time'] = "N/A";
            }
        }

        $data['ids'] = $ids;
        $data['parameters'] = $parameters;

        return $data;
    }

    /* 设置5项交流保护参数 */
    public function set_protection()
    {
        $results = array();

        //获取设置参数
        $parameters = array();
        $inverter = $this->input->post('inverter');
        if(strlen($this->input->post('under_voltage_slow')))
            $parameters['under_voltage_slow'] = intval($this->input->post('under_voltage_slow'));
        if(strlen($this->input->post('over_voltage_slow')))
            $parameters['over_voltage_slow'] = intval($this->input->post('over_voltage_slow'));
        if(strlen($this->input->post('under_frequency_slow')))
            $parameters['under_frequency_slow'] = number_format($this->input->post('under_frequency_slow'), 1);
        if(strlen($this->input->post('over_frequency_slow')))
            $parameters['over_frequency_slow'] = number_format($this->input->post('over_frequency_slow'), 1);
        if(strlen($this->input->post('grid_recovery_time')))
            $parameters['grid_recovery_time'] = intval($this->input->post('grid_recovery_time'));

        if(empty($parameters))
        {
            //输入保护参数全为空
            $results["value"] = 1;
        }
        else
        {
            //创建表单
            $this->pdo->exec("CREATE TABLE IF NOT EXISTS set_protection_parameters_inverter 
                (id VARCHAR(256), parameter_name VARCHAR(256), parameter_value REAL, set_flag INTEGER,
                primary key(id, parameter_name))");


            //将设置参数存入数据库
            if(!strncmp($inverter, "all", 3))
            {
                //设置所有逆变器
                foreach ($parameters as $key => $value) {
                    $this->pdo->exec("REPLACE INTO set_protection_parameters
                     (parameter_name, parameter_value, set_flag) VALUES 
                     ('$key', $value, 1)");
                }            
            }
            else
            {
                //设置单台逆变器
                foreach ($parameters as $key => $value) {
                    $this->pdo->exec("REPLACE INTO set_protection_parameters_inverter 
                     (id, parameter_name, parameter_value, set_flag) VALUES 
                     ('$inverter', '$key', $value, 1)");
                }
            }

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
            $record = "APS1300124A120AAA0".$ecuid."00000000000000END"
                     ."AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEND"
                     ."\n";
            
            //查询保护参数
            $query = "SELECT * FROM set_protection_parameters";
            $result = $this->pdo->prepare($query);
            if(!empty($result))
            {
                $result->execute();
                $res = $result->fetchAll(PDO::FETCH_ASSOC);
                foreach ($res as $value) 
                {
                    //外围电压下限
                    if($value['parameter_name'] == "under_voltage_fast")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 97, 3);
                    }
                    //外围电压上限
                    if($value['parameter_name'] == "over_voltage_fast")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 100, 3);
                    }
                    //内围电压下限
                    if($value['parameter_name'] == "under_voltage_slow")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 47, 3);
                    }
                    //内围电压上限
                    if($value['parameter_name'] == "over_voltage_slow")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 50, 3);
                    }
                    //外围频率下限
                    if($value['parameter_name'] == "under_frequency_fast")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 103, 3);
                    }
                    //外围频率上限
                    if($value['parameter_name'] == "over_frequency_fast")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 106, 3);
                    }
                    //内围频率下限
                    if($value['parameter_name'] == "under_frequency_slow")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 53, 3);
                    }
                    //内围频率上限
                    if($value['parameter_name'] == "over_frequency_slow")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 56, 3);
                    }
                    //外围过欠压延迟保护时间
                    if($value['parameter_name'] == "voltage_triptime_fast")
                    {
                        $parameter_value = sprintf("%02d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 109, 2);
                    }
                    //内围过欠压延迟保护时间
                    if($value['parameter_name'] == "voltage_triptime_slow")
                    {
                        $parameter_value = sprintf("%04d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 111, 4);
                    }
                    //外围过欠频延迟保护时间
                    if($value['parameter_name'] == "frequency_triptime_fast")
                    {
                        $parameter_value = sprintf("%02d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 115, 2);
                    }
                    //内围过欠频延迟保护时间
                    if($value['parameter_name'] == "frequency_triptime_slow")
                    {
                        $parameter_value = sprintf("%04d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 117, 4);
                    }
                    //并网恢复时间
                    if($value['parameter_name'] == "grid_recovery_time")
                    {
                        $parameter_value = sprintf("%05d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 59, 5);
                    }
                }
            }

            //将消息保存到数据库
            $sql = "REPLACE INTO process_result (item, result, flag) VALUES(120, '$record', 1)";
            $this->pdo->exec($sql);

            $results["value"] = 0;
        }
        return $results;
    }

    /* 读取逆变器交流保护参数 */
    public function read_inverter_parameters()
    {
        $results = array();

        //将读取命令写入配置文件
        $fp = @fopen("/tmp/presetdata.conf", 'w');
        if (!$fp) {
            $results["value"] = 1;//文件打开失败
            return $results;
        }
        fwrite($fp, "2");
        fclose($fp);
        $results["value"] = 0;
        
        return $results;
    }

    /* 获取13项交流保护参数 */
    public function get_protection2() 
    {
        //赋初值为NULL
        $ids = array();
        $parameters = array();
        $data = array(
                        'under_voltage_fast' => '',
                        'over_voltage_fast' => '',
                        'under_voltage_slow' => '',
                        'over_voltage_slow' => '',
                        'under_frequency_fast' => '',
                        'over_frequency_fast' => '',
                        'under_frequency_slow' => '',
                        'over_frequency_slow' => '',
                        'voltage_triptime_fast' => '',
                        'voltage_triptime_slow' => '',
                        'frequency_triptime_fast' => '',
                        'frequency_triptime_slow' => '',
                        'grid_recovery_time' => ''
                    );

        //获取逆变器ID
        $query = "SELECT id FROM id ORDER BY id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
            foreach ($res as $key => $value) {
                $ids[$key] = $value[0];
            }
        }

        //如果set_protection_parameters表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS set_protection_parameters 
            (parameter_name VARCHAR(256), parameter_value REAL, set_flag INTEGER,
                primary key(parameter_name))");

        //获取13项交流保护参数的值
        $query = "SELECT * FROM set_protection_parameters";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll(PDO::FETCH_ASSOC);
            foreach ($res as $value) {
                if( $value['parameter_name'] == "under_frequency_fast" || 
                    $value['parameter_name'] == "over_frequency_fast"  || 
                    $value['parameter_name'] == "under_frequency_slow" ||
                    $value['parameter_name'] == "over_frequency_slow" )
                {
                    //保留一位小数
                    $data[$value['parameter_name']] = number_format($value['parameter_value'], 1);
                }
                else if( $value['parameter_name'] == "voltage_triptime_fast" || 
                         $value['parameter_name'] == "voltage_triptime_slow"  || 
                         $value['parameter_name'] == "frequency_triptime_fast" ||
                         $value['parameter_name'] == "frequency_triptime_slow" )
                {
                    //保留两位小数
                    $data[$value['parameter_name']] = number_format($value['parameter_value'], 2);
                }
                else
                {
                    //取整
                    $data[$value['parameter_name']] = intval($value['parameter_value']);
                }            
            }
        }

        //获取逆变器交流保护参数的值
        $query = "SELECT id.id, protection_parameters.* FROM id LEFT OUTER JOIN protection_parameters ON (id.ID = protection_parameters.id) ORDER BY ID";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
            //print_r($res);
            foreach ($res as $key => $value) 
            {
                //逆变器ID
                $parameters[$key]['inverter_id'] = strval($value[0]);
                
                //外围电压下限
                if(strlen($value['under_voltage_fast']))
                    $parameters[$key]['under_voltage_fast'] = intval($value['under_voltage_fast']);
                else
                    $parameters[$key]['under_voltage_fast'] = "N/A";
                
                //外围电压上限
                if(strlen($value['over_voltage_fast']))
                    $parameters[$key]['over_voltage_fast'] = intval($value['over_voltage_fast']);
                else
                    $parameters[$key]['over_voltage_fast'] = "N/A";

                //内围电压下限
                if(strlen($value['under_voltage_slow']))
                    $parameters[$key]['under_voltage_slow'] = intval($value['under_voltage_slow']);
                else
                    $parameters[$key]['under_voltage_slow'] = "N/A";

                //内围电压上限
                if(strlen($value['over_voltage_slow']))
                    $parameters[$key]['over_voltage_slow'] = intval($value['over_voltage_slow']);
                else
                    $parameters[$key]['over_voltage_slow'] = "N/A";

                //外围频率下限
                if(strlen($value['under_frequency_fast']))
                    $parameters[$key]['under_frequency_fast'] = number_format($value['under_frequency_fast'], 1);
                else
                    $parameters[$key]['under_frequency_fast'] = "N/A";

                //外围频率上限
                if(strlen($value['over_frequency_fast']))
                    $parameters[$key]['over_frequency_fast'] = number_format($value['over_frequency_fast'], 1);
                else
                    $parameters[$key]['over_frequency_fast'] = "N/A";

                //内围频率下限
                if(strlen($value['under_frequency_slow']))
                    $parameters[$key]['under_frequency_slow'] = number_format($value['under_frequency_slow'], 1);
                else
                    $parameters[$key]['under_frequency_slow'] = "N/A";

                //内围频率上限
                if(strlen($value['over_frequency_slow']))
                    $parameters[$key]['over_frequency_slow'] = number_format($value['over_frequency_slow'], 1);
                else
                    $parameters[$key]['over_frequency_slow'] = "N/A";

                //外围过欠压延迟保护时间
                if(strlen($value['voltage_triptime_fast']))
                    $parameters[$key]['voltage_triptime_fast'] = number_format($value['voltage_triptime_fast'], 2);
                else
                    $parameters[$key]['voltage_triptime_fast'] = "N/A";

                //内围过欠压延迟保护时间
                if(strlen($value['voltage_triptime_slow']))
                    $parameters[$key]['voltage_triptime_slow'] = number_format($value['voltage_triptime_slow'], 2);
                else
                    $parameters[$key]['voltage_triptime_slow'] = "N/A";

                //外围过欠频延迟保护时间
                if(strlen($value['frequency_triptime_fast']))
                    $parameters[$key]['frequency_triptime_fast'] = number_format($value['frequency_triptime_fast'], 2);
                else
                    $parameters[$key]['frequency_triptime_fast'] = "N/A";

                //内围过欠频延迟保护时间
                if(strlen($value['frequency_triptime_slow']))
                    $parameters[$key]['frequency_triptime_slow'] = number_format($value['frequency_triptime_slow'], 2);
                else
                    $parameters[$key]['frequency_triptime_slow'] = "N/A";

                //并网恢复时间
                 if(strlen($value['grid_recovery_time']))
                    $parameters[$key]['grid_recovery_time'] = intval($value['grid_recovery_time']);
                else
                    $parameters[$key]['grid_recovery_time'] = "N/A";
            }
        }

        $data['ids'] = $ids;
        $data['parameters'] = $parameters;

        return $data;
    }

    /* 设置13项交流保护参数 */
    public function set_protection2()
    {
        $results = array();

        //获取设置参数
        $inverter = $this->input->post('inverter');
        if(strlen($this->input->post('under_voltage_fast')))
            $parameters['under_voltage_fast'] = intval($this->input->post('under_voltage_fast'));
        if(strlen($this->input->post('over_voltage_fast')))
            $parameters['over_voltage_fast'] = intval($this->input->post('over_voltage_fast'));
        if(strlen($this->input->post('under_voltage_slow')))
            $parameters['under_voltage_slow'] = intval($this->input->post('under_voltage_slow'));
        if(strlen($this->input->post('over_voltage_slow')))
            $parameters['over_voltage_slow'] = intval($this->input->post('over_voltage_slow'));
        if(strlen($this->input->post('under_frequency_fast')))
            $parameters['under_frequency_fast'] = number_format($this->input->post('under_frequency_fast'), 1);
        if(strlen($this->input->post('over_frequency_fast')))
            $parameters['over_frequency_fast'] = number_format($this->input->post('over_frequency_fast'), 1);
        if(strlen($this->input->post('under_frequency_slow')))
            $parameters['under_frequency_slow'] = number_format($this->input->post('under_frequency_slow'), 1);
        if(strlen($this->input->post('over_frequency_slow')))
            $parameters['over_frequency_slow'] = number_format($this->input->post('over_frequency_slow'), 1);
        if(strlen($this->input->post('voltage_triptime_fast')))
            $parameters['voltage_triptime_fast'] = number_format($this->input->post('voltage_triptime_fast'), 2);
        if(strlen($this->input->post('voltage_triptime_slow')))
            $parameters['voltage_triptime_slow'] = number_format($this->input->post('voltage_triptime_slow'), 2);
        if(strlen($this->input->post('frequency_triptime_fast')))
            $parameters['frequency_triptime_fast'] = number_format($this->input->post('frequency_triptime_fast'), 2);
        if(strlen($this->input->post('frequency_triptime_slow')))
            $parameters['frequency_triptime_slow'] = number_format($this->input->post('frequency_triptime_slow'), 2);
        if(strlen($this->input->post('grid_recovery_time')))
            $parameters['grid_recovery_time'] = intval($this->input->post('grid_recovery_time'));

         if(empty($parameters))
        {
            //输入保护参数全为空
            $results["value"] = 1;
        }
        else
        {
            //创建表单
            $this->pdo->exec("CREATE TABLE IF NOT EXISTS set_protection_parameters_inverter 
                (id VARCHAR(256), parameter_name VARCHAR(256), parameter_value REAL, set_flag INTEGER,
                primary key(id, parameter_name))");

            //将设置参数存入数据库
            if(!strncmp($inverter, "all", 3))
            {
                //设置所有逆变器
                foreach ($parameters as $key => $value) {
                    $this->pdo->exec("REPLACE INTO set_protection_parameters
                     (parameter_name, parameter_value, set_flag) VALUES 
                     ('$key', $value, 1)");
                }            
            }
            else
            {
                //设置单台逆变器
                foreach ($parameters as $key => $value) {
                    $this->pdo->exec("REPLACE INTO set_protection_parameters_inverter 
                     (id, parameter_name, parameter_value, set_flag) VALUES 
                     ('$inverter', '$key', $value, 1)");
                }
            }

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
            $record = "APS1300124A120AAA0".$ecuid."00000000000000END"
                     ."AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEND"
                     ."\n";
            
            //查询保护参数
            $query = "SELECT * FROM set_protection_parameters";
            $result = $this->pdo->prepare($query);
            if(!empty($result))
            {
                $result->execute();
                $res = $result->fetchAll(PDO::FETCH_ASSOC);
                foreach ($res as $value) 
                {
                    //外围电压下限
                    if($value['parameter_name'] == "under_voltage_fast")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 97, 3);
                    }
                    //外围电压上限
                    if($value['parameter_name'] == "over_voltage_fast")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 100, 3);
                    }
                    //内围电压下限
                    if($value['parameter_name'] == "under_voltage_slow")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 47, 3);
                    }
                    //内围电压上限
                    if($value['parameter_name'] == "over_voltage_slow")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 50, 3);
                    }
                    //外围频率下限
                    if($value['parameter_name'] == "under_frequency_fast")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 103, 3);
                    }
                    //外围频率上限
                    if($value['parameter_name'] == "over_frequency_fast")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 106, 3);
                    }
                    //内围频率下限
                    if($value['parameter_name'] == "under_frequency_slow")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 53, 3);
                    }
                    //内围频率上限
                    if($value['parameter_name'] == "over_frequency_slow")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 56, 3);
                    }
                    //外围过欠压延迟保护时间
                    if($value['parameter_name'] == "voltage_triptime_fast")
                    {
                        $parameter_value = sprintf("%02d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 109, 2);
                    }
                    //内围过欠压延迟保护时间
                    if($value['parameter_name'] == "voltage_triptime_slow")
                    {
                        $parameter_value = sprintf("%04d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 111, 4);
                    }
                    //外围过欠频延迟保护时间
                    if($value['parameter_name'] == "frequency_triptime_fast")
                    {
                        $parameter_value = sprintf("%02d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 115, 2);
                    }
                    //内围过欠频延迟保护时间
                    if($value['parameter_name'] == "frequency_triptime_slow")
                    {
                        $parameter_value = sprintf("%04d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 117, 4);
                    }
                    //并网恢复时间
                    if($value['parameter_name'] == "grid_recovery_time")
                    {
                        $parameter_value = sprintf("%05d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 59, 5);
                    }
                }
            }

            //将消息保存到数据库
            $sql = "REPLACE INTO process_result (item, result, flag) VALUES(120, '$record', 1)";
            $this->pdo->exec($sql);

           $results["value"] = 0;
        }

        return $results;
    }
    
    /* 获取17项交流保护参数 */
    public function get_protection17()
    {
        //赋初值为NULL
        $ids = array();
        $parameters = array();
        $data = array(
            'under_voltage_fast' => '',
            'over_voltage_fast' => '',
            'under_voltage_slow' => '',
            'over_voltage_slow' => '',
            'under_frequency_fast' => '',
            'over_frequency_fast' => '',
            'under_frequency_slow' => '',
            'over_frequency_slow' => '',
            'voltage_triptime_fast' => '',
            'voltage_triptime_slow' => '',
            'frequency_triptime_fast' => '',
            'frequency_triptime_slow' => '',
            'grid_recovery_time' => '',
            'regulated_dc_working_point' => '',
            'under_voltage_stage_2' => '',
            'voltage_3_clearance_time' => '',
            'start_time' => ''
        );
    
        //获取逆变器ID
        $query = "SELECT id FROM id ORDER BY id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
            foreach ($res as $key => $value) {
                $ids[$key] = $value[0];
            }
        }
    
        //如果set_protection_parameters表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS set_protection_parameters
            (parameter_name VARCHAR(256), parameter_value REAL, set_flag INTEGER,
                primary key(parameter_name))");
    
        //获取17项交流保护参数的值
        $query = "SELECT * FROM set_protection_parameters";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll(PDO::FETCH_ASSOC);
            foreach ($res as $value) {
                if( $value['parameter_name'] == "under_frequency_fast" ||
                    $value['parameter_name'] == "over_frequency_fast"  ||
                    $value['parameter_name'] == "under_frequency_slow" ||
                    $value['parameter_name'] == "over_frequency_slow" ||
                    $value['parameter_name'] == "regulated_dc_working_point" )
                {
                    //保留一位小数
                    $data[$value['parameter_name']] = number_format($value['parameter_value'], 1);
                }
                else if( $value['parameter_name'] == "voltage_triptime_fast" ||
                    $value['parameter_name'] == "voltage_triptime_slow"  ||
                    $value['parameter_name'] == "frequency_triptime_fast" ||
                    $value['parameter_name'] == "frequency_triptime_slow" ||
                    $value['parameter_name'] == "voltage_3_clearance_time" )
                {
                    //保留两位小数
                    $data[$value['parameter_name']] = number_format($value['parameter_value'], 2);
                }
                else
                {
                    //取整
                    $data[$value['parameter_name']] = intval($value['parameter_value']);
                }
            }
        }
    
        //获取逆变器交流保护参数的值
        $query = "SELECT id.id, protection_parameters.* FROM id LEFT OUTER JOIN protection_parameters ON (id.ID = protection_parameters.id) ORDER BY ID";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
            foreach ($res as $key => $value)
            {
                //逆变器ID
                $parameters[$key]['inverter_id'] = strval($value[0]);
    
                //外围电压下限
                if(isset($value['under_voltage_fast']))
                    $parameters[$key]['under_voltage_fast'] = intval($value['under_voltage_fast']);
                else
                    $parameters[$key]['under_voltage_fast'] = "N/A";
    
                //外围电压上限
                if(isset($value['over_voltage_fast']))
                    $parameters[$key]['over_voltage_fast'] = intval($value['over_voltage_fast']);
                else
                    $parameters[$key]['over_voltage_fast'] = "N/A";
    
                //内内围电压下限
                if(isset($value['under_voltage_slow']))
                    $parameters[$key]['under_voltage_slow'] = intval($value['under_voltage_slow']);
                else
                    $parameters[$key]['under_voltage_slow'] = "N/A";
    
                //内围电压上限
                if(isset($value['over_voltage_slow']))
                    $parameters[$key]['over_voltage_slow'] = intval($value['over_voltage_slow']);
                else
                    $parameters[$key]['over_voltage_slow'] = "N/A";
    
                //外围频率下限
                if(isset($value['under_frequency_fast']))
                    $parameters[$key]['under_frequency_fast'] = number_format($value['under_frequency_fast'], 1);
                else
                    $parameters[$key]['under_frequency_fast'] = "N/A";
    
                //外围频率上限
                if(isset($value['over_frequency_fast']))
                    $parameters[$key]['over_frequency_fast'] = number_format($value['over_frequency_fast'], 1);
                else
                    $parameters[$key]['over_frequency_fast'] = "N/A";
    
                //内围频率下限
                if(isset($value['under_frequency_slow']))
                    $parameters[$key]['under_frequency_slow'] = number_format($value['under_frequency_slow'], 1);
                else
                    $parameters[$key]['under_frequency_slow'] = "N/A";
    
                //内围频率上限
                if(isset($value['over_frequency_slow']))
                    $parameters[$key]['over_frequency_slow'] = number_format($value['over_frequency_slow'], 1);
                else
                    $parameters[$key]['over_frequency_slow'] = "N/A";
    
                //外围过欠压延迟保护时间
                if(isset($value['voltage_triptime_fast']))
                    $parameters[$key]['voltage_triptime_fast'] = number_format($value['voltage_triptime_fast'], 2);
                else
                    $parameters[$key]['voltage_triptime_fast'] = "N/A";
    
                //内围过欠压延迟保护时间
                if(isset($value['voltage_triptime_slow']))
                    $parameters[$key]['voltage_triptime_slow'] = number_format($value['voltage_triptime_slow'], 2);
                else
                    $parameters[$key]['voltage_triptime_slow'] = "N/A";
    
                //外围过欠频延迟保护时间
                if(isset($value['frequency_triptime_fast']))
                    $parameters[$key]['frequency_triptime_fast'] = number_format($value['frequency_triptime_fast'], 2);
                else
                    $parameters[$key]['frequency_triptime_fast'] = "N/A";
    
                //内围过欠频延迟保护时间
                if(isset($value['frequency_triptime_slow']))
                    $parameters[$key]['frequency_triptime_slow'] = number_format($value['frequency_triptime_slow'], 2);
                else
                    $parameters[$key]['frequency_triptime_slow'] = "N/A";
    
                //并网恢复时间
                if(isset($value['grid_recovery_time']))
                    $parameters[$key]['grid_recovery_time'] = intval($value['grid_recovery_time']);
                else
                    $parameters[$key]['grid_recovery_time'] = "N/A";
                
                //直流稳压设置
                if(isset($value['regulated_dc_working_point']))
                    $parameters[$key]['regulated_dc_working_point'] = number_format($value['regulated_dc_working_point'], 1);
                else
                    $parameters[$key]['regulated_dc_working_point'] = "N/A";
                
                //内围电压设置
                if(isset($value['under_voltage_stage_2']))
                    $parameters[$key]['under_voltage_stage_2'] = intval($value['under_voltage_stage_2']);
                else
                    $parameters[$key]['under_voltage_stage_2'] = "N/A";
                
                //内内围延迟保护时间
                if(isset($value['voltage_3_clearance_time']))
                    $parameters[$key]['voltage_3_clearance_time'] = number_format($value['voltage_3_clearance_time'], 2);
                else
                    $parameters[$key]['voltage_3_clearance_time'] = "N/A";
                
                //直流启动时间
                if(isset($value['start_time']))
                    $parameters[$key]['start_time'] = intval($value['start_time']);
                else
                    $parameters[$key]['start_time'] = "N/A";
            }
        }
    
        $data['ids'] = $ids;
        $data['parameters'] = $parameters;
    
        return $data;
    }
    
    /* 设置17项交流保护参数 */
    public function set_protection17()
    {
        $results = array();
    
        //获取设置参数
        $inverter = $this->input->post('inverter');
        if(strlen($this->input->post('under_voltage_fast')))
            $parameters['under_voltage_fast'] = intval($this->input->post('under_voltage_fast'));
        if(strlen($this->input->post('over_voltage_fast')))
            $parameters['over_voltage_fast'] = intval($this->input->post('over_voltage_fast'));
        if(strlen($this->input->post('under_voltage_slow')))
            $parameters['under_voltage_slow'] = intval($this->input->post('under_voltage_slow'));
        if(strlen($this->input->post('over_voltage_slow')))
            $parameters['over_voltage_slow'] = intval($this->input->post('over_voltage_slow'));
        if(strlen($this->input->post('under_frequency_fast')))
            $parameters['under_frequency_fast'] = number_format($this->input->post('under_frequency_fast'), 1);
        if(strlen($this->input->post('over_frequency_fast')))
            $parameters['over_frequency_fast'] = number_format($this->input->post('over_frequency_fast'), 1);
        if(strlen($this->input->post('under_frequency_slow')))
            $parameters['under_frequency_slow'] = number_format($this->input->post('under_frequency_slow'), 1);
        if(strlen($this->input->post('over_frequency_slow')))
            $parameters['over_frequency_slow'] = number_format($this->input->post('over_frequency_slow'), 1);
        if(strlen($this->input->post('voltage_triptime_fast')))
            $parameters['voltage_triptime_fast'] = number_format($this->input->post('voltage_triptime_fast'), 2);
        if(strlen($this->input->post('voltage_triptime_slow')))
            $parameters['voltage_triptime_slow'] = number_format($this->input->post('voltage_triptime_slow'), 2);
        if(strlen($this->input->post('frequency_triptime_fast')))
            $parameters['frequency_triptime_fast'] = number_format($this->input->post('frequency_triptime_fast'), 2);
        if(strlen($this->input->post('frequency_triptime_slow')))
            $parameters['frequency_triptime_slow'] = number_format($this->input->post('frequency_triptime_slow'), 2);
        if(strlen($this->input->post('grid_recovery_time')))
            $parameters['grid_recovery_time'] = intval($this->input->post('grid_recovery_time'));
        if(strlen($this->input->post('regulated_dc_working_point')))
            $parameters['regulated_dc_working_point'] = number_format($this->input->post('regulated_dc_working_point'), 1);
        if(strlen($this->input->post('under_voltage_stage_2')))
            $parameters['under_voltage_stage_2'] = intval($this->input->post('under_voltage_stage_2'));
        if(strlen($this->input->post('voltage_3_clearance_time')))
            $parameters['voltage_3_clearance_time'] = number_format($this->input->post('voltage_3_clearance_time'), 2);
        if(strlen($this->input->post('start_time')))
            $parameters['start_time'] = intval($this->input->post('start_time'));
        if(empty($parameters))
        {
            //输入保护参数全为空
            $results["value"] = 1;
        }
        else
        {
            //创建表单
            $this->pdo->exec("CREATE TABLE IF NOT EXISTS set_protection_parameters_inverter
                (id VARCHAR(256), parameter_name VARCHAR(256), parameter_value REAL, set_flag INTEGER,
                primary key(id, parameter_name))");
    
            //将设置参数存入数据库
            if(!strncmp($inverter, "all", 3))
            {
                //设置所有逆变器
                foreach ($parameters as $key => $value) {
                    $this->pdo->exec("REPLACE INTO set_protection_parameters
                        (parameter_name, parameter_value, set_flag) VALUES
                        ('$key', $value, 1)");
                }
            }
            else
            {
                //设置单台逆变器
                foreach ($parameters as $key => $value) {
                    $this->pdo->exec("REPLACE INTO set_protection_parameters_inverter
                        (id, parameter_name, parameter_value, set_flag) VALUES
                        ('$inverter', '$key', $value, 1)");
                }
            }
    
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
            $record = "APS1300153A130AAA0".$ecuid."00000000000000END"
                         ."AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                         ."AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                         ."END\n";    
            //查询保护参数
            $query = "SELECT * FROM set_protection_parameters";
            $result = $this->pdo->prepare($query);
            if (!empty($result)) 
            {
                $result->execute();
                $res = $result->fetchAll(PDO::FETCH_ASSOC);
                foreach ($res as $value)
                {
                    //外围电压下限
                    if($value['parameter_name'] == "under_voltage_fast")
                    {
                    $parameter_value = sprintf("%03d", $value['parameter_value']);
                    $record = substr_replace($record, $parameter_value, 97, 3);
                    }
                    //外围电压上限
                    if($value['parameter_name'] == "over_voltage_fast")
                    {
                    $parameter_value = sprintf("%03d", $value['parameter_value']);
                    $record = substr_replace($record, $parameter_value, 100, 3);
                    }
                    //内内围电压下限
                    if($value['parameter_name'] == "under_voltage_slow")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 47, 3);
                    }
                    //内围电压上限
                    if($value['parameter_name'] == "over_voltage_slow")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 50, 3);
                    }
                    //外围频率下限
                    if($value['parameter_name'] == "under_frequency_fast")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 103, 3);
                    }
                    //外围频率上限
                    if($value['parameter_name'] == "over_frequency_fast")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 106, 3);
                    }
                    //内围频率下限
                    if($value['parameter_name'] == "under_frequency_slow")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 53, 3);
                    }
                    //内围频率上限
                    if($value['parameter_name'] == "over_frequency_slow")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 56, 3);
                    }
                    //外围过欠压延迟保护时间
                    if($value['parameter_name'] == "voltage_triptime_fast")
                    {
                        $parameter_value = sprintf("%06d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 109, 6);
                    }
                    //内围过欠压延迟保护时间
                    if($value['parameter_name'] == "voltage_triptime_slow")
                    {
                        $parameter_value = sprintf("%06d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 115, 6);
                    }
                    //外围过欠频延迟保护时间
                    if($value['parameter_name'] == "frequency_triptime_fast")
                    {
                        $parameter_value = sprintf("%06d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 121, 6);
                    }
                    //内围过欠频延迟保护时间
                    if($value['parameter_name'] == "frequency_triptime_slow")
                    {
                        $parameter_value = sprintf("%06d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 127, 6);
                    }
                    //并网恢复时间
                    if($value['parameter_name'] == "grid_recovery_time")
                    {
                        $parameter_value = sprintf("%05d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 59, 5);
                    }
                    //直流稳压设置
                    if($value['parameter_name'] == "regulated_dc_working_point")
                    {
                        $parameter_value = sprintf("%03d", (int)(10*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 133, 3);
                    }
                    //内围电压设置
                    if($value['parameter_name'] == "under_voltage_stage_2")
                    {
                        $parameter_value = sprintf("%03d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 136, 3);
                    }
                    //内内围延迟保护时间
                    if($value['parameter_name'] == "voltage_3_clearance_time")
                    {
                        $parameter_value = sprintf("%06d", (int)(100*floatval($value['parameter_value'])));
                        $record = substr_replace($record, $parameter_value, 139, 6);
                    }
                    //直流启动时间
                    if($value['parameter_name'] == "start_time")
                    {
                        $parameter_value = sprintf("%05d", $value['parameter_value']);
                        $record = substr_replace($record, $parameter_value, 145, 5);
                    }
                }
             }
    
            //将消息保存到数据库
            $sql = "REPLACE INTO process_result (item, result, flag) VALUES(130, '$record', 1)";
            $this->pdo->exec($sql);
            $results["value"] = 0;
        }    
        return $results;
    }

    /* 获取逆变器GFDI状态 */
    public function get_gfdi_state() 
    {      
        //获取逆变器ID
        $ids = array();
        $gfdi_state = array();
        $query = "SELECT id,flag FROM id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
            foreach ($res as $key => $value) {
                $ids[$key] = $value[0];
            }
        }

        //初始化为"-"
        for ($i=0; $i<count($ids); $i++) 
        { 
            $gfdi_state[$i] = "-";
        }     

        //获取每台逆变器的GFDI状态
        $fp = @fopen("/tmp/gfdiresult.txt", 'r');
        if($fp)
        {            
            for ($i=0; $i<count($ids); $i++) 
            { 
                $temp = fgets($fp);
                if(strlen($temp))
                    $gfdi_state[$i] = $temp;
                else
                    $gfdi_state[$i] = "-";
            }
            fclose($fp);
        }

        //赋值
        $data['ids'] = $ids;
        $data['gfdi_state'] = $gfdi_state;

        return $data;
    }

    /* 设置逆变器GFDI状态 */
    public function set_gfdi_state() 
    {
        $results = array();
        
        //获取需要清除锁定状态的逆变器号
        $ids = array();
        $ids = $this->input->post('ids');

        //若数据表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS clear_gfdi 
            (id VARCHAR(256), set_flag INTEGER, primary key (id))");

        if(!empty($ids))
        {
            foreach ($ids as $value) 
            {
                $this->pdo->exec("REPLACE INTO clear_gfdi (id, set_flag) VALUES ('$value', 1)");
            }
            $results["value"] = 0;
        }
        else
        {
            //没有逆变器被选中
            $results["value"] = 1;
        }
        return $results;
    }

    /* 获取逆变器开关机状态 */
    public function get_switch_state() 
    {
        //获取逆变器ID
        $ids = array();
        $switch_state = array();
        $query = "SELECT id,flag FROM id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
            foreach ($res as $key => $value) {
                $ids[$key] = $value[0];
            }
        }

        //初始化为"-"
        for ($i=0; $i<count($ids); $i++) 
        { 
            $switch_state[$i] = "-";
        }       

        //获取每台逆变器的开关机状态
        $fp = @fopen("/tmp/connectresult.txt", 'r');
        if($fp)
        {            
            for ($i=0; $i<count($ids); $i++) 
            { 
                $temp = fgets($fp);
                if(strlen($temp))
                    $switch_state[$i] = $temp;
                else
                    $switch_state[$i] = "-";
            }
            fclose($fp);
        }

        //赋值
        $data['ids'] = $ids;
        $data['switch_state'] = $switch_state;

        return $data;
    }

    /* 设置逆变器开关机状态 */
    public function set_switch_state() 
    {
        $results = array();

        //获取需要开关机的逆变器ID号
        $ids = array();
        $ids = $this->input->post('ids');

        //若数据表不存在，则创建
        $this->pdo->exec("CREATE TABLE IF NOT EXISTS turn_on_off 
            (id VARCHAR(256), set_flag INTEGER, primary key (id))");

        if(!empty($ids))
        {
            foreach ($ids as $value) 
            {
                $inverter = substr($value, 0, 12);
                $flag = substr($value, 12, 1);
                $this->pdo->exec("REPLACE INTO turn_on_off (id, set_flag) VALUES ('$inverter', $flag)");
            }

            $results["value"] = 0;
        }
        else
        {
            //没有逆变器被选中
            $results["value"] = 1;
        }

        return $results;
    }

    /* 设置所有逆变器为开机状态 */
    public function set_switch_all_on()
    { 
        $results = array();
//         //若数据表不存在，则创建
//         $this->pdo->exec("CREATE TABLE IF NOT EXISTS turn_on_off 
//             (id VARCHAR(256), set_flag INTEGER, primary key (id))");

//         $query = "SELECT id FROM id";
//         $result = $this->pdo->prepare($query);
//         if(!empty($result))
//         {
//             $result->execute();
//             $res = $result->fetchAll();

//             foreach ($res as $key => $value) {
//                 $this->pdo->exec("REPLACE INTO turn_on_off (id, set_flag) VALUES ('$value[0]', 1)");
//             }
//         }
        $fp = @fopen("/tmp/connect.conf", 'w');
        if($fp){
            fwrite($fp, "connect all");
            fclose($fp);
            $results["value"] = 0;
        }
        else {
            $results["value"] = 2;
        }       

        return $results;
    }

    /* 设置所有逆变器为关机状态 */
    public function set_switch_all_off()
    {
        $results = array();
//         //若数据表不存在，则创建
//         $this->pdo->exec("CREATE TABLE IF NOT EXISTS turn_on_off 
//             (id VARCHAR(256), set_flag INTEGER, primary key (id))");

//         $query = "SELECT id FROM id";
//         $result = $this->pdo->prepare($query);
//         if(!empty($result))
//         {
//             $result->execute();
//             $res = $result->fetchAll();

//             foreach ($res as $key => $value) {
//                 $this->pdo->exec("REPLACE INTO turn_on_off (id, set_flag) VALUES ('$value[0]', 2)");
//             }
//         }
        $fp = @fopen("/tmp/connect.conf", 'w');
        if($fp){
            fwrite($fp, "disconnect all");
            fclose($fp);
            $results["value"] = 0;
        }
        else {
            $results["value"] = 2;
        }   

        return $results;
    }

    /* 获取逆变器最大功率 */
    public function get_maxpower() 
    {
        $data = array();

        $query = "SELECT power.id, power.limitedpower, power.limitedresult 
            FROM id LEFT JOIN power ON id.ID=power.id";
        $result = $this->pdo->prepare($query);
        if(!empty($result))
        {
            $result->execute();
            $res = $result->fetchAll();
        }
        $data['ids'] = $res;

        return $data;
    }

    /* 设置逆变器最大功率 */
    public function set_maxpower() 
    {
        $results = array();

        //获取页面输入的最大功率
        $maxpower = intval($this->input->post('maxpower'));
        $id = $this->input->post('id');
        if ($maxpower < 20 || $maxpower >300) 
        {
            //超出范围
            $results["value"] = 1;
        }
        else
        {   
            $this->pdo->exec("UPDATE power SET limitedpower=$maxpower,flag=1 WHERE id=\"$id\"");
            $results["value"] = 0;
        }

        return $results;
    }
    
    /* 读取逆变器最大功率 */
    public function read_maxpower()
    {
        $results = array();
    
        $fp = @fopen("/tmp/getmaxpower.conf", "w");
        if($fp)
        {
            fwrite($fp, "ALL");
            fclose($fp);
            $results["value"] = 0;
        }
        else{
            $results["value"] = 2;
        }
        return $results;
    }
    
//     /* 获取用户信息 */
//     public function get_user_info()
//     {
//         $data = array();
//         return $data;
//     }
    
    /* 设置用户信息 */
    public function set_user_info()
    {
        $results =array();
    
        //系统默认用户名密码
        $data['username'] = "admin";
        $data['password'] = "admin";
        $fp = @fopen("/etc/yuneng/userinfo.conf",'r');
        if ($fp)
        {
            $data['username'] = fgets($fp);
            $data['username'] = str_replace("\n", "", $data['username']);
            $data['password'] = fgets($fp);
            $data['password'] = str_replace("\n", "", $data['password']);
            fclose($fp);
        }
    
        //获取页面输入用户名密码
        $username = $this->input->post('username');
        $old_password =  $this->input->post('old_password');
        $new_password = $this->input->post('new_password');
        $confirm_password = $this->input->post('confirm_password');
        $new_username = $this->input->post('new_username');
    
        //判断原密码是否正确并修改密码
        if($data['username'] == $username && $data['password'] == $old_password)
        {
            if(!strcmp($new_password, $confirm_password))
            {
                if(strlen($new_password))
                {
                    $fp = @fopen("/etc/yuneng/userinfo.conf",'w');
                    if($fp){
                        //若新用户名不为空，则保存新用户名
                        if(strlen($new_username)) {
                            fwrite($fp, $new_username."\n".$new_password);
                        }
                        else {
                            fwrite($fp, $username."\n".$new_password);
                        }
                        fclose($fp);
                    }
                    $results["value"] = 0;
                    //成功修改密码后需要重新登录
                    $this->session->set_userdata('logged_in',FALSE);
                }
                else
                {
                    //新输入密码为空
                    $results["value"] = 2;
                }
            }
            else
            {
                //两次输入的密码不相同
                $results["value"] = 3;
            }
        }
        else
        {
            //用户名密码错误
            $results["value"] = 1;
        }
        return $results;
    }
    
    /* 验证是否加密 */
    public function check_encryption()
    {
    	$data = array();
    
    	$this->pdoen->exec("CREATE TABLE IF NOT EXISTS key
            (item INTEGER,key VARCHAR(8), operator INTEGER, cmd INTEGER,set_flag INTEGER,read_flag INTEGER,set_time_flag INTEGER,timeout INTEGER,PRIMARY KEY(item))");
//    	$query = "SELECT * FROM key WHERE key=null and item=1";
    	$query = "SELECT item FROM key";
    	$result = $this->pdoen->prepare($query);
    	if(!empty($result)) { 
    		$result->execute();
    		$res = $result->fetchAll(); 
//    		foreach ($res as $key => $value) {
//    			$pwd[$key] = $value[3];
//    		}
    			if(!count($res))	
    				$data['flagen'] = 1;  	
    			else 
    			{
    				$data['flagen'] = 0;  
    				$query = "SELECT timeout FROM key";
    				$result = $this->pdoen->prepare($query);
    				if(!empty($result))
    				{
    					$result->execute();
    					$res = $result->fetchAll();
    					foreach ($res as $value) {
    						$data['mod_time'] = $value[0];
    					}
    				}
    				else 
    					$data['mod_time'] = 240;
    			} 		
    	}
    	return $data;
    }
    
    /* 加密设置 */
    public function encrypt()
    {
    	$results = array();
    	
    		if(strlen($this->input->post('password')))
    		$password = $this->input->post('password');
    		
    		if(!empty($password)){
    		//若数据表不存在，则创建
    		$this->pdoen->exec("CREATE TABLE IF NOT EXISTS key
            (item INTEGER,key VARCHAR(8), operator INTEGER, cmd INTEGER,set_flag INTEGER,read_flag INTEGER,set_time_flag INTEGER,timeout INTEGER,PRIMARY KEY(item)");
    		
    		$this->pdoen->exec("REPLACE INTO key(item,key,operator,cmd,set_flag) VALUES (1,'$password',0,1,1)");
    		
    			$results["value"] = 0;
    		}
    		else 
    			$results["value"] = 1;
    	return $results;
    }
    
    /* 加密时间设置 */
    public function modi_time()
    {
    	$results = array();

    	if(strlen($this->input->post('cipher')))
    		$cipher = $this->input->post('cipher');
    	if(strlen($this->input->post('mod_time')))
    		$mod_time = $this->input->post('mod_time');
		if(empty($cipher)||empty($mod_time))
			$results["value"] = 2;
		else{
    	$query = "SELECT key FROM key";
    	$result = $this->pdoen->prepare($query);
    	if(!empty($result)) {
    	            $result->execute();
            $res = $result->fetchAll();
            foreach ($res as $value) {
                $key = $value[0];
            }
			if($cipher == $key){
    			$this->pdoen->exec("update key set timeout=$mod_time,set_time_flag=1 where item=1");    		
    			$results["value"] = 0;
			}
			else
				$results["value"] = 1;
    	}
    	else
    		$results["value"] = 1;
		}
		return $results;
    }

    /* 获取心跳包时间 */
    public function heartbeat()
    {
    	$results = array();
    	$fp = @fopen("/etc/yuneng/heart_interval_time.txt", "r");
    	if($fp)
    	{
    		$temp = fgets($fp);
    			if(strlen($temp))
    				$results['heartbeat'] = $temp;
    			else
    				$results['heartbeat'] = "300";    		
    		fclose($fp);
    	} 
    	else
    		$results['heartbeat'] = "300";
    	return $results;
    }
    /* 心跳包时间设置 */
    public function set_heartbeat()
    {
    	$results = array();
    
    	if(strlen($this->input->post('heartbeat')))
    		$heartbeat = $this->input->post('heartbeat');
    	if(!empty($heartbeat))
			{
					$fp = @fopen("/etc/yuneng/heart_interval_time.txt", "w");
					fwrite($fp,$heartbeat);
					fclose($fp);
					system('killall main.exe');
    				$results["value"] = 0;
    		}    
    	else 
    		$results["value"] = 1;
    	return $results;
    }
    
    
    
}

/* End of file configuration_model.php */
/* Location: ./application/models/configuration_model.php */