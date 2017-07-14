<?php $this->load->view('templates/header'); ?>
<?php $this->load->view('templates/nav'); ?>
<table class="table table-condensed table-striped table-bordered">    
    <tr>
        <!-- ECU ID -->
        <th scope="row" class="col-xs-6"><?php echo $this->lang->line('ecu_id'); ?></th>
        <td><?php echo $ecuid;?></td>
    </tr>        
    <tr>
        <!-- 历史发电量 -->
        <th scope="row"><?php echo $this->lang->line('home_lifetimepower'); ?></th>
        <td><?php echo $lifetimepower, ' kWh ';?></td>
    </tr>
    <tr>
        <!-- 最近一次系统功率 -->
        <th scope="row"><?php echo $this->lang->line('home_systemp'); ?></th>
        <td><?php echo $systemp, ' W ';?></td>
    </tr>
    <tr>
        <!-- 系统当天累计发电量 -->
        <th scope="row"><?php echo $this->lang->line('home_todaypower'); ?></th>
        <td><?php echo $todaypower, ' kWh ';?></td>
    </tr>
    <tr>
        <!-- 最近一次连接服务器时间 -->
        <th scope="row"><?php echo $this->lang->line('home_datetime'); ?></th>
        <td><?php echo $datetime;?></td>
    </tr>
    <tr>
        <!-- 逆变器总台数 -->
        <th scope="row"><?php echo $this->lang->line('home_maxnum'); ?></th>
        <td><?php echo $maxnum;?></td>
    </tr>
    <tr>
        <!-- 最近一次逆变器连接台数 -->
        <th scope="row"><?php echo $this->lang->line('home_curnum'); ?></th>
        <td><?php echo $curnum;?></td>
    </tr>
    <tr>
        <!-- 软件版本号 -->
        <th scope="row"><?php echo $this->lang->line('home_version'); ?></th>
        <td><?php echo $version; ?></td>
    </tr>
    <tr>
        <!-- 当前时区 -->
        <th scope="row"><?php echo $this->lang->line('home_timezone'); ?></th>
        <td><?php echo $timezone; ?></td>
    </tr>
    <tr>
        <!-- ECU有线网络Mac地址 -->
        <th scope="row"><?php echo $this->lang->line('home_eth0_mac'); ?></th>
        <td><?php echo $eth0_mac; ?></td>
    </tr>
    <!-- ECU无线网络Mac地址 -->
    <?php 
        $wifi_flag="1";
        $fp=@fopen("/etc/yuneng/wifi.conf","r");
        if($fp){
        $wifi=fgets($fp);
        fclose($fp);
            if(!strncmp("0",$wifi,1))
               $wifi_flag="0";
        }
    ?>
    <?php 
       if(!strcmp($wifi_flag,"0"))
       echo "<!--";               	
    ?>   
    <tr>
        <th scope="row"><?php echo $this->lang->line('home_wlan0_mac'); ?></th>
        <td><?php echo $wlan0_mac; ?></td>
    </tr>
    <?php 
       if(!strcmp($wifi_flag,"0"))
       echo "-->";
    ?>       
<?php if (strlen($grid_quality) > 0): ?>
    <tr>
        <!-- 逆变器通信信号强度 -->
        <th scope="row"><?php echo $this->lang->line('home_grid_quality'); ?></th>
        <td><?php echo $grid_quality; ?></td>
    </tr>
<?php endif; ?>
</table>

<script>
var weekday_en = new Array(7);
    weekday_en[0]="Sunday";
    weekday_en[1]="Monday";
    weekday_en[2]="Tuesday";
    weekday_en[3]="Wednesday";
    weekday_en[4]="Thursday";
    weekday_en[5]="Friday";
    weekday_en[6]="Saturday";
var weekday_ch = new Array(7);
    weekday_ch[0]="星期日";
    weekday_ch[1]="星期一";
    weekday_ch[2]="星期二";
    weekday_ch[3]="星期三";
    weekday_ch[4]="星期四";
    weekday_ch[5]="星期五";
    weekday_ch[6]="星期六";
var today = new Date();
    today.setFullYear(<?php echo date("Y");?>);
    today.setMonth(<?php echo date("n")."-1";?>);
    today.setDate(<?php echo date("j");?>); 
    today.setHours(<?php echo date("H");?>);
    today.setMinutes(<?php echo date("i");?>);
    today.setSeconds(<?php echo date("s");?>);
var timestamp =  today.getTime();

/* 显示实时时间 */
runTime();     
    	  
function runTime() {
    setTimeout(runTime, 1000);
    timestamp = timestamp + 1000;
    today.setTime(timestamp);     
    showTime();
}
  
function showTime() {
    var year, month, date, hour, minute, second, day;
    year = today.getFullYear();
    month = today.getMonth() + 1; 
    date = today.getDate(); 
    hour = today.getHours(); 
    minute = today.getMinutes(); 
    second = today.getSeconds();
    day = today.getDay();  
    if (month < 10) { month = "0" + month;}
    if (date < 10) { date = "0" + date;}  
    if (hour < 10) { hour = "0" + hour;}
    if (minute < 10) { minute = "0" + minute;}
    if (second < 10) { second = "0" + second;}
    if (/[\u4E00-\u9FA5]/i.test($("#ecu_title").html())) {
        $('#ecu_time').html(year + "年" + month + "月" + date + "日 " + hour + ":" + minute + ":" + second + "<br>" + weekday_ch[day]); 
    }
    else {
        $('#ecu_time').html(year + "-" + month + "-" + date + " " + hour + ":" + minute + ":" + second + "<br>" + weekday_en[day]); 
    }
}
</script>
<?php $this->load->view('templates/footer'); ?>