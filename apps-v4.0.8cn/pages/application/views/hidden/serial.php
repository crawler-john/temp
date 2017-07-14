<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result">
    <button class="close" type="button"><span aria-hidden="true">&times;</span></button>
    <strong></strong><small></small>
</div>

<!-- 串口设置表单 -->
<form class="form-horizontal" id="defaultForm" method="ajax">
  <div class="form-group">    
    <label class="col-sm-5 control-label"><?php echo $this->lang->line('serial_switch')?></label>
    <div class="col-sm-4">
      <select name='serial_switch' class="form-control" id="inputdata1">";
        <option value="on" <?php if(!strncmp($serial_switch, "on", 2))
                                    echo "selected=\"selected\"";
                                ?>><?php echo $this->lang->line('serial_switch_on')?></option>
        <option value="off" <?php if(!strncmp($serial_switch, "off", 3))
                                    echo "selected=\"selected\"";
                                ?>><?php echo $this->lang->line('serial_switch_off')?></option>
      </select>
    </div>
  </div>

  <div class="form-group">    
    <label class="col-sm-5 control-label"><?php echo $this->lang->line('serial_baud_rate')?></label>
    <div class="col-sm-4">
      <select name='baud_rate' class="form-control" id="inputdata2">";
        <option value="2400" <?php if(!strncmp($baud_rate, "2400", 4))
                                    echo "selected=\"selected\"";?>>2400</option>
        <option value="4800" <?php if(!strncmp($baud_rate, "4800", 4))
                                    echo "selected=\"selected\"";?>>4800</option>
        <option value="9600" <?php if(!strncmp($baud_rate, "9600", 4))
                                    echo "selected=\"selected\"";?>>9600</option>
        <option value="19200" <?php if(!strncmp($baud_rate, "19200", 5))
                                    echo "selected=\"selected\"";?>>19200</option>
        <option value="38400" <?php if(!strncmp($baud_rate, "38400", 5))
                                    echo "selected=\"selected\"";?>>38400</option>
        <option value="57600" <?php if(!strncmp($baud_rate, "57600", 5))
                                    echo "selected=\"selected\"";?>>57600</option>
        <option value="115200" <?php if(!strncmp($baud_rate, "115200", 6))
                                    echo "selected=\"selected\"";?>>115200</option>
      </select>
    </div>
  </div>

  <div class="form-group">    
    <label for="inputdata3" class="col-sm-5 control-label"><?php echo $this->lang->line('serial_ecu_address')?></label>
    <div class="col-sm-4">
      <input type="text" name="ecu_address" class="form-control" id="inputdata3" value="<?php echo $ecu_address;?>">
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
            ecu_address: {
                message: 'The ecu_address is not valid',
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_ecu_address')?>'
                    },
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    regexp: {
                        regexp: /^0*(\d{1,2}|([1][0-1]\d)|([1][2][0-8]))$/,
                        message: '<?php echo $this->lang->line('validform_ecu_address')?>'
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
    		url : "<?php echo base_url('index.php/hidden/set_serial');?>",
    		type : "post",
            dataType : "json",
    		data: "serial_switch=" + $("#inputdata1").val()
    			  + "&baud_rate=" + $("#inputdata2").val() 
    		      + "&ecu_address=" + $("#inputdata3").val(),
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