<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result"></div>

<form class="form-horizontal" id="defaultForm" method="ajax">

  <div class="form-group">    
    <label for="inputdata1" class="col-sm-5 control-label"><?php echo $this->lang->line('user_info_username')?></label>
    <div class="col-sm-4">
      <input type="text" name="username" class="form-control" id="inputdata1" value="">
    </div>
  </div>

  <div class="form-group">    
    <label for="inputdata2" class="col-sm-5 control-label"><?php echo $this->lang->line('user_info_old_password')?></label>
    <div class="col-sm-4">
      <input type="password" name="old_password" class="form-control" id="inputdata2" value="">
    </div>
  </div>
  
  <div class="form-group">    
    <label for="inputdata5" class="col-sm-5 control-label"><?php echo $this->lang->line('user_info_new_username')?></label>
    <div class="col-sm-4">
      <input type="text" name="new_username" class="form-control" id="inputdata5" value="">
    </div>
  </div>

  <div class="form-group">    
    <label for="inputdata3" class="col-sm-5 control-label"><?php echo $this->lang->line('user_info_new_password')?></label>
    <div class="col-sm-4">
      <input type="password" name="new_password" class="form-control" id="inputdata3" value="">
    </div>
  </div>

  <div class="form-group">    
    <label for="inputdata4" class="col-sm-5 control-label"><?php echo $this->lang->line('user_info_confirm_password')?></label>
    <div class="col-sm-4">
      <input type="password" name="confirm_password" class="form-control" id="inputdata4" value="">
    </div>
  </div>

  <div class="form-group">
    <div class="col-sm-offset-5 col-sm-2">
      <button class="btn btn-primary btn-sm" id="button_update" type="submit" ><?php echo $this->lang->line('button_update')?></button>
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
            username: {
                message: 'The username is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_username')?>'
                    },
                    stringLength: {
                        min: 4,
                        max: 18,
                        message: '<?php echo $this->lang->line('validform_username')?>'
                    }
                }
            },
            old_password: {
                message: 'The old_password is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_old_password')?>'
                    },
                    stringLength: {
                        min: 5,
                        max: 18,
                        message: '<?php echo $this->lang->line('validform_old_password')?>'
                    }
                }
            },
            new_password: {
                message: 'The new_password is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_new_password')?>'
                    },
                    stringLength: {
                        min: 5,
                        max: 18,
                        message: '<?php echo $this->lang->line('validform_new_password')?>'
                    }
                }
            },
            confirm_password: {
                message: 'The confirm_password is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_confirm_password')?>'
                    },
                    stringLength: {
                        min: 5,
                        max: 18,
                        message: '<?php echo $this->lang->line('validform_new_password')?>'
                    },
                    identical: {
                        field: 'new_password',
                        message: '<?php echo $this->lang->line('validform_confirm_password')?>'
                    }
                }
            },
            new_username: {
                message: 'The new_username is not valid',
                validators: {
                    stringLength: {
                        min: 4,
                        max: 18,
                        message: '<?php echo $this->lang->line('validform_username')?>'
                    },
                }
            },
        }
    })
    .on('success.form.bv', function(e) {
        //防止默认表单提交，采用ajax提交
        e.preventDefault();

        //设置表单处理
        $("#result").hide();       	
	    $.ajax({
    		url : "<?php echo base_url('index.php/configuration/set_user_info');?>",
    		type : "post",
            dataType : "json",
    		data: "username=" + $("#inputdata1").val()
    			  + "&old_password=" + $("#inputdata2").val() 
    		      + "&new_password=" + $("#inputdata3").val() 
    		      + "&confirm_password=" + $("#inputdata4").val()
    		      + "&new_username=" + $("#inputdata5").val(),
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
            },
            error : function() { alert("Error"); }
        })        
    });    
    
});
</script>