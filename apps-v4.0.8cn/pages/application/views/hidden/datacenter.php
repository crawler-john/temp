<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result">
    <button class="close" type="button"><span aria-hidden="true">&times;</span></button>
    <strong></strong><small></small>
</div>

<!-- 表单 -->
<form class="form-horizontal" id="defaultForm" method="ajax">
  <div class="form-group">    
    <label for="inputdata1" class="col-sm-5 control-label"><?php echo $this->lang->line('datacenter_domain')?></label>
    <div class="col-sm-4">
      <input type="text" name="domain" class="form-control" id="inputdata1" value="<?php echo $domain;?>">
    </div>
  </div>

  <div class="form-group">    
    <label for="inputdata2" class="col-sm-5 control-label"><?php echo $this->lang->line('datacenter_ip')?></label>
    <div class="col-sm-4">
      <input type="text" name="ip" class="form-control" id="inputdata2" value="<?php echo $ip;?>">
    </div>
  </div>

  <div class="form-group">    
    <label for="inputdata3" class="col-sm-5 control-label"><?php echo $this->lang->line('datacenter_port1')?></label>
    <div class="col-sm-4">
      <input type="text" name="port1" class="form-control" id="inputdata3" value="<?php echo $port1;?>">
    </div>
  </div>

  <div class="form-group">    
    <label for="inputdata4" class="col-sm-5 control-label"><?php echo $this->lang->line('datacenter_port2')?></label>
    <div class="col-sm-4">
      <input type="text" name="port2" class="form-control" id="inputdata4" value="<?php echo $port2;?>">
    </div>
  </div>

  <div class="form-group">
    <div class="col-sm-offset-5 col-sm-2">
      <button type="submit" class="btn btn-primary btn-sm"><?php echo $this->lang->line('button_update')?></button>
    </div>
  </div>
</form>

<script>
$(document).ready(function() {

	//隐藏设置结果栏
	$("#result").hide();
	$(".close").click(function(){
		$("#result").hide();
    }); 

	//表单验证
    $('#defaultForm').bootstrapValidator({
        message: 'This value is not valid',
        fields: {
            domain: {
                message: 'The domain is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_domain')?>'
                    },
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    regexp: {
                        regexp: /\w*\.\w+$/,
                        message: '<?php echo $this->lang->line('validform_domain')?>'
                    }
                }
            },
            ip: {
                message: 'The ip is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_ip_address')?>'
                    },
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    ip: {
                        message: '<?php echo $this->lang->line('validform_ip_address')?>'
                    }
                }
            },
            port1: {
                message: 'The port1 is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_port1')?>'
                    },
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    regexp: {
                        regexp: /^(\d{1,4}|([1-5]\d{4})|([1-6][0-4]\d{3})|([1-6][0-5][0-4]\d{2})|([1-6][0-5][0-5][0-2]\d)|([1-6][0-5][0-5][0-3][0-5]))$/,
                        message: '<?php echo $this->lang->line('validform_port1')?>'
                    }
                }
            },
            port2: {
                message: 'The port2 is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_port2')?>'
                    },
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    regexp: {
                        regexp: /^(\d{1,4}|([1-5]\d{4})|([1-6][0-4]\d{3})|([1-6][0-5][0-4]\d{2})|([1-6][0-5][0-5][0-2]\d)|([1-6][0-5][0-5][0-3][0-5]))$/,
                        message: '<?php echo $this->lang->line('validform_port2')?>'
                    }
                }
            }
        }
    })
    .on('success.form.bv', function(e) {
    	$("#result").hide();
    	
        //防止默认表单提交，采用ajax提交
        e.preventDefault();
        //验证成功，采用ajax提交表单
	    $.ajax({
    		url : "<?php echo base_url('index.php/hidden/set_datacenter');?>",
    		type : "post",
            dataType : "json",
    		data: "domain=" + $("#inputdata1").val()
    			  + "&ip=" + $("#inputdata2").val() 
    		      + "&port1=" + $("#inputdata3").val()
    		      + "&port2=" + $("#inputdata4").val(),
  	    	success : function(Results){
                if(Results.value == 0){
  	                $("#result").removeClass().addClass("alert alert-success alert-dismissible");
  	                $("#result strong").text("<?php echo $this->lang->line('message_success')?>" + "：");  
  	            }
                else{
                    $("#result").removeClass().addClass("alert alert-warning alert-dismissible");
                    $("#result strong").text("<?php echo $this->lang->line('message_warning')?>" + "：");  
                }
                $("#result small").text(Results.message);        		 
            	$("#result").show();
    		},
  	    	error : function(){
  	    		alert("Error");
  	    	}
        })
        window.scrollTo(0,0);//页面置顶        
    });
    
});
</script>