<?php
  if("" !== $result){
    if($result == 0){
      echo "<div class=\"alert alert-success alert-dismissible\" role=\"alert\">"."\n";
      echo "<button type=\"button\" class=\"close\" data-dismiss=\"alert\"><span aria-hidden=\"true\">&times;</span><span class=\"sr-only\">Close</span></button>"."\n";
      echo "<strong>".$this->lang->line("initialize_success")."&nbsp;!&nbsp;&nbsp;</strong>"."\n";
      echo "</div>"."\n";
    }
    else{
      echo "<div class=\"alert alert-danger alert-dismissible\" role=\"alert\">"."\n";
      echo "<button type=\"button\" class=\"close\" data-dismiss=\"alert\"><span aria-hidden=\"true\">&times;</span><span class=\"sr-only\">Close</span></button>"."\n";
      echo "<strong>".$this->lang->line("initialize_failed")."&nbsp;!&nbsp;&nbsp;</strong>"."\n";
      echo "</div>"."\n";
    }
  }
?>
<?php echo form_open('hidden/exec_initialize');?>
  <div class="form-group">    
    <div class="col-sm-12">
      <button type="submit" class="btn btn-primary btn-sm"><?php echo $this->lang->line('initialize_clear_energy')?></button>
    </div>
  </div>
</from>
