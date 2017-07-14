<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result"></div>

<!-- 设置GPRS -->
<form class="form-horizontal">
<fieldset>
    <legend><?php echo $this->lang->line('network_set_gprs')?></legend>   

    <div class="form-group">    
      <div class="col-sm-4 col-sm-offset-4">
        <input id="gprs_status" type="checkbox" name="gprs" 
        <?php if ($gprs==1) { echo "checked='checked'"; }?>>
        <?php echo $this->lang->line('network_use_gprs')?>
      </div>
    </div>
  </fieldset>

  <div class="form-group">
    <div class="col-sm-offset-4 col-sm-2">
      <button class="btn btn-primary btn-sm" id="button_update_gprs" type="button"><?php echo $this->lang->line('button_update')?></button>
    </div>
  </div>
</form>

<script>
$(document).ready(function() 
{
    //设置GPRS
    $("#button_update_gprs").click(function(){
    	$("#result").hide();     	
	    $.ajax({
    		url : "<?php echo base_url('index.php/hidden/set_gprs');?>",
    		type : "post",
            dataType : "json",
    		data: "gprs=" + $('#gprs_status').is(':checked'),
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
