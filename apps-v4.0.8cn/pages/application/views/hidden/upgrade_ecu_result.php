<?php 
    if($value == 0)
        echo "<div class=\"alert alert-success\">".$this->lang->line('upgrade_ecu_result_0')."</div>";
    else
        echo "<div class=\"alert alert-danger\">".$this->lang->line('upgrade_ecu_result_1')."</div>";          
?>
<div class="col-sm-12">
    <p><?php echo "<!--".$result."-->";?></p>
<!--     <a href="upgrade_ecu">&lt;&lt;Back</a> -->
</div>
