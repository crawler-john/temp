<form class="form-horizontal">
  <div class="form-group">    
    <label class="col-sm-5 control-label"><?php echo $this->lang->line('language_current_language')?></label>
    <div class="col-sm-4">
      <select name="language" class="form-control" id="lang_value">";
        <option value="english" <?php if(!strncmp($language, "english", 7))
                                    echo "selected=\"selected\"";
                                ?>><?php echo $this->lang->line('language_english');?></option>
        <option value="chinese" <?php if(!strncmp($language, "chinese", 7))
                                    echo "selected=\"selected\"";
                                ?>><?php echo $this->lang->line('language_chinese');?></option>
      </select>
    </div>
  </div>
  <div class="form-group">
    <div class="col-sm-offset-5 col-sm-2">
      <button class="btn btn-primary btn-sm" id="button_update" type="button"><?php echo $this->lang->line('button_update')?></button>
    </div>
  </div>
</form>

<script>
$(document).ready(function() {
    //切换语言
    $("#button_update").click(function(){
        $.ajax({
      		url : "<?php echo base_url('index.php/management/set_language');?>",
      		type : "post",
      	    dataType : "json",
      		data: "language=" + $("#lang_value").val(),
        	success : function(Results){ },
        	error : function() { }
	    })
        setTimeout("location.reload();",500);//刷新页面
  });
});
</script>