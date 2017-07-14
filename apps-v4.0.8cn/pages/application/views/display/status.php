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
            echo "<td>".$this->lang->line("display_status_event_{$value['event']}")."</td>";
            echo "<td>".$value['inverter_id']."</td>";
            echo "<td>".$value['date']."</td>";
            echo "</tr>";
        }
    ?>
    </tbody>
</table>
