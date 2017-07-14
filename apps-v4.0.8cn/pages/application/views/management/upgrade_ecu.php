<form class="form-horizontal" action="exec_upgrade_ecu" method="post" enctype="multipart/form-data">
   <div class="form-group">    
        <label for="file" class="col-sm-5 control-label"><?php echo $this->lang->line('upgrade_ecu_filename');?></label>
        <div class="col-sm-4">
            <div class="input-group">
                <input type="file" name="file" id="file" style="display:none" onchange="filesize(this)" />
                <div class="input-group">
                <input class="form-control" id="path" readonly>
                <span class="input-group-addon btn_upgrade_ecu" onclick="document.getElementById('file').click()"><?php echo $this->lang->line('upgrade_ecu_browse');?></span>
                </div>  
            </div>
        </div>
    </div>
    <div class="form-group">
        <div class="col-sm-offset-5 col-sm-5">
          <input class="btn btn-primary btn-sm" id="btn_upgrade_ecu" type="submit" name="submit" value="<?php echo $this->lang->line('button_ok');?>" />
        </div>
    </div>
</form>

<script>
function filesize(ele) {
	document.getElementById('path').value=ele.value;
	if((ele.files[0].size / 1048576).toFixed(2) > 8) {
		$("#btn_upgrade_ecu").attr("disabled", true);
		alert("<?php echo $this->lang->line('upgrade_ecu_result_2');?>");
	}
	else{
		$("#btn_upgrade_ecu").attr("disabled", false);
	}    
}
</script>