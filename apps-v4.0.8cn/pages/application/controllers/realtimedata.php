<?php  if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Realtimedata extends CI_Controller {

    public $page = "realtimedata";
    public $nav = array(
        'realtimedata' => array(
            'url' => 'index.php/realtimedata',
            'active' => '0',
            'name' => 'realtimedata'
            ),
        'power_graph' => array(
            'url' => 'index.php/realtimedata/power_graph',
            'active' => '0',
            'name' => 'power'
            ),
        'energy_graph' => array(
            'url' => 'index.php/realtimedata/energy_graph',
            'active' => '0',
            'name' => 'energy'
            )
    );

    public function __construct()
    {
        parent::__construct();

        $this->load->library('session');
        $this->load->helper('url');
        $this->load->helper('form');    
        $this->load->model('realtimedata_model');

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

    /* 显示实时数据(默认函数) */
    public function index()
    {
        //页面、标题、导航
        $data['page'] = $this->page;
        $data['func'] = "realtimedata";
        $this->nav['realtimedata']['active'] = '1';
        $data['nav'] = $this->nav;
        
        //获取数据
        $result = $this->realtimedata_model->get_data();
        $data = array_merge($data, $result);
        
        //显示页面
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('realtimedata/realtimedata');
        $this->load->view('templates/footer');
    }

    /* 显示功率曲线图 */
    public function power_graph()
    { 
        $data['func'] = "power";
        $this->nav['power_graph']['active'] = '1';
        $data['nav'] = $this->nav;
        $data['page'] = $this->page;
        $result = $this->realtimedata_model->get_power_graph();
        $data = array_merge($data, $result);

        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('realtimedata/power_graph');
        $this->load->view('templates/footer');
    }
    
    /* 显示历史功率曲线图 */
    public function old_power_graph()
    {
        $results = $this->realtimedata_model->get_power_graph();
        echo json_encode($results);
    }

    /* 显示能量柱状图 */
    public function energy_graph()
    {
        $data['func'] = "energy";
        $this->nav['energy_graph']['active'] = '1';
        $data['nav'] = $this->nav;
        $data['page'] = $this->page;
        $result = $this->realtimedata_model->get_energy_graph();
        $data = array_merge($data, $result);

        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('realtimedata/energy_graph');
        $this->load->view('templates/footer');
    }
    
    /* 显示历史能量柱状图 */
    public function old_energy_graph()
    {
        $results = $this->realtimedata_model->get_energy_graph();
        echo json_encode($results);
    }

    /* 显示逆变器工作状态 */
    public function inverter_status()
    {    
        $data['func'] = "inverter_status";
        $data['nav'] = $this->nav;
        $data['page'] = $this->page;
        $result = $this->realtimedata_model->get_inverter_status();
        $data = array_merge($data, $result);
        
        $this->load->view('templates/header', $data);
        $this->load->view('templates/nav');
        $this->load->view('realtimedata/inverter_status');
        $this->load->view('templates/footer');
    }

}

/* End of file realtimedata.php */
/* Location: ./application/controllers/realtimedata.php */