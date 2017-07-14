<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result"></div>

<!-- 最大功率设置表格 -->
<table class="table table-condensed table-striped table-hover table-bordered">
<thead>
<tr>
<th><?php echo $this->lang->line('device_id')?></th>      
      <th><?php echo $this->lang->line('maxpower_actual_maxpower')?></th>
      <th><?php echo $this->lang->line('maxpower_maxpower')?></th>
      <th></th>
    </tr>
  </thead>
  <tbody>
    <?php 
      foreach ($ids as $key => $value) 
      {
        echo "<tr>";
        echo "<td>".$value[0]."</td>";
        echo "<td>".$value[1]."</td>";
        echo "<td class=\"col-sm-4\"><div class=\"input-group input-group-sm col-sm-8\">
          <input class=\"form-control\" type=\"text\" name=\"{$value[0]}\" value=\"\">
            <span class=\"input-group-addon\">W</span>
            </div></td>";
        echo "<td><button class=\"btn btn-default btn-sm btn-save\" id=\"{$value[0]}\" type=\"submit\">".$this->lang->line('button_save')."</button></td>";
        echo "</tr>";
      }
    ?>
  </tbody>
</table>
<div class="col-xs-2 col-xs-offset-5">
    <button class="btn btn-primary btn-sm" id="read_all" type="button"><?php echo $this->lang->line('button_read_all')?></button>
</div>

<script>
$(document).ready(function() {
    //设置DC处理	
	$(".btn-save").click(function(){
		$("#result").hide();
	    $.ajax({
    		url : "<?php echo base_url('index.php/hidden/set_dc');?>",
    		type : "post",
            dataType : "json",
    		data: "id=" + $(this).attr("id")
			  + "&dc=" + $("input[name='"+$(this).attr("id")+"']").val(),
            success : function(Results){
            	$("#result").text(Results.message);
                if(Results.value == 0){
                    $("#result").removeClass().addClass("alert alert-success");
                    setTimeout('$("#result").fadeToggle("slow")', 3000);
                }
                else{
                    $("#result").removeClass().addClass("alert alert-warning");
                }
                $("#result").fadeToggle("slow");
                window.scrollTo(0,0);//页面置顶 
            },
            error : function() { alert("Error"); }
        })
    });
    
    //读取所有逆变器DC	
	$("#read_all").click(function(){
		$("#result").hide();			
	    $.ajax({
    		url : "<?php echo base_url('index.php/hidden/read_dc');?>",
    		type : "post",
            dataType : "json",
            data: "read_all",
            success : function(Results){
            	$("#result").text(Results.message);
                if(Results.value == 0){
                    $("#result").removeClass().addClass("alert alert-success");
                    setTimeout('$("#result").fadeToggle("slow")', 3000);
                }
                else{
                    $("#result").removeClass().addClass("alert alert-warning");
                }
                $("#result").fadeToggle("slow");
                window.scrollTo(0,0);//页面置顶 
            },
            error : function() { alert("Error"); }
        })
    });
});
</script>