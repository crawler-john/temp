<!DOCTYPE html>
<html lang="zh-cn">
  <head>
    <meta charset="utf-8">
    <!-- 兼容IE -->
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <!-- 支持国产浏览器高速模式 -->
    <meta name="renderer" content="webkit">
    <!-- 响应式布局 -->
    <meta name="viewport" content="width=device-width, initial-scale=1">   

    <title><?php echo $this->lang->line('title')?></title>

    <link type="image/x-icon" href="<?php echo base_url('images/logo-icon.png');?>" rel="shortcut icon">    
    <link href="<?php echo base_url('css/bootstrap.min.css');?>" rel="stylesheet">
    <link href="<?php echo base_url('css/ecu-style.css');?>" rel="stylesheet">  
    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn"t work if you view the page via file:// -->
    <!--[if lt IE 9]>
      <script src="js/html5shiv.min.js"></script>
      <script src="js/respond.min.js"></script>
    <![endif]-->    
    <script src="<?php echo base_url('js/jquery-1.8.2.min.js');?>"></script>
    <script src="<?php echo base_url('js/bootstrap.min.js');?>"></script>
  </head>

  <body class="ecu-reboot">
    <div class="container">
        <div class="col-md-offset-3 col-md-6 content">
          <h2><?php echo $this->lang->line('ecu_reboot_title')?></h2>
            <div class="progress">
              <div class="progress-bar progress-bar-striped active" id="bar" role="progressbar"  aria-valuemin="0" aria-valuemax="100" style="width: 2%">
              </div>
            </div>
          <p><?php echo $this->lang->line('ecu_reboot')?></p>
        </div>
    </div>
  </body>
  <script>
function $(obj){ 
  return document.getElementById(obj); 
} 
function go(){ 
  $("bar").style.width = parseInt($("bar").style.width) + 1 + "%"; 
  if($("bar").style.width == "100%"){ 
    window.clearInterval(bar); 
  }
} 
var bar = window.setInterval("go()",1000); 
window.onload = function(){ 
  bar; 
} 
	    
function jumpurl(){  
    location="<?php echo base_url('index.php');?>";  
}
setTimeout('jumpurl()',100000); 
    </script>
</html>