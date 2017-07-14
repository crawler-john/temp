<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Configuration extends CI_Controller {

    public $page = "configuration";
    public $nav = array(
        'protection' => array(
            'url' => 'index.php/configuration',
            'active' => '0',
            'name' => 'protection'
        ),
        'gfdi_state' => array(
            'url' => 'index.php/configuration/gfdi_state',
            'active' => '0',
            'name' => 'gfdi'
        ),
        'switch_state' => array(
            'url' => 'index.php/configuration/switch_state',
            'active' => '0',
            'name' => 'switch'
        ),
        'user_info' => array(
            'url' => 'index.php/configuration/user_info',
            'active' => '0',
            'name' => 'user_info'
        ),
        'maxpower' => array(
            'url' => 'index.php/configuration/maxpower',
            'active' => '0',
            'name' => 'maxpower'
        )
    );

    public function __construct()
    {
        parent::__construct();

        $this->load->library('session');
        $this->load->helper('url');
        $this->load->helper('form');    
        $this->load->model('configuration_model');

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

    /* 显示登陆页面 */
    public function login()
    {
        $data['page'] = $this->page;
        $data['func'] = "login";
        $data['nav'] = $this->nav;
        
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('configuration/login');
        $this->load->view('templates/footer');
       
    }

    /* 验证登陆信息 */
    public function check_login()
    {
        $results = $this->configuration_model->check_login();
        $results["message"] = $this->lang->line("login_result_{$results["value"]}");
        echo json_encode($results);           
    }

    /* 显示5项交流保护参数(默认函数) */
    public function index()
    {
        if($this->session->userdata('logged_in') == FALSE){
            $this->login();
        }
        else{
            //页面、标题、导航
            $data['page'] = $this->page;
            $data['func'] = "protection";
            $this->nav['protection']['active'] = '1';
            $data['nav'] = $this->nav;
            
            //获取数据            
            $result = $this->configuration_model->get_protection();
            $data = array_merge($data, $result);
            
            $this->load->view('templates/header', $data);
            $this->load->view('templates/nav');
            $this->load->view('configuration/protection');
            $this->load->view('templates/footer');
        }   
    } 

    /* 设置5项交流保护参数 */
    public function set_protection()
    {
        $results = $this->configuration_model->set_protection();       
        $results["message"] = $this->lang->line("protection_result_{$results["value"]}"); 
        echo json_encode($results);
    }

    /* 读取逆变器交流保护参数 */
    public function read_inverter_parameters()
    {
        $results = $this->configuration_model->read_inverter_parameters();
        $results["message"] = $this->lang->line("read_protection_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示13项交流保护参数(默认函数) */
    public function protection2()
    {
        if($this->session->userdata('logged_in') == FALSE)
        {
            $this->login();
        }
        else
        {
            //页面、标题、导航
            $data['page'] = $this->page;
            $data['func'] = "protection2";
            $data['nav'] = $this->nav;
            
            //获取数据
            $result = $this->configuration_model->get_protection2();
            $data = array_merge($data, $result);
            
            $this->load->view('templates/header', $data);
            $this->load->view('templates/nav');
            $this->load->view('configuration/protection2');
            $this->load->view('templates/footer');
        }   
    } 

    /* 设置13项交流保护参数 */
    public function set_protection2()
    {
        $results = $this->configuration_model->set_protection2();
        $results["message"] = $this->lang->line("protection_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 显示17项交流保护参数(默认函数) */
    public function protection17()
    {
        if($this->session->userdata('logged_in') == FALSE)
        {
            $this->login();
        }
        else
        {
            //页面、标题、导航
            $data['page'] = $this->page;
            $data['func'] = "protection";
            $data['nav'] = $this->nav;
    
            //获取数据
            $result = $this->configuration_model->get_protection17();
            $data = array_merge($data, $result);
    
            $this->load->view('templates/header', $data);
            $this->load->view('templates/nav');
            $this->load->view('configuration/protection17');
            $this->load->view('templates/footer');
        }
    }
    
    /* 设置17项交流保护参数 */
    public function set_protection17()
    {
        $results = $this->configuration_model->set_protection17();
        $results["message"] = $this->lang->line("protection_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示逆变器GFDI状态 */
    public function gfdi_state()
    {
        if($this->session->userdata('logged_in') == FALSE)
        {
            $this->login();
        }
        else
        {
            //页面、标题、导航
            $data['page'] = $this->page;
            $data['func'] = "gfdi";
            $this->nav['gfdi_state']['active'] = '1';
            $data['nav'] = $this->nav;
            
            //获取数据
            $result = $this->configuration_model->get_gfdi_state();
            $data = array_merge($data, $result);
            
            $this->load->view('templates/header', $data);
            $this->load->view('templates/nav');
            $this->load->view('configuration/gfdi_state');
            $this->load->view('templates/footer');
        }      
    }

    /* 设置逆变器GFDI状态 */
    public function set_gfdi_state()
    {
        $results = $this->configuration_model->set_gfdi_state();
        $results["message"] = $this->lang->line("gfdi_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示逆变器开关机状态 */
    public function switch_state()
    {
        if($this->session->userdata('logged_in') == FALSE)
        {
            $this->login();
        }
        else
        {
            $data['page'] = $this->page;
            $data['func'] = "switch";
            $this->nav['switch_state']['active'] = '1';
            $data['nav'] = $this->nav;
            
            //获取数据
            $result = $this->configuration_model->get_switch_state();
            $data = array_merge($data, $result);
            
            $this->load->view('templates/header', $data);
            $this->load->view('templates/nav');
            $this->load->view('configuration/switch_state');
            $this->load->view('templates/footer');
        }
    }

    /* 设置逆变器开关机状态 */
    public function set_switch_state()
    {
        $results = $this->configuration_model->set_switch_state();
        $results["message"] = $this->lang->line("switch_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 设置所有逆变器为开机状态 */
    public function set_switch_all_on()
    {
        $results = $this->configuration_model->set_switch_all_on();
        $results["message"] = $this->lang->line("switch_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 设置所有逆变器为关机状态 */
    public function set_switch_all_off()
    {
        $results = $this->configuration_model->set_switch_all_off();
        $results["message"] = $this->lang->line("switch_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示逆变器最大功率 */
    public function maxpower($result= "")
    {
        if($this->session->userdata('logged_in') == FALSE)
        {
            $this->login();
        }
        else
        {
            $data['page'] = $this->page;
            $data['func'] = "maxpower";
            $data['nav'] = $this->nav;
            
            //获取数据
            $result = $this->configuration_model->get_maxpower();
            $data = array_merge($data, $result);
            
            $this->load->view('templates/header', $data);
            $this->load->view('templates/nav');
            $this->load->view('configuration/maxpower');
            $this->load->view('templates/footer');
        }
    }

    /* 设置逆变器最大功率 */
    public function set_maxpower()
    {
        $results = $this->configuration_model->set_maxpower();
        $results["message"] = $this->lang->line("maxpower_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 读取逆变器最大功率 */
    public function read_maxpower()
    {
        $results = $this->configuration_model->read_maxpower();
        $results["message"] = $this->lang->line("maxpower_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 显示用户信息 */
    public function user_info()
    {
        $data['page'] = $this->page;
        $data['func'] = "user_info";
        $this->nav['user_info']['active'] = '1';
        $data['nav'] = $this->nav;
        
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('configuration/user_info');
        $this->load->view('templates/footer');
    }
    
    /* 设置用户信息 */
    public function set_user_info()
    {
        $results = $this->configuration_model->set_user_info();
        $results["message"] = $this->lang->line("user_info_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示加密界面 */
    public function encryption()
    {
    	if($this->session->userdata('logged_in') == FALSE)
    	{
    		$this->login();
    	}
    	else
    	{
    		$data['page'] = $this->page;
    		$data['func'] = "encryption";
    		$data['nav'] = $this->nav;
    	 
    		$result = $this->configuration_model->check_encryption();
    		$data = array_merge($data, $result);
    		
    		$this->load->view('templates/header', $data);
    		$this->load->view('templates/nav');
    		$this->load->view('configuration/encryption');
    		$this->load->view('templates/footer');
    	}
    }
    
    /* 验证加密信息 */
    public function check_encrypt()
    {
    	$results = $this->configuration_model->encrypt();
    	$results["message"] = $this->lang->line("encrypt_result_{$results["value"]}");
    	echo json_encode($results);
    }
    
    /* 修改加密时间 */
    public function modification_time()
    {
    	$results = $this->configuration_model->modi_time();
    	$results["message"] = $this->lang->line("modi_time_result_{$results["value"]}");
    	echo json_encode($results);
    }
    
    /* 显示心跳包时间设置界面 */
    public function heartbeat()
    {
    	if($this->session->userdata('logged_in') == FALSE)
    	{
    		$this->login();
    	}
    	else
    	{
    		$data['page'] = $this->page;
    		$data['func'] = "heartbeat";
    		$data['nav'] = $this->nav;
    
    		$result = $this->configuration_model->heartbeat();
    		$data = array_merge($data, $result);
    
    		$this->load->view('templates/header', $data);
    		$this->load->view('templates/nav');
    		$this->load->view('configuration/heartbeat');
    		$this->load->view('templates/footer');
    	}
    }
    
    /* 修改心跳包时间 */
    public function set_heartbeat()
    {
    	$results = $this->configuration_model->set_heartbeat();
    	$results["message"] = $this->lang->line("set_heartbeat_result_{$results["value"]}");
    	echo json_encode($results);
    }

}


/* End of file configuration.php */
/* Location: ./application/controllers/configuration.php */