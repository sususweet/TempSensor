<?php
date_default_timezone_set('Asia/Shanghai');
function get_between($start,$end,$content) {
	$r = explode($start, $content);
	if (isset($r[1])){
		$r = explode($end, $r[1]);
		return $r[0];
	}
	return FALSE;
}


function echo_jsonp ($str){
	header("Content-Type: text/javascript; charset=utf-8");
	$callback = isset($_GET["callback"]) ? $_GET["callback"] : FALSE;
	if($callback) {
		echo $callback;
		echo '('.$str.')';
	} else {
		echo $str;
	}
	exit;
}

function my_show_error($msg){
	$json = json_encode(
		array(
			'code' => 0,
			'msg' => $msg
		));
	echo_jsonp($json);
}

function my_show_msg($msg){
	$json = json_encode(
		array(
			'code' => 1,
			'msg' => $msg
		));
	echo_jsonp($json);
}
