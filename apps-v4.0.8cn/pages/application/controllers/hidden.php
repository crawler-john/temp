<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Hidden extends CI_Controller {

    public $page = "hidden";

    public function __construct()
    {
        parent::__construct();

        $this->load->library('session');
        $this->load->helper('url');
        $this->load->helper('form');    
        $this->load->model('hidden_model');
        $this->load->library('upload');

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
    
    public function index()
    {
        $data['page'] = $this->page;
        $data['func'] = "hidden_index";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/hidden_index');
        $this->load->view('templates/footer');
    }

    /* 显示DEBUG页面 */
    public function debug($result = "")
    {
        $data['page'] = $this->page;
        $data['func'] = "debug";
        $data['result'] = $result;      
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/debug');
        $this->load->view('templates/footer');
    }

     /* 执行DEBUG命令 */
    public function exec_command()
    {      
        $results = $this->hidden_model->exec_command();
//        $results["message"] = $this->lang->line("updatecenter_result_{$results["value"]}");
        echo json_encode($results);        
    }

    /* 显示导数据页面 */
    public function export_file()
    {
        $data = $this->hidden_model->get_export_time();
        $data['page'] = $this->page;
        $data['func'] = "export_file";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/export_file');
        $this->load->view('templates/footer');
    }

    /* 执行导数据操作 */
    public function exec_export_file()
    {
        $result = $this->hidden_model->exec_export_file();
    }

    /* 显示自动更新的服务器的地址和端口 */
    public function updatecenter()
    {        
        $data = $this->hidden_model->get_updatecenter();
        $data['page'] = $this->page;
        $data['func'] = "updatecenter";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/updatecenter');
        $this->load->view('templates/footer'); 
    }

    /* 设置自动更新的服务器的地址和端口 */
    public function set_updatecenter()
    {
        $results = $this->hidden_model->set_updatecenter();
        $results["message"] = $this->lang->line("updatecenter_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示EMA的地址和端口 */
    public function datacenter($result = "")
    {        
        $data = $this->hidden_model->get_datacenter();
        $data['page'] = $this->page;
        $data['func'] = "datacenter";
        $data['result'] = $result;
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/datacenter');
        $this->load->view('templates/footer');  
    }

    /* 设置EMA的地址和端口 */
    public function set_datacenter()
    {
        $results = $this->hidden_model->set_datacenter();
        $results["message"] = $this->lang->line("datacenter_result_{$results["value"]}");
        echo json_encode($results);
    }

    /* 显示初始化页面 */
    public function initialize($result = "")
    {
        $data['page'] = $this->page;
        $data['func'] = "initialize";
        $data['result'] = $result;      
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/initialize');
        $this->load->view('templates/footer');
    }

    /* 执行初始化操作 */
    public function exec_initialize()
    {      
        $data = $this->hidden_model->exec_initialize();
        $this->initialize($data['result']);
    }

    /* 显示串口信息 */
    public function serial($result = "")
    {        
        $data = $this->hidden_model->get_serial();
        $data['page'] = $this->page;
        $data['func'] = "serial";
        $data['result'] = $result;  
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/serial');
        $this->load->view('templates/footer');  
    }

    /* 设置串口信息 */
    public function set_serial()
    {        
        $results = $this->hidden_model->set_serial();
        $results["message"] = $this->lang->line("serial_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 显示电网环境页面 */
    public function grid_environment()
    {
        $data = $this->hidden_model->get_grid_environment();
        $data['page'] = $this->page;
        $data['func'] = "grid_environment";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/grid_environment');
        $this->load->view('templates/footer');
    }
    
    /* 设置电网环境 */
    public function set_grid_environment()
    {
        $results = $this->hidden_model->set_grid_environment();
        $results["message"] = $this->lang->line("grid_environment_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 读取电网环境 */
    public function read_grid_environment()
    {
        $results = $this->hidden_model->read_grid_environment();
        $results["message"] = $this->lang->line("grid_environment_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 显示IRD控制页面 */
    public function ird()
    {
        $data = $this->hidden_model->get_ird();
        $data['page'] = $this->page;
        $data['func'] = "ird";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/ird');
        $this->load->view('templates/footer');
    }
    
    /* 设置IRD控制 */
    public function set_ird()
    {
        $results = $this->hidden_model->set_ird();
        $results["message"] = $this->lang->line("ird_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 读取IRD控制 */
    public function read_ird()
    {
        $results = $this->hidden_model->read_ird();
        $results["message"] = $this->lang->line("ird_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 显示逆变器信号强度页面 */
    public function signal_level()
    {
        $data = $this->hidden_model->get_signal_level();
        $data['page'] = $this->page;
        $data['func'] = "signal_level";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/signal_level');
        $this->load->view('templates/footer');
    }
    
    /* 读取逆变器信号强度 */
    public function read_signal_level()
    {
        $results = $this->hidden_model->read_signal_level();
        $results["message"] = $this->lang->line("signal_level_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 显示上传文件页面 */
    public function upload()
    {
        $data['page'] = $this->page;
        $data['func'] = "upload";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/upload');
        $this->load->view('templates/footer');
    }
    
    /* 执行上传文件 */
    function do_upload()
    {
        $data = $this->hidden_model->do_upload();
        $data['page'] = $this->page;
        $data['func'] = "upload";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/upload_result');
        $this->load->view('templates/footer');        
    }
    
    /* 显示信道页面 */
    function channel()
    {
        $data = $this->hidden_model->get_channel();
        $data['page'] = $this->page;
        $data['func'] = "";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/channel');
        $this->load->view('templates/footer');
    }
    
    /* 设置信道 */
    function set_channel()
    {
        $results = $this->hidden_model->set_channel();
        if(!$results['value'])
            $results["message"] = "channel saved successfully";
        else
            $results["message"] = "channel saved failed";
        echo json_encode($results);
    }

    /* 重置信道 */
    function reset_channel()
    {
        $results = $this->hidden_model->reset_channel();
        if(!$results['value'])
            $results["message"] = "channel reseted successfully";
        else
            $results["message"] = "channel reseted failed";
        echo json_encode($results);
    }
    
//    /* 显示重置优化器页面 */
//    function reset()
//    {
//        $data = $this->hidden_model->get_reset();
//        $data['page'] = $this->page;
//        $data['func'] = "";
//        $this->load->view('templates/header', $data);
//        $this->load->view('templates/nav');
//        $this->load->view('hidden/reset_channel');
//        $this->load->view('templates/footer');
//    }
    
    /* 显示DC设置 */
    function dc()
    {
        $data = $this->hidden_model->get_dc();
        $data['page'] = $this->page;
        $data['func'] = "";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/dc');
        $this->load->view('templates/footer');
    }
    
    /* DC设置 */
    function set_dc()
    {
        $results = $this->hidden_model->set_dc();
        if(!$results['value'])
            $results["message"] = "channel saved successfully";
        else
            $results["message"] = "channel saved failed";
        echo json_encode($results);
    }
    
    /* 读取DC */
    function read_dc()
    {
        $results = $this->hidden_model->read_dc();
        $results["message"] = $this->lang->line("dc_result_{$results["value"]}");
        echo json_encode($results);
    }
    
    /* 远程更新 */
    function remoteupdate()
    {
        $data = $this->hidden_model->get_remoteupdate();
        $data['page'] = $this->page;
        $data['func'] = "remoteupdate";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/remoteupdate');
        $this->load->view('templates/footer');
    }
    
    /* 设置远程更新 */
    function set_remoteupdate()
    {
        $results['success'] = $this->hidden_model->set_remoteupdate();
        echo json_encode($results);
    }

    /* 查看隐藏版本号 */
    function version()
    {
        $data = $this->hidden_model->get_version();
        $data['page'] = $this->page;
        $data['func'] = "version";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/version');
        $this->load->view('templates/footer');
    }

    /* 显示GPRS */
    function gprs()
    {
        $data = $this->hidden_model->get_gprs();
        $data['page'] = $this->page;
        $data['func'] = "gprs";
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('hidden/gprs');
        $this->load->view('templates/footer');
    }
    
    /* 设置GPRS */
    public function set_gprs()
    {
        $results = $this->hidden_model->set_gprs();
        $results["message"] = $this->lang->line("gprs_result_{$results["value"]}_{$results["gprs"]}");
        echo json_encode($results);
    }

}

/* End of file hidden.php */
/* Location: ./application/controllers/hidden.php */