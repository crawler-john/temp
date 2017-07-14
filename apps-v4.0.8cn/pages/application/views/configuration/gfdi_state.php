<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result"></div>

<!-- GFDI设置表单 -->
<form class="form-horizontal" id="defaultForm" method="ajax">
  <table class="table table-condensed table-striped table-hover table-bordered">
    <thead>
      <tr>
        <th class="col-xs-4"><?php echo $this->lang->line('device_id')?></th>
        <th class="col-xs-4"><?php echo $this->lang->line('gfdi_state')?></th>
        <th class="col-xs-4"><?php echo $this->lang->line('gfdi_unlock')?></th>
      </tr>
    </thead>
    <tbody>
    <?php
      foreach ($ids as $key => $value) 
      {
        echo "<tr>";
        echo "<td>".$value."</td>";
        echo "<td>".$gfdi_state[$key]."</td>";
        echo "<td><input type=\"checkbox\" name=\"ids\" value=\"".$value."\"></td>";
        echo "</tr>";
      }
    ?>
    </tbody>
  </table>
    <div class="form-group">
      <div class="col-sm-offset-5 col-sm-2">
        <button class="btn btn-primary btn-sm" id="gfdi_unlock" type="submit"><?php echo $this->lang->line('gfdi_unlock')?></button>
      </div>
    </div>
</form>

<script>
$(document).ready(function() 
{
 	$('#gfdi_unlock').removeAttr("disabled"); 
	//表单验证
    $('#defaultForm').bootstrapValidator({
    })
    .on('success.form.bv', function(e) {
        //防止默认表单提交，采用ajax提交
        e.preventDefault();
    });

    //设置表单处理
    $("#gfdi_unlock").click(function(){
    	$("#result").hide();
        //保存选中的逆变器ID
        var ids = new Array();
        $('input[name="ids"]:checked').each(function(){    
        	ids.push($(this).val());    
        });    
        
	    $.ajax({
    		url : "<?php echo base_url('index.php/configuration/set_gfdi_state');?>",
    		type : "post",
            dataType : "json",
    		data: {"ids":ids},
            success : function(Results){
            	$("#result").text(Results.message);
                if(Results.value == 0){
                    $("#result").removeClass().addClass("alert alert-success");
                    setTimeout('$("#result").fadeToggle("slow")', 3000);
                    setTimeout('$("#gfdi_unlock").removeAttr("disabled")', 3000);
                }
                else{
                    $("#result").removeClass().addClass("alert alert-warning");
            		$('#gfdi_unlock').removeAttr("disabled"); 
                }
                $("#result").fadeToggle("slow");
                window.scrollTo(0,0);//页面置顶
            },
            error : function() { alert("Error"); }
        })
    });
});
</script>