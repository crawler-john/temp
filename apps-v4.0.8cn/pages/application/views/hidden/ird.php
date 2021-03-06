<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result">
    <button class="close" type="button"><span aria-hidden="true">&times;</span></button>
    <strong></strong><small></small>
</div>

<!-- 电网环境设置表格 -->
<table class="table table-condensed table-striped table-hover table-bordered">
  <thead>
    <tr>
      <th class="col-xs-3"><?php echo $this->lang->line('device_id')?></th>
      <th class="col-xs-3"><?php echo $this->lang->line('ird_result')?></th>
      <th class="col-xs-4"><?php echo $this->lang->line('ird_setting')?></th>
      <th class="col-xs-1"></th>
    </tr>
  </thead>
  <tbody>
    <?php 
      foreach ($ids as $key => $value) 
      {
        echo "<tr>";
        
        //逆变器ID
        echo "<td>".$value[0]."</td>";

        //设置结果
        if($value["result"] === "1"){
            echo "<td>{$this->lang->line('ird_select_1')}</td>";}
        else if ($value["result"] === "2"){
            echo "<td>{$this->lang->line('ird_select_2')}</td>";}
        else if ($value["result"] === "3"){
            echo "<td>{$this->lang->line('ird_select_3')}</td>";}
        else{ 
            echo "<td>-</td>";}    

        //设置选项
        echo "<td><select name=\"{$value[0]}\" class=\"form-control input-sm\">
                    <option value=-1>{$this->lang->line('ird_select')}</option>
                    <option value=1>{$this->lang->line('ird_select_1')}</option>
                    <option value=2>{$this->lang->line('ird_select_2')}</option>
                    <option value=3>{$this->lang->line('ird_select_3')}</option>
                    </select></td>";
        
        //设置按钮
        echo "<td><button class=\"btn btn-default btn-sm\" id=\"{$value[0]}\" type=\"button\">".$this->lang->line('button_save')."</button></td>";
        echo "</tr>";
      }
    ?>  
  </tbody>
</table>

<div class="col-sm-3 col-sm-offset-3 col-xs-6 ">
<button class="btn btn-primary btn-sm" id="read_all" type="button"><?php echo $this->lang->line('button_read_all')?></button>
</div>

<div class="btn-group col-sm-3 col-xs-6">
  <button class="btn btn-primary btn-sm dropdown-toggle" type="button" data-toggle="dropdown">
  <?php echo $this->lang->line('button_save_all')?> <span class="caret"></span>
  </button>
  <ul class="dropdown-menu" id="save_all" role="menu">
    <li><a class="btn btn-xs" value="1"><?php echo $this->lang->line('ird_select_1')?></a></li>
    <li><a class="btn btn-xs" value="2"><?php echo $this->lang->line('ird_select_2')?></a></li>
    <li><a class="btn btn-xs" value="3"><?php echo $this->lang->line('ird_select_3')?></a></li>
  </ul>
</div>

<script>
$(document).ready(function() {

	//隐藏设置结果栏
	$("#result").hide();
	$(".close").click(function(){
		$("#result").hide();
    }); 

    //设置电网环境处理	
	$(".btn-default").click(function(){
		$("#result").hide();
		   
	    $.ajax({
    		url : "<?php echo base_url('index.php/hidden/set_ird');?>",
    		type : "post",
            dataType : "json",
    		data: "id=" + $(this).attr("id")
			  + "&ird=" + $("select[name='"+$(this).attr("id")+"']").val(),
  	    	success : function(Results){
  	  	    	//alert(Results);
                if(Results.value == 0){
  	                $("#result").removeClass().addClass("alert alert-success");
  	                $("#result strong").text("<?php echo $this->lang->line('message_success')?>" + "：");  
  	            }
                else{
                    $("#result").removeClass().addClass("alert alert-warning");
                    $("#result strong").text("<?php echo $this->lang->line('message_warning')?>" + "：");
                }
                $("#result small").text(Results.message);        		 
            	$("#result").show();
    		},
  	    	error : function(){
  	    		alert("Error");
  	    	}
        })
        window.scrollTo(0,0);//页面置顶
    });

	//设置所有逆变器
	$("#save_all li a").click(function(){
		$("#result").hide();
		
	    $.ajax({
    		url : "<?php echo base_url('index.php/hidden/set_ird');?>",
    		type : "post",
            dataType : "json",
    		data: "id=ALL" 
        		  + "&ird=" + $(this).attr("value"),
  	    	success : function(Results){
                if(Results.value == 0){
  	                $("#result").removeClass().addClass("alert alert-success");
  	                $("#result strong").text("<?php echo $this->lang->line('message_success')?>" + "：");  
  	            }
                else{
                    $("#result").removeClass().addClass("alert alert-warning");
                    $("#result strong").text("<?php echo $this->lang->line('message_warning')?>" + "：");
                }
                $("#result small").text(Results.message);        		 
            	$("#result").show();
    		},
  	    	error : function(){
  	    		alert("Error");
  	    	}
        })
        window.scrollTo(0,0);//页面置顶
	});

	//读取所有逆变器的IRD
	$("#read_all").click(function(){
		$("#result").hide();
		
	    $.ajax({
    		url : "<?php echo base_url('index.php/hidden/read_ird');?>",
    		type : "post",
            dataType : "json",
    		data: "read_all",
  	    	success : function(Results){
                if(Results.value == 0){
  	                $("#result").removeClass().addClass("alert alert-success");
  	                $("#result strong").text("<?php echo $this->lang->line('message_success')?>" + "：");  
  	            }
                else{
                    $("#result").removeClass().addClass("alert alert-warning");
                    $("#result strong").text("<?php echo $this->lang->line('message_warning')?>" + "：");
                }
                $("#result small").text(Results.message);
            	$("#result").show();
    		},
  	    	error : function(){
  	    		alert("Error");
  	    	}
        })
        window.scrollTo(0,0);//页面置顶
	});
	
});
</script>