<form class="form-horizontal">
  <div class="form-group">
    <label class="col-sm-5 control-label"><?php echo $this->lang->line('channel_now')?></label>
    <div class="col-sm-4">
      <p class="form-control-static"><?php echo $channel;?></p>
    </div>
  </div>
</form>
<form id="form-channel" class="form-horizontal" 
action="<?php echo base_url('index.php/hidden/set_channel');?>" method="post">
  <div class="form-group">
    <label for="old_channel" class="col-sm-5 control-label"><?php echo $this->lang->line('old_channel')?></label>
    <div class="col-sm-4">
      <select id="old_channel" class="form-control" name="old_channel">
        <option value="0"> Unknown </option>
        <option value="11"> 0x0B </option>
        <option value="12"> 0x0C </option>
        <option value="13"> 0x0D </option>
        <option value="14"> 0x0E </option>
        <option value="15"> 0x0F </option>
        <option value="16"> 0x10 </option>
        <option value="17"> 0x11 </option>
        <option value="18"> 0x12 </option>
        <option value="19"> 0x13 </option>
        <option value="20"> 0x14 </option>
        <option value="21"> 0x15 </option>
        <option value="22"> 0x16 </option>
        <option value="23"> 0x17 </option>
        <option value="24"> 0x18 </option>
        <option value="25"> 0x19 </option>
        <option value="26"> 0x1A </option>
      </select>
    </div>
  </div>
  <div class="form-group">
    <label for="new_channel" class="col-sm-5 control-label"><?php echo $this->lang->line('new_channel')?></label>
    <div class="col-sm-4">
      <select id="new_channel" class="form-control" name="new_channel">
        <option value="11"> 0x0B </option>
        <option value="12"> 0x0C </option>
        <option value="13"> 0x0D </option>
        <option value="14"> 0x0E </option>
        <option value="15"> 0x0F </option>
        <option value="16"> 0x10 </option>
        <option value="17"> 0x11 </option>
        <option value="18"> 0x12 </option>
        <option value="19"> 0x13 </option>
        <option value="20"> 0x14 </option>
        <option value="21"> 0x15 </option>
        <option value="22"> 0x16 </option>
        <option value="23"> 0x17 </option>
        <option value="24"> 0x18 </option>
        <option value="25"> 0x19 </option>
        <option value="26"> 0x1A </option>
      </select>
    </div>
  </div>

  <?php if ($ids): ?>
  <table class="table table-condensed table-striped table-hover table-bordered">
    <thead>
      <tr>
        <th><input type='checkbox' id="checkall" onclick="checkId();"/></th>
        <th class="col-xs-4"><?php echo $this->lang->line('device_id')?></th>
        <th class="col-xs-4"><?php echo $this->lang->line('short_addr')?></th>
        <th class="col-xs-4"><?php echo $this->lang->line('bind_flag')?></th>        
      </tr>
    </thead>
    <tbody>
    <?php foreach ($ids as $key => $value): ?>
        <tr>
            <?php if ($value[3]): ?>
              <td><input type='checkbox' name='id[]' value='<?php echo $value[0]; ?>' checked=true /></td>
            <?php else: ?>
              <td><input type='checkbox' name='id[]' value='<?php echo $value[0]; ?>' /></td>
            <?php endif; ?>
            <td><?php echo $value[0]; ?></td>
            <td><?php echo $value[1]; ?></td>
            <td><?php echo $value[2]; ?></td>
        </tr>
    <?php endforeach; ?>
    </tbody>
  </table>
  <?php endif; ?>

  <div class="form-group">
    <div class="col-sm-offset-5 col-sm-4">
      <button type="submit" class="btn btn-primary btn-sm"><?php echo $this->lang->line('button_ok')?></button>
    </div>
  </div>
  <div class="form-group">
    <div class="col-sm-offset-2 col-sm-8">
      <div id="divload"></div>
    </div>
  </div>
</form>

<script>
  function checkId() { 
    var checkCtrl = document.getElementById('checkall'); 
    var checkObj = document.getElementsByName('id[]'); 
    if(checkCtrl.checked) { 
      for(var i=0; i<checkObj.length; i++)
        checkObj[i].checked=true; 
    } 
    else { 
      for(var i=0; i<checkObj.length; i++)
        checkObj[i].checked=false; 
    } 
  }

// 注:需要在加载jquery之后加载
// $(document).ajaxStart(function(){
//     $("#divload").html("正在请求数据...");
// });
// $(document).ajaxStop(function(){
//     $("#divload").html("数据请求完成！");
// });

function ajaxSubmit(form, fn){
  $(document).ajaxStart(function(){
    $("#divload").html("正在请求数据...");
});
$(document).ajaxStop(function(){
    $("#divload").html("数据请求完成！");
});

    $.ajax({
        url: form.action,
        type: form.method,
        data: $(form).serialize(),
      dataType: "json",
    success: fn,
    error: function(jqXHR){     
       alert("发生错误：" + jqXHR.status);
    }
    });
}

$(function(){
  $("#old_channel").val("<?php echo $old_channel;?>");
  $("#new_channel").val("<?php echo $new_channel;?>");
  $('#form-channel').submit(function(){
    ajaxSubmit(this, function(data){
      //alert(data.result);
      location.reload();
    });
    return false;
  });
});
</script>