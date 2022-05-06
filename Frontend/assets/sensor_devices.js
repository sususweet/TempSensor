function getQueryString(name) {
	var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i");
	var r = window.location.search.substr(1).match(reg);
	if (r != null) return unescape(r[2]); return null;
}

function addDevice(){
    $("#loading").show();
	$.ajax({
		url: "../api/device/addDevice",
		type : 'GET',
		dataType:'json',
		success:function(targetdata){
			alert(targetdata.msg);
			if (targetdata.status==0){
                updateTable();
				//window.location.reload();
			}
            $("#loading").hide();
		}
	});
}

function delDevice(r){
	var item = $(r).parent().parent();
	var dev_id = $(item[0]).children().eq(1).get(0).innerText;
	console.log(dev_id);
    $("#loading").show();
	$.ajax({
		url: "../api/device/delDevice",
		type : 'POST',
		dataType:'json',
		data:{
			dev_id: dev_id
		},
		cache:false,
		success:function(targetdata){
			if (targetdata.status==0){
				alert(targetdata.msg);
				$(item).remove();
				//window.location.reload();
			}else if (targetdata.status==-1){
				alert(targetdata.msg);
			}else {
				alert("未知错误");
			}
            $("#loading").hide();
		}
	});
}

function showDevice(r){
	var item = $(r).parent().parent();
	var dev_id = $(item[0]).children().eq(1).get(0).innerText;
	window.location.href += "/detail?dev_id=" + dev_id;
}


function showDeviceChart(r){
    var item = $(r).parent().parent();
    var dev_id = $(item[0]).children().eq(0).get(0).innerText;
    window.location.href += "?dev_id=" + dev_id;
}
