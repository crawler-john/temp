<div class="table-responsive">
  <table class="table table-condensed table-striped table-hover table-bordered">
    <thead>
      <tr>
        <th scope="col"><?php echo $this->lang->line('device_id')?></th>
        <th scope="col"><?php echo $this->lang->line('status_channel')?></th>
        <th scope="col"><?php echo $this->lang->line('status_status')?></th>
        <th scope="col"><?php echo $this->lang->line('status_energy')?></th>
      </tr>
    </thead>
    <tbody>
    <?php
      foreach ($ids as $value){
        echo "<tr>
                <td>$value</td>
                <td></td>
                <td></td>
                <td></td>
              </tr>";
      }
    ?>
    </tbody>
  </table>
</div>

<script>
function myrefresh(){
   window.location.reload();
}
setTimeout('myrefresh()',300000); //指定5分钟刷新一次
</script>