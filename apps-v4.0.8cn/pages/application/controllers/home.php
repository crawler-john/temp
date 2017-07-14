<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Home extends CI_Controller {

    public $page = "home";
    public $nav = array();

    public function __construct()
    {
        parent::__construct();

        $this->load->library('session');
        $this->load->helper('url');
        $this->load->model('home_model');

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
        
        $data['func'] = "home";
        $data['nav'] = $this->nav;
        $data['page'] = $this->page;
        
        //获取主页显示数据
        $result = $this->home_model->get_data();
        $data = array_merge($data, $result);
        
        //显示页面
        $this->load->view('home', $data);
    }

}

/* End of file home.php */
/* Location: ./application/controllers/home.php */