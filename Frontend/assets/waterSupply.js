/*
 * Created by tangyq on 2017/7/25.
 */
var chart;
var i = 0;
function TranslateData(mdata){

	if (mdata.data.pressure < 20){
		$('#presentPressure').html(mdata.data.pressure + 'kPa');
		//if (i % 2 === 0){
		chart.series[0].addPoint(mdata.data.pressure);
		//}
		//i += 1;
	}

	if (mdata.data.waterFlow < 20) {
		$('#presentFlow').html(mdata.data.waterFlow + 'L/min');
	}

	if (mdata.data.motorState == 1){
		$('#Mottor').html('停止');
	}else if (mdata.data.motorState == 0){
		$('#Mottor').html('开始');
	}
	//console.log(mdata.data.motorState)
}


$(document).ready(function() {

	$('#standbyPressure').slider({
		formatter: function(value) {
			value=value.toFixed(1);
			$('#standbyPressureValue').html(value + 'kPa');
			return 'Current value: ' + value;
		}
	});
	$('#workingPressure').slider({
		formatter: function(value) {
			value=value.toFixed(1);
			$('#workPressureValue').html(value + 'kPa');
			return 'Current value: ' + value;
		}
	});

	$(function () {
		chart = Highcharts.chart('PressureChart', {
			chart:{
				height:280,
				//backgroundColor: 'rgba(0,0,0,0)'
			},
			series: [{
				//data: [1, 2, 3]
			}],
			credits: {
				enabled: false
			},
			legend: {
				enabled: false
			},
			yAxis: {
				max: 30,
				minTickInterval: 0.1,
				title: {
					text: 'Pressure/kPa'
				}
			},
			xAxis:{
				minTickInterval: 1
			},
			title: {
				text: null,
				//text: '系统供水压力实时监测',
				style: {
					//color: '#FF00FF',
					fontWeight: 'bold',
					fontSize: '25px'
				}
			}
		});
		// the button action
		//var i = 0;
		//$('#button').click(function () {
		//  chart.series[0].addPoint(50 * (i % 3));
		//i += 1;
		//});
	});

	var waitForConnection = function (callback, interval) {
		if (ws.readyState === 1) {
			callback();
		} else {
			var that = this;
			// optional: implement backoff for interval here
			setTimeout(function () {
				waitForConnection(callback, interval);
			}, interval);
		}
	};

	var sendWebsocket = function (header, message, callback) {
		waitForConnection(function () {
			ws.send(header, message);
			if (typeof callback !== 'undefined') {
				callback();
			}
		}, 1000);
	};


	const ws = $.websocket("ws://10.189.15.113:8888/",{
		open: function() {
			$("#warning").css('display','none');
		},
		close: function() {
			//主机断开websocket或不可用 console.log(123)
			$("#warning").css('display','block');
		},
		message:function(e) {
			var m = eval("(" + (e.originalEvent.data) + ")");
			console.log(m);
			var data = m.header;
			console.log(data);
			if (m.header === "init_info"){
				TranslateData(m);
			}
		}, events: {
			say: function(e) {
				console.log(e); // 'foo'
				// alert(e.data.text); // 'baa' }
			}
		}
	});
	$('#Mottor').click(function(){
		var standBy = $('#standbyPressure').val();
		var working = $('#workingPressure').val();
		sendWebsocket("init_motor", {standbyPressure: standBy,workingPressure: working});
		sendWebsocket("oper_motor", "start");
		//ws.send("oper_motor", "start");
	});

	//this.send("oper_motor","");



	/*this.send("enter_room",{
	 room_id: 1,
	 p_id: -1
	 });*/
	/*$('#message').change(function(){
	 ws.send('message', this.value);
	 this.value = '';
	 });*/


});
