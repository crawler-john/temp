<!-- 设置结果显示框 -->
<div class="alert" id="result">
    <button class="close" type="button"><span aria-hidden="true">&times;</span></button>
    <strong></strong><small></small>
</div>

<!-- 输入自定义命令 -->
<div class="form-group">    
    <label class="col-sm-12 control-label"><?php echo $this->lang->line('debug_command_input')?></label>
    <div class="col-sm-10">
        <input type="text" name="command" class="form-control" id="inputdata1" value="">
    </div>
</div>
<div class="form-group">
    <div class="col-sm-2">
        <button class="btn btn-primary btn-sm" id="button_execute" type="button"><?php echo $this->lang->line('debug_command_execute')?></button>
    </div>
</div>
 
 <!-- 外部程序执行结果原始输出区域 --> 
<div class="col-sm-12">
    <p><pre id="res_array"></pre></p>
</div>
  
<script>
$(document).ready(function() {

	//隐藏设置结果栏
	$("#result").hide();
	$(".close").click(function(){
		$("#result").hide();
    }); 

    //设置表单处理
    $("#button_execute").click(function(){
    	$("#result").hide();
	    $.ajax({
    		url : "<?php echo base_url('index.php/hidden/exec_command');?>",
    		type : "post",
            dataType : "json",
    		data: "command=" + $("#inputdata1").val(),
  	    	success : function(Results){
                if(Results.value == 0){
  	                $("#result").removeClass().addClass("alert alert-success");
  	                $("#result strong").text("<?php echo $this->lang->line('debug_command_success')?>" + "：");  
  	            }
                else if(Results.value == -1){
  	                $("#result").removeClass().addClass("alert alert-warning");
  	                $("#result strong").text("<?php echo $this->lang->line('debug_command_is_null')?>" + "：");  
  	            }
                else{
                    $("#result").removeClass().addClass("alert alert-danger");
                    $("#result strong").text("<?php echo $this->lang->line('debug_command_failed')?>" + "：");  
                }
                $("#result small").text(Results.value);        		 
            	$("#result").show();
            	$("#res_array").text(Results.res_array.join('\n'));
    		},
  	    	error : function(){
  	    		alert("Error");
  	    	}
        })
        window.scrollTo(0,0);//页面置顶
    });    
});
</script>