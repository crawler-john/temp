<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Management extends CI_Controller {

    public $page = "management";
    public $nav = array(
        'id' => array(
            'url' => 'index.php/management',
            'active' => '0',
            'name' => 'id'
        ),
        'datetime' => array(
            'url' => 'index.php/management/datetime',
            'active' => '0',
            'name' => 'time'
        ),
        'language' => array(
            'url' => 'index.php/management/language',
            'active' => '0',
            'name' => 'language'
        ),
        'network' => array(
            'url' => 'index.php/management/network',
            'active' => '0',
            'name' => 'network'
        ),
        'wlan' => array(
            'url' => 'index.php/management/wlan',
            'active' => '0',
            'name' => 'wlan'
        ),
        'upgrade_ecu' => array(
            'url' => 'index.php/management/upgrade_ecu',
            'active' => '0',
            'name' => 'upgrade_ecu'
        )
    );

    public function __construct()
    {
        parent::__construct();

        $this->load->library('session');
        $this->load->helper('url');
        $this->load->helper('form');    
        $this->load->model('management_model');

        /* 设置系统语言 */
        $language = "english";
        $fp = @fopen("/etc/yuneng/language.conf",'r');
        if($fp)
        {
            $language = fgets($fp);
            fclose($fp);
        }
        else if($this->session->userdata("language"))
        {
            $language = $this->session->userdata("language");
        }
        //加载页面显示语言文件
        $this->lang->load('page', $language);
        //加载验证信息语言文件
        $this->lang->load('validform', $language);
    }

    /* 显示逆变器列表(默认函数) */
    public function index()
    {      
        $data['page'] = $this->page;
        $data['func'] = "id";
        $this->nav['id']['active'] = '1';
        $data['nav'] = $this->nav;
        
        $result = $this->management_model->get_id();
        $data = array_merge($data, $result);

        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('management/id');
        $this->load->view('templates/footer');
    }

	/* 设置逆变器列表 */
    public function set_id()
    { 
        $results = $this->management_model->set_id();
        $results["message"] = $this->lang->line("set_id_result_{$results["value"]}");
        $results["num"] = $this->lang->line("set_id_result_num").": ".$results["num"];
        echo json_encode($results);
    }

    /* 清空逆变器列表 */
    public function set_id_clear()
    { 
        $results = $this->management_model->set_id_clear();
        $results["message"] = $this->lang->line("clear_id_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示时间时区信息 */
    public function datetime()
    {  
        $data['page'] = $this->page;
        $data['func'] = "time";
        $this->nav['datetime']['active'] = '1';
        $data['nav'] = $this->nav;
        
        $result = $this->management_model->get_datetime();
        $data = array_merge($data, $result);
        
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('management/datetime');
        $this->load->view('templates/footer');
    }

    /* 设置日期时间 */
    public function set_datetime()
    { 
        $results = $this->management_model->set_datetime();
        $results["message"] = $this->lang->line("datetime_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 设置时区 */
    public function set_timezone()
    {  
        $results = $this->management_model->set_timezone(); 
        $results["message"] = $this->lang->line("timezone_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 设置NTP服务器 */
    public function set_ntp_server()
    {  
        $results = $this->management_model->set_ntp_server();
        $results["message"] = $this->lang->line("ntp_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示语言信息 */
    public function language()
    {      
        $data['page'] = $this->page;
        $data['func'] = "language";
        $this->nav['language']['active'] = '1';
        $data['nav'] = $this->nav;
        
        $result = $this->management_model->get_language();
        $data = array_merge($data, $result);
        
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('management/language');
        $this->load->view('templates/footer');
    }

    /* 设置语言 */
    public function set_language()
    {      
        $results = $this->management_model->set_language();
        $results["message"] = $this->lang->line("language_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示网络配置信息 */
    public function network()
    {      
        $data['page'] = $this->page;
        $data['func'] = "network";
        $this->nav['network']['active'] = '1';
        $data['nav'] = $this->nav;
        
        $result = $this->management_model->get_network();
        $data = array_merge($data, $result);
        
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('management/network');
        $this->load->view('templates/footer');
    }

    /* 设置IP */
    public function set_ip()
    {
        $results = $this->management_model->set_ip();
        echo json_encode($results);
    }

    /* 显示无线局域网 */
    public function wlan()
    {      
        $data['page'] = $this->page;
        $data['func'] = "wlan";
        $this->nav['wlan']['active'] = '1';
        $data['nav'] = $this->nav;
        
        $result = $this->management_model->get_wlan();
        $data = array_merge($data, $result);
        
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('management/wlan');
        $this->load->view('templates/footer');
    }

    /* 获取WLAN信息 */
    public function get_wlan_info()
    {      
        $results = $this->management_model->get_wlan_info();
        echo json_encode($results);
    }

    /* 设置主机模式参数 */
    public function set_wlan_ap()
    {
        $results = $this->management_model->set_wlan_ap();
        echo json_encode($results);
    }

    /* 设置从机模式参数 */
    public function set_wlan_sta()
    {      
        $results = $this->management_model->set_wlan_sta();
        $results["msg"] = $this->lang->line("wlan_result_{$results["msg"]}");
        echo json_encode($results);
    }
    
    /* 重启ECU */
    public function ecu_reboot()
    {
        $this->load->view('templates/reboot');
    }

    /* 显示本地升级ECU页面 */
    public function upgrade_ecu()
    {
        $data['page'] = $this->page;
        $data['func'] = "upgrade_ecu";
        $this->nav['upgrade_ecu']['active'] = '1';
        $data['nav'] = $this->nav;
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('management/upgrade_ecu');
        $this->load->view('templates/footer');
    }
    
    /* 执行本地升级ECU */
    function exec_upgrade_ecu()
    {
        $data = $this->management_model->exec_upgrade_ecu();
        $data['page'] = $this->page;
        $data['func'] = "exec_upgrade_ecu";
        $this->nav['upgrade_ecu']['active'] = '1';
        $data['nav'] = $this->nav;
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('management/upgrade_ecu_result');
        $this->load->view('templates/footer');        
    }

}


/* End of file management.php */
/* Location: ./application/controllers/management.php */