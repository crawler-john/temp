<form class="form-horizontal" id="defaultForm" action="<?php echo base_url('index.php/hidden/exec_export_file');?>" method="post">
  <div class="form-group">    
    <label for="start" class="col-sm-5 control-label"><?php echo $this->lang->line('export_file_start_time')?></label>
    <div class="col-sm-4">
      <input id="start" class="form-control datepicker" type="text" name="start_time" value="<?php echo $start_time;?>" 
            onClick="WdatePicker({
                maxDate:'#F{$dp.$D(\'end\')}',
                dateFmt:'yyyy-MM-dd HH:mm:ss',
                <?php echo $this->lang->line('graph_language')?>})" 
            readonly>
    </div>
  </div>
  <div class="form-group">    
    <label for="end" class="col-sm-5 control-label"><?php echo $this->lang->line('export_file_end_time')?></label>
    <div class="col-sm-4">
      <input id="end" class="form-control datepicker" type="text" name="end_time" value="<?php echo $end_time;?>" 
            onClick="WdatePicker({
                minDate:'#F{$dp.$D(\'start\')}',
                maxDate:'%y-%M-%d',
                dateFmt:'yyyy-MM-dd HH:mm:ss',
                <?php echo $this->lang->line('graph_language')?>})" 
            readonly>
    </div>
  </div>
  <div class="form-group">
    <div class="col-sm-offset-5 col-sm-2">
      <button class="btn btn-primary btn-sm" type="submit"><?php echo $this->lang->line('export_file_export')?></button>
    </div>
  </div>
</form>

<script src="<?php echo base_url('resources/js/datepicker/WdatePicker.js');?>"></script>