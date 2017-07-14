<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result"></div>

<!-- 加密信息设置表格 -->
<!-- 表单 -->
<form class="form-horizontal" id="defaultForm" method="ajax">

<div class="form-group">    
  <label for="inputdata1" class="col-sm-3 control-label"><?php echo $this->lang->line('set_heartbeat')?></label>
  <div class="col-sm-4">
    <div class="input-group">     
    <input type="text" name="heartbeat" class="form-control" id="heartbeat" placeholder="<?php echo $heartbeat;?>">
         <span class="input-group-addon"><div class="unit">Sec</div></span>  
    </div> 
   </div>
	<div class="col-sm-1">
  	  <button class="btn btn-primary btn-sm" id="button_ok" type="submit"><?php echo $this->lang->line('button_ok')?></button>
	</div> 
</div>
 
</form>

<script>
$(document).ready(function() 
		{	
	//表单验证
    $('#defaultForm').bootstrapValidator({
        message: 'This value is not valid',
        fields: {
        	heartbeat: {
                validators: {
//                    notEmpty: {
//                        message: '<?php echo $this->lang->line('validform_null_heart')?>'
//                    },
 //                   stringLength: {
 //                       max: 1,
 //                       message: 'The_input_string_must_be_1'
 //                   },
                    regexp: {
                        regexp: /^([6-9]\d|[1-2]\d\d|300)$/,
                        message: '<?php echo $this->lang->line('validform_heart')?>'
                    }
                }
            }
            		            		            
        }
    })
.on('success.form.bv', function(e) {
//防止默认表单提交，采用ajax提交
e.preventDefault();
});
	//设置表单处理
	$("#button_ok").click(function(){
	$("#result").hide(); 
    	$.ajax({
			url : "<?php echo base_url('index.php/configuration/set_heartbeat');?>",
			type : "post",
        	dataType : "json",
			data: "heartbeat=" + $("#heartbeat").val(),			  
	    		success : function(Results){
	    			$("#result").text(Results.message);
            	if(Results.value == 0){
	                	$("#result").removeClass().addClass("alert alert-success");
                	setTimeout('$("#result").fadeToggle("slow")', 3000);
	            	}
            	else{
                	$("#result").removeClass().addClass("alert alert-warning");
            	}              		 
            	$("#result").fadeToggle("slow");
        		window.scrollTo(0,0);//页面置顶
            	setTimeout("location.reload();",60000);//刷新页面
			},
	    		error : function() { alert("Error"); }
    	})        
	});   

  
  });	    
</script>
