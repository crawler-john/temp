<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result"></div>

<!-- 模态框 重启ECU -->
<div class="modal fade" id="ecu_reboot" tabindex="-1" role="dialog" aria-labelledby="ecu_reboot_title" aria-hidden="true" data-backdrop="static" data-keyboard="false">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header"><h4 class="modal-title" id="ecu_reboot_title"><?php echo $this->lang->line('ecu_reboot_title')?></h4></div>
      <div class="modal-body">
        <div class="progress">
          <div class="progress-bar progress-bar-striped active" id="bar" role="progressbar"  aria-valuemin="0" aria-valuemax="100" style="width: 2%"></div>
        </div>
        <p><?php echo $this->lang->line('ecu_reboot')?></p>
      </div>
    </div>
  </div>
</div>

<!-- 设置IP地址 -->
<form id="defaultForm" method="ajax" class="form-horizontal" role="form">
  <fieldset>
    <legend><?php echo $this->lang->line('network_set_ip')?></legend>

    <div class="form-group">    
      <div class="col-sm-8 col-sm-offset-4">
        <input type='radio' name="dhcp" value="1" onclick="setip(this.value)"<?php
                        if ($dhcp==1){
                            echo "checked='checked'";
                        }
                    ?>> <?php echo $this->lang->line('network_use_dhcp')?>
      </div>
    </div>

    <div class="form-group">    
      <div class="col-sm-8 col-sm-offset-4">
        <input type='radio' name="dhcp" value="0" onclick="setip(this.value)"<?php
                        if ($dhcp==0){
                            echo "checked='checked'";
                        }
                    ?>> <?php echo $this->lang->line('network_use_static_ip')?>
      </div>
    </div>
  </fieldset>

  <fieldset id="network_ip" <?php if($dhcp==1){echo "style=\"display:none;\"";}?>>
    <div class="form-group">
      <label for="inputdata1" class="col-sm-4 control-label"><?php echo $this->lang->line('network_ip_address')?></label>
      <div class="col-sm-4">
        <input type="text" name="ip" class="form-control" id="inputdata1" value="<?php echo $ip;?>">
      </div>
    </div>

    <div class="form-group">
      <label for="inputdata2" class="col-sm-4 control-label"><?php echo $this->lang->line('network_subnet_mask')?></label>
      <div class="col-sm-4">
        <input type="text" name="mask" class="form-control" id="inputdata2" value="<?php echo $mask;?>">
      </div>
    </div>

    <div class="form-group">
      <label for="inputdata3" class="col-sm-4 control-label"><?php echo $this->lang->line('network_default_gateway')?></label>
      <div class="col-sm-4">
        <input type="text" name="gate" class="form-control" id="inputdata3" value="<?php echo $gate;?>">
      </div>
    </div>

    <div class="form-group">
      <label for="inputdata4" class="col-sm-4 control-label"><?php echo $this->lang->line('network_preferred_dns_server')?></label>
      <div class="col-sm-4">
        <input type="text" name="dns1" class="form-control" id="inputdata4" value="<?php echo $dns1;?>">
      </div>
    </div>

    <div class="form-group">
      <label for="inputdata5" class="col-sm-4 control-label"><?php echo $this->lang->line('network_alternate_dns_server')?></label>
      <div class="col-sm-4">
        <input type="text" name="dns2" class="form-control" id="inputdata5" value="<?php echo $dns2;?>">
      </div>
    </div>
  </fieldset>

  <div class="form-group">
    <div class="col-sm-offset-4 col-sm-2">
      <button type="submit" class="btn btn-primary btn-sm" id="button_set_ip"><?php echo $this->lang->line('button_update')?></button>
    </div>
  </div>    
</form>

<script>
function setip(value) { 
    if (value == 1){
        document.getElementById('network_ip').style.display= 'none'; 
    }
    if (value == 0) {
        document.getElementById('network_ip').style.display= '';
        //$('#set_static_ip').removeAttr("disabled");
    }
}

/* 进度条控制函数 */
var progress = 1;  
function doProgress() {
	progress = progress +1;
	$("#bar").css("width", progress + "%");
	if($("#bar").css("width") == "100%"){
		//完成
	}
}

$(document).ready(function() 
{		
    $('#defaultForm').bootstrapValidator({
        fields: {
            ip: {
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
            mask: {
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_subnet_mask')?>'
                    },
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    ip: {
                        message: '<?php echo $this->lang->line('validform_subnet_mask')?>'
                    }
                }
            },
            gate: {
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_default_gateway')?>'
                    },
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    ip: {
                        message: '<?php echo $this->lang->line('validform_default_gateway')?>'
                    }
                }
            },
            dns1: {
                validators: {
                    notEmpty: {
                        message: '<?php echo $this->lang->line('validform_null_preferred_dns_server')?>'
                    },
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    ip: {
                        message: '<?php echo $this->lang->line('validform_preferred_dns_server')?>'
                    }
                }
            },
            dns2: {
                validators: {
                    stringLength: {
                        max: 16,
                        message: 'The input string must be less than 16 characters long'
                    },
                    ip: {
                        message: '<?php echo $this->lang->line('validform_alternate_dns_server')?>'
                    }
                }
            },
        }
    })
    .on('success.form.bv', function(e) {
        //防止默认表单提交，采用ajax提交
        e.preventDefault();
        
        //设置IP
        $('#button_set_ip').removeAttr("disabled");
        $("#result").text("<?php echo $this->lang->line('network_result_success')?>");
        $("#result").removeClass().addClass("alert alert-warning");
        $("#result").fadeToggle("slow");
        $.ajax({
          url : "<?php echo base_url('index.php/management/set_ip');?>",
          type : "post",
              dataType : "json",
              data: "dhcp=" + $("input[name='dhcp']:checked").val()
            + "&ip=" + $("#inputdata1").val()
            + "&mask=" + $("#inputdata2").val()
            + "&gate=" + $("#inputdata3").val()
            + "&dns1=" + $("#inputdata4").val()
            + "&dns2=" + $("#inputdata5").val(),
          success : function(Results) {},
          error : function() {}
        });
    });

});
</script>