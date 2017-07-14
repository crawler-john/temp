<form class="form-horizontal">
  <div class="col-sm-4 col-sm-offset-4">
    <div class="input-group input-group-sm">
        <input class="form-control datepicker" type="text" name="date" value="<?php echo date("Y-m-d",time());?>" onClick="WdatePicker({maxDate:'%y-%M-%d', <?php echo $this->lang->line('graph_language')?>, onpicked:select_date})" readonly>
        <span class="input-group-btn">
            <button class="btn btn-default" id="query" type="button"><?php echo $this->lang->line('button_query')?></button>
        </span>
    </div>
  </div>
</form>

<!-- 显示图表 -->
<div class="col-sm-12 mychart">
    <div id="myChart"></div>
</div>

<!-- 总能量 -->
<h4><span class="label label-warning pull-right" id="total"><?php echo $this->lang->line('graph_daily_energy').": ".$today_energy;?> kWh</span></h4>

<?php
    $power_data = "";
    foreach ($power as $key => $value) {
        $power_data = $power_data."[".$value['time'].",".$value['each_system_power']."],";
    }
?>

<script src="<?php echo base_url('resources/js/datepicker/WdatePicker.js');?>"></script>
<script src="<?php echo base_url('resources/js/highcharts.js');?>"></script>
<script src="<?php echo base_url('resources/js/modules/exporting.js');?>"></script>
<script>
//window.scrollTo(0,110);//页面位置调整（显示完整图表）
$(document).ready(function() {
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
            defaultSeriesType: 'spline', //绘制圆滑的曲线
            backgroundColor: 'rgba(255,255,255,0)', //设置透明背景
            //height: 400, //图表高度(默认是400px)
            //zoomType: 'x', //可沿着X轴放大
            spacing: [10, 10, 5, 0], //上右下左[外边框和绘图区之间的距离]
            //margin:[10, 10, 25, 70],//上右下左 
        },
        title: {
            text: '<?php echo $this->lang->line('graph_title_power'); ?>'
        },
        subtitle: { //副标题设置
            y: 200,
            style: {
                fontSize: "16px",
            }
        },
        xAxis: { //X坐标轴设置
            type: 'datetime',
        },
        yAxis: { //Y坐标轴设置
            title: {
            	//enabled: false,
            	text: '<?php echo $this->lang->line('graph_y_label_power'); ?>',
                style: {
                    fontSize: "16px",
                }
            },
            min:0,
        },
        tooltip: { //数据点提示框设置
        	borderWidth: 0,
            crosshairs: true,
            //shared: true,
            snap: 0,
            hideDelay:0, //提示框隐藏延时为0
            valueSuffix: ' W',
        },
        legend: { //图例说明
            enabled: false,
//             borderColor: '#DDDDDD',
//             borderWidth: 1,
//             borderRadius: 6,
//             itemStyle: {
//                 fontWeight: 'slin'
//             }
        },
        credits: { //图表版权信息
            enabled: false
        },
        plotOptions: { //数据点配置
        	series: {
        		//shadow: true, //显示曲线阴影
                marker: {
                    enabled: false,
                    fillColor: '#FFFFFF',
                    radius: 2,
                    lineWidth: 1,
                    lineColor: null                    
                },
                states: {
                	hover: {
                    	halo:{
                    	    size: 0 //不显示数据点边缘阴影
                   	    }
                    }
            	},
            },           
        },        
    });
        
    /* 创建图表 */
    $('#myChart').highcharts({
        subtitle: { //副标题设置
        	text: '<?php echo $subtitle; ?>',
        },
        series:[{
            name: '<?php echo $this->lang->line('graph_value_power'); ?>',    
            data: [<?php echo $power_data; ?>]
        }]
    });
    $("#query").click(function(){
    	show_graph();
    });
});
function select_date()
{
	$(".datepicker").blur();
	show_graph();
}
function show_graph()
{
	$.ajax({
		url : "<?php echo base_url('index.php/realtimedata/old_power_graph');?>",
		type : "post",
        dataType : "json",
        data: "date=" + $(".datepicker").val(),
        success : function(Results){
            //整理数据
        	var data_power = new Array();     	
        	$.each(Results.power,function(key,value){   
        		data_power.push([value.time,value.each_system_power]);
     		});     		
     		//重新绘制图表
            $('#myChart').highcharts({
                subtitle: {
                	text: Results.subtitle
                },
                series:[{
                    name: '<?php echo $this->lang->line('graph_value_power'); ?>',    
                    data: data_power
                }]
            });
            //显示发电量
            $('#total').text("<?php echo $this->lang->line('graph_daily_energy');?>: " + Results.today_energy + " kWh");
        },
        error : function() { alert("Error"); }
    })
}
</script>