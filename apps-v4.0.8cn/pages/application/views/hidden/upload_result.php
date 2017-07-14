<?php 
    if($value == 0)
        echo "<div class=\"alert alert-success\">".$this->lang->line('upload_result_0')."</div>";
    else if($value == 1)
        echo "<div class=\"alert alert-danger\">".$this->lang->line('upload_result_1')."</div>";
    else{
        if($value == 2)
            echo "<div class=\"alert alert-warning\">".$this->lang->line('upload_result_2')."</div>";
    }            
?>
<div class="col-sm-12">
    <p><?php echo "<!--".$result."-->";?></p>
<!--     <a href="upload">&lt;&lt;Back</a> -->
</div>
