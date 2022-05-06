<?php
//设置时区 重要！
date_default_timezone_set('Asia/Shanghai');

function is_stuid($str) {
    if(is_numeric($str) && strlen($str) === 10)
        return TRUE;

    return FALSE;   
}
function my_show_error($info = FALSE) {
    header('Location: '.site_url('error/show/'.rawurlencode(str_replace('=', '-', base64_encode($info)))));
    exit;
}
function formspecialchars($var) {
    if (is_array($var)) {
        $out = array();    
        foreach ($var as $key => $v) {     
            $out[$key] = formspecialchars($v);
        }
    } else {
        $out = $var;
        $older = array('&quot;', '&#039;', '&lt', '&gt', '&amp;');
        $replace = array('"', "'", '<', '>', '&');
        $out = str_replace($older, $replace, $out);
        $out = htmlspecialchars(stripslashes(trim($out)), ENT_QUOTES,'UTF-8',true);
    }
    return $out;
 }
//自动验证码辅助工具
function crack($gif_index) {
    $im = imagecreatefromgif($gif_index);
    $n[8]='001110011100011111111110110001100011110001100011110001100011110001100011011111111110001110011100';
    $n[3]='001000001100011000001110110000000111110011000011110011000011110011100111111111111110011100111100';
    $n[6]='000111111000011111111110111001100111110011000011110011000011110011100011111001111110011000111100';
    $n[4]='000000011100000000111100000011101100000111001100011100001100111111111111111111111111000000001100';
    $n[2]='001100000011011100000111111000001111110000011011110000111011110001110011011111100011001110000011';
    $n[1]='000000000000000110000000001100000000011000000000111111111111111111111111000000000000000000000000';
    $n[7]='110000000000110000000000110000001111110001111111110111111000111110000000111000000000110000000000';
    $n[5]='000111101100111111101110111111000111110011000011110011000011110011100111110001111110000000111100';
    $n[0]='001111111100011111111110111000000111110000000011110000000011111000000111011111111110001111111100';

    $num = null;
    for($k = 0; $k <= 4; $k++){

        $s=null;$r=0; // s 为所有位二进制点  r 为每位

        //生成 s
        for($i = 5+$k*9;$i <= 12+$k*9; $i++)
            for($j = 5;$j <= 16; $j++)   
                {
                    $rgb = imagecolorat($im,$i,$j);
                    if($rgb<150)$r=1;
                    else $r=0;
                    $s.="$r";   // s 为 r 的累加
                }

        //判断 s 与哪个数字最接近, 利用循环寻找最匹配的数字
        $choice = -1; 
        $match  = -1;
        for($i=0;$i<9;$i++)
            {
                $new_match = 0;
                for($j=0;$j<96;$j++)
                    {
                        if($s[$j]==$n[$i][$j])
                            $new_match ++;
                    }
                if($new_match > $match)
                    {
                        $choice = $i;
                        $match = $new_match;
                    }
            }

        switch($choice){
        case 8 : $num.='8';break;
        case 3 : $num.='3';break;
        case 6 : $num.='6';break;
        case 1 : $num.='1';break;
        case 2 : $num.='2';break;
        case 5 : $num.='5';break;
        case 7 : $num.='7';break;
        case 4 : $num.='4';break;
        case 0 : $num.='0';break;

        }
    }
    return $num;
}
function get_week($offset = 0) {
    $weekarray = array("周日","周一","周二","周三","周四","周五","周六");
    $offset += date('w');

    if($offset == 7)
        $offset = 0;
  
    return $weekarray[$offset];//Numeric representation of the day of the week
}
function get_date() {
    return date('d');//d Day of the month, 2 digits with leading  01 to 31
}
function get_class_time_detail($time) {
    $weekarray = array("周日","周一","周二","周三","周四","周五","周六");
    //输入:周三第1,2节 周四第3,4节{单周}

    $out = FALSE;
    $class_day = FALSE;

    $class_time_array = explode(' ', $time);

    if(count($class_time_array)){
        foreach ($class_time_array as $class_time) {

            foreach ($weekarray as $day) {
                if(stripos($class_time, $day) !== FALSE) {
                    $class_day = $day;
                }
            }

            $out .= '<br>'.$class_day.'&nbsp;';
            $class_str = get_between('第', '节', $class_time);
            $class_array = explode(',', $class_str);
            foreach ($class_array as $class) {
                if($class) //$class至少不为空。否则军训之类会出错
                    $out .= get_sksj($class).'&nbsp;';
            }
        }
    }

    return $out;
  
}
function get_between($start,$end,$content) {
    $r = explode($start, $content);
    if (isset($r[1])){
        $r = explode($end, $r[1]);
        return $r[0];
    }
    return FALSE;
}

function judge_time_duration($target_time,$start,$end){
	$checkDayStr = date('Y-m-d ', time());
	$timeBegin1 = strtotime($checkDayStr . $start . ":00");
	$timeEnd1 = strtotime($checkDayStr . $end . ":00");
	if ($target_time >= $timeBegin1 && $target_time <= $timeEnd1) {
		return true;
	}else{
		return false;
	}
}

function get_class_from_time(){
	$time_start_array = ['8:00','8:45','9:35','10:35','11:25','12:15','14:00','14:50','15:40','16:40','17:30','19:15','20:05'];
	$time_end_array = ['8:45','9:35','10:35','11:25','12:15','14:00','14:50','15:40','16:40','17:30','19:15','20:05','20:55'];
	$curr_time = time();

	if (judge_time_duration($curr_time, '0:00', '7:30')){
		return -1;
	}else if (judge_time_duration($curr_time, '22:00', '24:00')){
		return -1;
	}else{
		for ($i = 0; $i < sizeof($time_start_array); $i++){
			if (judge_time_duration($curr_time, $time_start_array[$i], $time_end_array[$i])) {
				return $i;
				break;
			}
		}
		return -2;
	}

}


function get_sksj($xzxh) {
    if ($xzxh=="1"){
        $sksj="8:00-8:45";
    }else if ($xzxh=="2"){
        $sksj="8:50-9:35";
    }else if ($xzxh=="3"){
        $sksj="9:50-10:35";
    }else if ($xzxh=="4"){
        $sksj="10:40-11:25";
    }else if ($xzxh=="5"){
        $sksj="11:30-12:15";
    }else if ($xzxh=="6"){
        $sksj="13:15-14:00";
    }else if ($xzxh=="7"){
        $sksj="14:05-14:50";
    }else if ($xzxh=="8"){
        $sksj="14:55-15:40";
    }else if ($xzxh=="9"){
        $sksj="15:55-16:40";
    }else if ($xzxh=="10"){
        $sksj="16:45-17:30";
    }else if ($xzxh=="11"){
        $sksj="18:30-19:15";
    }else if ($xzxh=="12"){
        $sksj="19:20-20:05";
    }else if ($xzxh=="13"){
        $sksj="20:10-20:55";
    }

    return $sksj;
}


function array_msort($array, $cols) {

    $colarr = array();
    foreach ($cols as $col => $order) {
        $colarr[$col] = array();
        foreach ($array as $k => $row) { $colarr[$col]['_'.$k] = $row[$col]; }
    }
    $params = array();
    foreach ($cols as $col => $order) {
        $params[] =& $colarr[$col];
        $params = array_merge($params, (array)$order);
    }
    call_user_func_array('array_multisort', $params);
    $ret = array();
    $keys = array();
    $first = true;
    foreach ($colarr as $col => $arr) {
        foreach ($arr as $k => $v) {
            if ($first) { $keys[$k] = substr($k,1); }
            $k = $keys[$k];
            if (!isset($ret[$k])) $ret[$k] = $array[$k];
            $ret[$k][$col] = $array[$k][$col];
        }
        $first = false;
    }
    return $ret;

}

function echo_jsonp ($str) {
    $callback = isset($_GET["callback"]) ? $_GET["callback"] : FALSE;
    if ($callback) {
        header("Content-type: application/javascript; charset=utf-8");
        echo $callback;
        echo '(' . json_encode(json_decode($str)) . ')';
    } else {
        header("Content-type: application/json; charset=utf-8");
        echo json_encode(json_decode($str), JSON_UNESCAPED_UNICODE);
    }
    exit;
}

function ret_json($err, $msg = null, $data = null) {
	header('Content-Type: application/json');
	$retdata = array(
		'status' => $err,
		'msg' => $msg,
		'data' => $data,
	);
	echo json_encode($retdata);
	exit;
}

