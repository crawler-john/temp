<form class="form-horizontal" >
    <div class="col-sm-6 col-sm-offset-3">
        <div class="input-group input-group-sm">
            <span class="input-group-addon" id="show_period" style="background-color:transparent;"><?php echo $this->lang->line('energy_weekly'); ?></span>
            <input class="form-control datepicker" type="text" name="date" value="<?php echo date("Y-m-d",time());?>" onClick="WdatePicker({maxDate:'%y-%M-%d', <?php echo $this->lang->line('graph_language')?>, onpicked:select_date})" readonly>
            <div class="input-group-btn">
                <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown"><?php echo $this->lang->line('button_query')?> <span class="caret"></span></button>
                <ul class="dropdown-menu dropdown-menu-right">
                    <li><a href="javascript:select_period('weekly')"><?php echo $this->lang->line('energy_weekly'); ?></a></li>
                    <li><a href="javascript:select_period('monthly')"><?php echo $this->lang->line('energy_monthly'); ?></a></li>
                    <li><a href="javascript:select_period('yearly')"><?php echo $this->lang->line('energy_yearly'); ?></a></li>
                </ul>
            </div>
        </div>
    </div>
</form>

<!-- 显示图表 -->
<div class="col-sm-12 mychart">
    <div id="myChart"></div>
</div>

<!-- 总能量 -->
<h4><span class="label label-warning pull-right" id="total"><?php echo $this->lang->line("graph_weekly_energy").": ".$total_energy;?> kWh</span></h4>

<?php
    $x_label = "";
    $y_value = "";
    foreach ($energy as $key => $value) {
        $x_label = $x_label."\"".$value['date']."\",";
        $y_value = $y_value.$value['energy'].",";
    }
?>

<script src="<?php echo base_url('resources/js/datepicker/WdatePicker.js');?>"></script>
<script src="<?php echo base_url('resources/js/highcharts.js');?>"></script>
<script src="<?php echo base_url('resources/js/modules/exporting.js');?>"></script>
<script>
//window.scrollTo(0,110);//页面位置调整（显示完整图表）
$(document).ready(function(){
    Highcharts.setOptions({
        global: { //全局配置
            useUTC: false 
        },
        lang: {
        	contextButtonTitle: "<?php echo $this->lang->line('contextButtonTitle'); ?>",
        	downloadJPEG: "<?php echo $this->lang->line('downloadJPEG'); ?>",
            downloadPDF: "<?php echo $this->lang->line('downloadPDF'); ?>",
            downloadPNG: "<?php echo $this->lang->line('downloadPNG'); ?>",
            downloadSVG: "<?php echo $this->lang->line('downloadSVG'); ?>",
            printChart:" <?php echo $this->lang->line('printChart'); ?>",
        },
        chart: { //图表通用参数设置
        	type: 'column',
            backgroundColor: 'rgba(255,255,255,0)', //设置透明背景
            //height: 400, //图表高度(默认是400px)
            //zoomType: 'x', //可沿着X轴放大
            spacing: [10, 10, 5, 0], //上右下左[外边框和绘图区之间的距离]
        },
        title: {
            text: '<?php echo $this->lang->line('graph_title_energy'); ?>'
        },
        subtitle: { //副标题设置
            y: 200,
            style: {
                fontSize: "16px",
            }
        },
        xAxis: {
            labels: {
                maxStaggerLines: 1, //横坐标只显示一行
            },            
        },
        yAxis: { //Y坐标轴设置
        	//gridLineDashStyle: 'longdash', //设置网格线样式
            title: {
            	text: '<?php echo $this->lang->line('graph_y_label_energy'); ?>',
                style: {
                    fontSize: "16px",
                },
                //align: 'high', offset: -45, rotation: 0, y: -15
            },
            min:0,
        },
        tooltip: { //数据点提示框设置
        	borderWidth: 0,
            snap: 0,
            hideDelay:0, //提示框隐藏延时为0
            valueSuffix: ' kWh',
        },
        legend: { //图例说明
            enabled: false,
        },
        credits: { //图表版权信息
            enabled: false
        },
        plotOptions: { //数据点配置     
        },        
    });

    /* 创建图表 */
    $('#myChart').highcharts({
        subtitle: {
        	text: '<?php echo $subtitle; ?>',
        },
        xAxis: {
            categories: [<?php echo $x_label; ?>],
        },
        series: [{
            name: '<?php echo $this->lang->line('graph_value_energy'); ?>',
            data: [<?php echo $y_value; ?>],
        }]
    });
});
var data_period = 'weekly'; //默认查询一周
function select_period(period)
{
	if(period == 'yearly'){
	    $("#show_period").html("<?php echo $this->lang->line('energy_yearly'); ?>");
	}
	else if(period == 'monthly'){
	    $("#show_period").html("<?php echo $this->lang->line('energy_monthly'); ?>");
	}
	else{
		$("#show_period").html("<?php echo $this->lang->line('energy_weekly'); ?>");
	}
	data_period = period;
    show_graph();
}
function select_date()
{
	$(".datepicker").blur();
	show_graph();
}
function show_graph()
{
	$.ajax({
		url : "<?php echo base_url('index.php/realtimedata/old_energy_graph');?>",
		type : "post",
        dataType : "json",
        data: "date=" + $(".datepicker").val() 
          + "&period=" + data_period,
        success : function(Results){
            //整理数据
        	var data_date = new Array();
        	var data_energy = new Array();
        	$.each(Results.energy,function(key,value){   
        		data_date.push(value.date);
        		data_energy.push(value.energy);
     		});
     		//重新绘制图表
            if(data_period == 'yearly'){ //最近一年
                $('#myChart').highcharts({
                    subtitle: {
                    	text: Results.subtitle
                    },
                    xAxis: {
                        categories: data_date
                    },
                    series: [{
                        name: '<?php echo $this->lang->line('graph_value_energy'); ?>',
                        data: data_energy,
                    }]
                });
                $('#total').text("<?php echo $this->lang->line('graph_yearly_energy');?>: " + Results.total_energy + " kWh");
            }
            else if(data_period == 'monthly'){
            	$('#myChart').highcharts({
                    subtitle: {
                    	text: Results.subtitle
                    },
                    xAxis: {
                        categories: data_date,
                        labels: {
                            step: 2,
                            rotation: -45
                        }
                    },
                    series: [{
                        name: '<?php echo $this->lang->line('graph_value_energy'); ?>',
                        data: data_energy,
                    }]
                });
                $('#total').text("<?php echo $this->lang->line('graph_monthly_energy');?>: " + Results.total_energy + " kWh");
            }
            else{
            	$('#myChart').highcharts({
                    subtitle: {
                    	text: Results.subtitle
                    },
                    xAxis: {
                        categories: data_date
                    },
                    series: [{
                        name: '<?php echo $this->lang->line('graph_value_energy'); ?>',
                        data: data_energy,
                    }]
                });
            	$('#total').text("<?php echo $this->lang->line('graph_weekly_energy');?>: " + Results.total_energy + " kWh");
            }
        },
        error : function() { alert("Error"); }
    })
}
</script>