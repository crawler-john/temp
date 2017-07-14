<!-- 登录结果显示框 -->
<div class="alert alert-success" id="result"></div>

<!-- 登录表单 -->
<form class="form-horizontal" id="defaultForm" method="ajax">
  <br><br>
  <div class="form-group">    
    <label for="inputdata1" class="col-sm-4 control-label"><?php echo $this->lang->line('login_username')?></label>
    <div class="col-sm-4">
      <input class="form-control" id="inputdata1"  type="text" name="username">
    </div>
  </div>  
  <div class="form-group">    
    <label for="inputdata2" class="col-sm-4 control-label"><?php echo $this->lang->line('login_password')?></label>
    <div class="col-sm-4">
      <input class="form-control" id="inputdata2"  type="password" name="password">
    </div>
  </div>
  <div class="form-group">    
    <div class="col-sm-4 col-sm-offset-4">
      <button class="btn btn-primary" id="button_login" type="submit"><?php echo $this->lang->line('login_login')?></button>
    </div>
  </div>
</form>

<script>
	
$(document).ready(function() 
{    
	$('#defaultForm').bootstrapValidator({
	    fields: {
	    	username: {
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_username')?>'
                    },
                    stringLength: {
                        min: 4,
                        max: 16,
                        message: '<?php echo $this->lang->line('validform_username')?>'
                    },
                }
            },
            password: {
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_password')?>'
                    },
                    stringLength: {
                    	min: 5,
                        max: 16,
                        message: '<?php echo $this->lang->line('validform_old_password')?>'
                    },
                }
            },
        }
    })
    .on('success.form.bv', function(e) {
        //防止默认表单提交，采用ajax提交
        e.preventDefault();
    });

    //按下回车触发点击事件
    $("body").keydown(function() {
        if (event.keyCode == "13") {
            //keyCode=13是回车键
            $('#button_login').click();
        }
    }); 
    
    //登录表单处理
    $("#button_login").click(function(){
    	$("#result").hide();    	
	    $.ajax({
    		url : "<?php echo base_url('index.php/configuration/check_login');?>",
    		type : "post",
            dataType : "json",
    		data: "username=" + $("#inputdata1").val()
    			  + "&password=" + $("#inputdata2").val(),
  	    	success : function(Results){
  	    		$("#result").text(Results.message);
                if(Results.value == 0){
  	                $("#result").removeClass().addClass("alert alert-success");
    	            setTimeout("location.reload();",500);
  	            }
                else{
                    $("#result").removeClass().addClass("alert alert-danger");
                    $('#button_login').removeAttr("disabled");   
                }		 
                $("#result").fadeToggle("slow");
    		},
    		error : function() { alert("Error"); }
        })
    });
});
</script>