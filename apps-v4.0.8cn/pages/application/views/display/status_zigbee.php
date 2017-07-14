<form class="form-horizontal" action="<?php echo base_url('index.php/display/status_zigbee');?>" method="post">
  <div class="col-sm-4 col-sm-offset-4">
    <div class="input-group input-group-sm">
        <input class="form-control datepicker" type="text" name="date" value="<?php echo $date; ?>" onClick="WdatePicker({maxDate:'%y-%M-%d', <?php echo $this->lang->line('graph_language')?>})" readonly>
        <span class="input-group-btn">
            <button class="btn btn-default" id="query" type="submit"><?php echo $this->lang->line('button_query')?></button>
        </span>
    </div>
  </div>
</form>
<table class="table table-condensed table-striped table-hover table-bordered">
    <thead>
      <tr>
        <th><?php echo $this->lang->line('display_status_event_id')?></th>
        <th><?php echo $this->lang->line('display_status_event')?></th>
        <th><?php echo $this->lang->line('device_id')?></th>
        <th><?php echo $this->lang->line('display_status_date')?></th>
      </tr>
    </thead>
    <tbody>
    <?php
        foreach ($status as $key => $value) 
        {
            echo "<tr>";
            echo "<td>".++$key."</td>";
            echo "<td>".$this->lang->line("display_status_zigbee_{$value['event']}")."</td>";
            echo "<td>".$value['inverter_id']."</td>";
            echo "<td>".$value['date']."</td>";
            echo "</tr>";
        }
    ?>
    </tbody>
</table>

<script src="<?php echo base_url('resources/js/datepicker/WdatePicker.js');?>"></script>
