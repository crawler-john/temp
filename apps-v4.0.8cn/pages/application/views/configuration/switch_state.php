<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result"></div>

<!-- 开关机设置表单 -->
<form class="form-horizontal" id="defaultForm" method="ajax">
  <table class="table table-condensed table-striped table-hover table-bordered">
    <thead>
      <tr>
        <th class="col-xs-3"><?php echo $this->lang->line('device_id')?></th>
        <th class="col-xs-3"><?php echo $this->lang->line('switch_state')?></th>
        <th class="col-xs-3"><?php echo $this->lang->line('switch_turn_on')?></th>
        <th class="col-xs-3"><?php echo $this->lang->line('switch_turn_off')?></th>
      </tr>
    </thead>
    <tbody>
      <?php 
        foreach ($ids as $key => $value) 
        {
            echo "<tr>";
            echo "<td>".$value."</td>";
            echo "<td>".$switch_state[$key]."</td>";
            echo "<td><input type=\"radio\" name=\"ids[".$key."]\" value=\"".$value."1"."\" onclick='check(this)'></td>";
            echo "<td><input type=\"radio\" name=\"ids[".$key."]\" value=\"".$value."2"."\" onclick='check(this)'></td>";
            echo "</tr>";
        }
      ?>
    </tbody>
  </table>
  <div class="col-sm-offset-5 col-sm-4">
    <div class="btn-group">
      <button class="btn btn-primary btn-sm" id="switch_turn_on_off" type="submit"><?php echo $this->lang->line('switch_turn_on_off')?></button>
      <button type="button" class="btn btn-primary btn-sm dropdown-toggle" data-toggle="dropdown">
        <span class="caret"></span>
      </button>
      <ul class="dropdown-menu" role="menu">
        <li><a class="btn btn-sm" id="switch_turn_on_all"><?php echo $this->lang->line('switch_turn_on_all')?></a></li>
        <li><a class="btn btn-sm" id="switch_turn_off_all"><?php echo $this->lang->line('switch_turn_off_all')?></a></li>
      </ul>
    </div>
  </div>
</form>

<script>
//取消单选按钮选中
var tempradio = null;    
function check(checkedRadio)    
{    
    if(tempradio == checkedRadio)
    {  
        tempradio.checked = false;  
        tempradio = null;  
    }   
    else
    {  
        tempradio= checkedRadio;    
    }  
} 

$(document).ready(function() 
{	
	$('#switch_turn_on_off').removeAttr("disabled"); 
	//表单验证
    $('#defaultForm').bootstrapValidator({
    })
    .on('success.form.bv', function(e) {
        //防止默认表单提交，采用ajax提交
        e.preventDefault();
    });

    //设置表单处理(指定开关机)
    $("#switch_turn_on_off").click(function(){
    	$("#result").hide();    	
        //保存选中的逆变器ID
        var ids = new Array();
        $('input[type="radio"]:checked').each(function(){    
        	ids.push($(this).val());    
        });
        
	    $.ajax({
    		url : "<?php echo base_url('index.php/configuration/set_switch_state');?>",
    		type : "post",
            dataType : "json",
    		data: {"ids":ids},
            success : function(Results){
            	$("#result").text(Results.message);
                if(Results.value == 0){
                    $("#result").removeClass().addClass("alert alert-success");
                    setTimeout('$("#result").fadeToggle("slow")', 3000);
                    setTimeout('$("#switch_turn_on_off").removeAttr("disabled")', 3000);
                }
                else{
                    $("#result").removeClass().addClass("alert alert-warning");
            		$('#switch_turn_on_off').removeAttr("disabled"); 
                }
                $("#result").fadeToggle("slow");
                window.scrollTo(0,0);//页面置顶 
            },
            error : function() { alert("Error"); }
        })
    });
    
    //设置表单处理(打开所有)
    $("#switch_turn_on_all").click(function(){
    	$("#result").hide();
        
	    $.ajax({
    		url : "<?php echo base_url('index.php/configuration/set_switch_all_on');?>",
    		type : "post",
            dataType : "json",
    		data: "switch_turn_on_all",
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
    
    //设置表单处理(关闭所有)
    $("#switch_turn_off_all").click(function(){
    	$("#result").hide();
    	
	    $.ajax({
    		url : "<?php echo base_url('index.php/configuration/set_switch_all_off');?>",
    		type : "post",
            dataType : "json",
    		data: "switch_turn_off_all",
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