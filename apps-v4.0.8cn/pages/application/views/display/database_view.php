<style>
.on{left:0px;}
.off{left:-320px;}
.show{visibility:visible;}
.hide{visibility:hidden;}
#hide_table{
    width:340px;
    height:400px; 
    background-color:transparent;
    position:fixed;
    top:165px;
}
#show_table{
    float:left;height:100%;
    width:320px;
    background-color: transparent; 
    border: 0;
    text-align:left;
/*    box-shadow: 2px 2px 3px #ccc;*/
}
#icon{
    float:right;
    width:15px;
    height:15px;
    margin-top:150px;
}

</style>

<div class="off" id="hide_table">
    <div id="show_table">
    <h3>表单列表</h3>
    <?php
    echo "<ul>";
        foreach ($table_name as $value) {
            echo "<li><a href=".base_url("index.php/display/$func/$value").">".$value."</a></li>";
        }
    echo "</ul>";
    ?>
    </div>
    <div id="icon"><img src="<?php echo base_url('resources/images/arrow.png');?>" width="30"></div>
</div>

<div class="table-responsive">
  <table class="table table-condensed table-striped table-hover table-bordered">
    <thead>
      <tr>
        <?php
            if(!empty($table_value))
            {
                foreach ($table_value[0] as $key => $value) {
                    echo "<th>".$key."</th>";
                }
            }
        ?>
      </tr>
    </thead>
    <tbody>
        <?php
            foreach ($table_value as $temp) {
                echo "<tr>";
                foreach ($temp as $value) {
                    if(strlen($value) > 50){
                        echo "<td>".wordwrap($value, 50, "<br>", true)."</td>";                        
                    }
                    else
                        echo "<td>".$value."</td>";
                }
                echo "</tr>";
            }
        ?>
    </tbody>
  </table>
</div>

<script>
    $(document).ready(
        function () {
            $('#icon').bind('mouseover', function () {
                $('#hide_table').removeClass("off");
                $('#hide_table').addClass("on");
                $('#icon').addClass("hide");
                $('#icon').removeClass("show");
            })
            $('#show_table').bind('mouseleave', function () {
                $('#hide_table').removeClass("on");
                $('#hide_table').addClass("off");
                $('#icon').removeClass("hide");
                $('#icon').addClass("show");
            })
        }
    ) 
</script>
