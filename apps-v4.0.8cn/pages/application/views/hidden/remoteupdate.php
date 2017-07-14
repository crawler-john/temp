<input type='radio' name='select_id' value='0' /><?php echo $this->lang->line('all_inverter')?>
<input type='radio' name='select_id' value='1' checked='checked' /><?php echo $this->lang->line('part_of_inverter')?><br>
<table class="table table-condensed table-striped table-hover table-bordered">
    <thead>
      <tr>
        <th class="col-xs-4"><input type='checkbox' id='ids_all' /><?php echo $this->lang->line('device_id')?></th>
        <th class="col-xs-4"><?php echo $this->lang->line('home_version') ?></th>
        <th class="col-xs-4"><?php echo $this->lang->line('remoteupdate_flag') ?></th>
      </tr>
    </thead>
    <tbody>
    <?php foreach ($ids as $key => $value): ?>
        <tr>
            <td>
                <input type='checkbox' name='ids' value='<?php echo $value[0]; ?>' />
                <?php echo $value[0]; ?>
            </td>
            <td><?php echo $value[1]; ?></td>
            <td><?php echo $value[2]; ?></td>
        </tr>
    <?php endforeach; ?>
    </tbody>
</table>

<button class='submit' id='remoteupdate'><?php echo $this->lang->line('remote_update')?></button><br>
<div id='divload'></div>

<script src="<?php echo base_url('resources/js/jquery-1.11.3.min.js'); ?>"></script>
<script>
$(function(){

	$('#ids_all').on('click', function(){
		if(this.checked)
			$("input[name='ids']").each(function(){this.checked=true;});
		else
			$("input[name='ids']").each(function(){this.checked=false;});
	});

	/* 设置全局Ajax默认选项 */
	// $.ajaxSetup({
 //        type:"POST",
	// 	dataType: "json",
	// 	success: function(Result){
	// 		if(Result.success) {
	// 			alert('设置成功！');
	// 		}
	// 		else{
	// 			alert('设置失败 !');
	// 		}			
	// 	},
	// 	error: function(jqXHR){     
	// 	   alert("发生错误：" + jqXHR.status);  
	// 	}
 //    });

	/* 设置 */
	$('#remoteupdate').on('click', function(){
			$(document).ajaxStart(function(){
        $("#divload").html("正在设置标志位...");
    });
	$(document).ajaxStop(function(){
        $("#divload").html("标志位设置完成！");
    });

		 var ids = new Array();
        var scope = $("input:radio[name='select_id']:checked").val();
        $("input:checkbox[name='ids']:checked").each(function(){    
        	ids.push($(this).val());    
        });
        if(ids.length == 0 && scope == 1){
            alert("请选择至少一个逆变器！");
        	return false;
        }
		$.ajax({

			       type:"POST",
				dataType: "json",

		    url: '<?php echo base_url('index.php/hidden/set_remoteupdate');?>',
		    data: {
			    ids: ids,
			    scope: scope
			},
		success: function(Result){
			if(Result.success) {
				alert('设置成功！');
			}
			else{
				alert('设置失败 !');
			}			
		},
		error: function(jqXHR){     
		   alert("发生错误：" + jqXHR.status);  
		}

		});
	});
	
});
</script>