<div class="table-responsive">
  <table class="table table-condensed table-bordered">
    <thead>
      <tr>
        <th scope="col"><?php echo $this->lang->line('device_id')?></th>
        <th scope="col"><?php echo $this->lang->line('realtimedata_current_power')?></th>
        <th scope="col"><?php echo $this->lang->line('realtimedata_grid_frequency')?></th>
        <th scope="col"><?php echo $this->lang->line('realtimedata_grid_voltage')?></th>
        <th scope="col"><?php echo $this->lang->line('realtimedata_temperature')?></th>
        <th scope="col"><?php echo $this->lang->line('realtimedata_date')?></th>
      </tr>
    </thead>
    <tbody>
    <?php foreach($curdata as $key => $value): ?>
    <div>
      <?php $row = count($value[0]); ?>
      <tr <?php if ($key%2 == 0) {echo "class='active'";} ?>>
        <td><?php echo $value[0][0]; ?> </td>
        <td><?php echo $value[1][0]; ?> </td>
        <td rowspan=<?php echo $row; ?> style='vertical-align: middle;'><?php echo $value[2]; ?> </td>
        <td><?php echo $value[3][0]; ?> </td>
        <td rowspan=<?php echo $row; ?> style='vertical-align: middle;'><?php echo $value[4]; ?> </td>
        <td rowspan=<?php echo $row; ?> style='vertical-align: middle;'><?php echo $value[5]; ?> </td>
      </tr>
      <?php for ($i=1; $i<$row; $i++): ?>
      <tr <?php if ($key%2 == 0) {echo "class='active'";} ?>>
        <td><?php echo $value[0][$i]; ?> </td>
        <td><?php echo $value[1][$i]; ?> </td>
        <td><?php echo $value[3][$i]; ?> </td>
      </tr>
      <?php endfor; ?>
    </div>
    <?php endforeach; ?>
    </tbody>
  </table>
</div>