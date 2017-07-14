<!-- 设置结果显示框 -->
<div class="alert alert-success" id="result"></div>

<!-- 设置逆变器列表 -->
<form class="form-horizontal" method="ajax">
  <div class="form-group">    
    <div class="col-sm-offset-4 col-sm-4">
       <textarea class="form-control" name="ids" id="inverter_list" rows="12"><?php foreach ($ids as $value) {
          echo $value."\n";
        }?></textarea>
    </div>
  </div>
  <div class="form-group">
    <div class="col-sm-offset-4 col-sm-4">
      <div class="btn-group btn-group-justified">
        <div class="btn-group">
          <button class="btn btn-primary btn-sm" id="update_id" type="button"><?php echo $this->lang->line('button_update')?></button>
        </div>
        <div class="btn-group">
          <button class="btn btn-primary btn-sm" id="clear_id" type="button"><?php echo $this->lang->line('id_clear_id')?></button>
        </div>
      </div>
    </div>
  </div>
</form>

<script>
$(document).ready(function() 
{	
	// 获取光标位置
    function caret(node) {
        //node.focus(); 
        /* without node.focus() IE will returns -1 when focus is not on node */
        if(node.selectionStart) return node.selectionStart;
        else if(!document.selection) return 0;
        var c		= "\001";
        var sel	= document.selection.createRange();
        var dul	= sel.duplicate();
        var len	= 0;
        dul.moveToElementText(node);
        sel.text	= c;
        len		= (dul.text.indexOf(c));
        sel.moveStart('character',-1);
        sel.text	= "";
        return len;
    }

	// 设置光标位置
	function setCaretPosition(elem, caretPos) {
	    if(elem != null) {
	        if(elem.createTextRange) {
	            var range = elem.createTextRange();
	            range.move('character', caretPos);
	            range.select();
	        }
	        else {
	            if(elem.selectionStart) {
	                elem.focus();
	                elem.setSelectionRange(caretPos, caretPos);
	            }
	            else
	                elem.focus();
	        }
	    }
	}

	// 只返回数字（和回车）
	function onlyNumbers(e) {
	    var keynum;
	    var keychar;
	    var numcheck;

	    if(window.event) // IE
	    {
	        keynum = e.keyCode;
	    }
	    else if(e.which) // Netscape/Firefox/Opera
	    {
	        keynum = e.which;
	    }
	    keychar = String.fromCharCode(keynum);
	    if(keynum == 13) {
	    	return TRUE;
	    }
	    else {
		    numcheck = /\d/;
	    	return numcheck.test(keychar);
	    }	    
	}

	var textarea = document.getElementsByTagName('textarea')[0], // 获取页面上textarea的值
	    timer, // 计时器
	    fn = function () {
	        var cp = caret(textarea),
	            v = textarea.value, // 当前显示值
	            value = v.replace(/[^\d\n]/g, '').split('\n'), // 除去纯数字和回车以外的字符，并按回车分组存入数组
	            tmpValue = []; // 临时数组

	        //遍历数组，取前12位
	        for(var i = 0; i < value.length; i++) {
	            tmpValue.push(value[i].substr(0, 12));
	        }

	        //将数组组合为一个字符串，以回车符分隔
	        newValue = tmpValue.join('\n');

	        //如果当前最后一行到达12位，则加上回车符换行
	        if (tmpValue.pop().length == 12) {
	            newValue = newValue + '\n';
	        }

	        //更新页面显示
	        if (v != newValue) {
	            textarea.value = newValue;
	            if (cp < v.length) {
	                setCaretPosition(textarea, cp);
	            }
	        }        
	    };

	// 获得焦点时每隔100ms调用一次fn函数
	textarea.onfocus = function () {
	    timer = setInterval(fn, 100);
	};

	// 失去焦点时清除timer，取消对fn的调用
	textarea.onblur = function () {
	    clearInterval(timer);
	    fn();
	};

	// 按键被按下是检测是否为纯数字，否则无法输入
	textarea.onkeypress = function () {
	    return onlyNumbers(event);
	};
    
    //设置逆变器列表
    $("#update_id").click(function(){
    	$("#result").hide();    
        $.ajax({
    		url : "<?php echo base_url('index.php/management/set_id');?>",
    		type : "post",
            dataType : "json",
    		data: "ids=" + $("#inverter_list").val(),
  	    	success : function(Results){ 	    		
                if(Results.value == 0){
                	$("#result").text(Results.message + ' ' + Results.num);
  	                $("#result").removeClass().addClass("alert alert-success");
                    setTimeout('$("#result").fadeToggle("slow")', 3000);
                }
                else if (Results.value == 2) {
                    $("#result").text(Results.message);
                    $("#result").removeClass().addClass("alert alert-warning");
                }
                else {
                	$("#result").text(Results.message+Results.error_ids + ' ' + Results.num);
                    $("#result").removeClass().addClass("alert alert-warning");
                }
                $("#result").fadeToggle("slow");
                window.scrollTo(0,0);//页面置顶 
    		},
  	    	error : function() { alert("Error"); }
        })
    });

    //清空逆变器列表
    $("#clear_id").click(function(){
        $("#result").hide();
        $("#inverter_list").attr("value","");   	
        $.ajax({
    		url : "<?php echo base_url('index.php/management/set_id_clear');?>",
    		type : "post",
            dataType : "json",
    		data: "clear_id",
    		success : function(Results){
    			$("#result").text(Results.message);  		
                if(Results.value == 0){
  	                $("#result").removeClass().addClass("alert alert-success");
                    setTimeout('$("#result").fadeToggle("slow")', 3000);
                    $("#inverter_list").text("");
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