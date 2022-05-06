<?php
defined('BASEPATH') OR exit('No direct script access allowed');

//header('Content-type: charset=gb2312');

class Device extends CI_Controller {

	private $campusValue = '';
	private $buildingsValue = '';
	private $classroomValue = '';
	private $ComAddress = 'COM9';

	public function __construct(){
		parent::__construct();
		$this->load->database();
		//$this->load->model('User_model');
		$this->load->model('User_model');
		$this->load->helper('qsc_helper');
		$this->load->helper('url_helper');
		$this->load->helper('url');   //url函数库文件
	}

	public function index(){

	}

	/*public function deviceList(){
		$query = $this->db->query("SELECT * FROM devices ORDER BY id ASC");
		$data['users'] = $query->result_array();
		$this->load->view('devices',$data);
	}*/

	public function getDeviceList(){
		$query = $this->db->query("SELECT * FROM devices ORDER BY id ASC");
		$dataArray = $query->result_array();
		$jsonData = array();
		foreach ($dataArray as $data){
			$jsonData[] = array(
				'id'=> $data['id'],
				'dev_id' => $data['dev_id'],
				'dev_secret' => $data['dev_secret'],
				'dev_status' => intval($data['dev_status']),
			);
		}

		ret_json(0,"操作成功！", $jsonData);
	}
	/*public function detail(){
		$this->load->view('device_detail');
	}*/

	public function updateDeviceDetail(){
		/*$dev_id = $this->input->post('dev_id');
		$dev_secret = $this->input->post('dev_secret');

		$this->db->where('dev_id', $dev_id);
		$this->db->where('dev_secret', $dev_secret);
		$this->db->get('devices');
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<=0){
			ret_json(-1,"Bad dev_secret or dev_id.");
		}*/

		$postdata = file_get_contents('php://input');
		$data = array(
			'id'=> null,
			'content' => $postdata,
		);
		$this->db->insert('devices_log', $data);

		//$data = $this->input->post();
		$postdata = json_decode($postdata);
		if (is_null($postdata) || !isset($postdata)){
			ret_json(-1,"Bad data json.");
		}

		@$dev_id = $postdata->dev_id;
		@$dev_secret = $postdata->dev_secret;

		$this->db->where('dev_id', $dev_id);
		$this->db->where('dev_secret', $dev_secret);
		$this->db->get('devices');
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<=0){
			ret_json(-1,"Bad dev_secret or dev_id.");
		}

		@$temperature=$postdata->data->temperature;
		@$humidity=$postdata->data->humidity;
		@$PM25=$postdata->data->PM25;
		@$longitude=$postdata->data->longitude;
		@$latitude=$postdata->data->latitude;


		if ($longitude == 0) $longitude = null;
		if ($latitude == 0) $latitude = null;

		/*$temperature = $this->input->post('temperature');
		$humidity = $this->input->post('humidity');
		$PM25 = $this->input->post('PM25');
		$longitude = $this->input->post('longitude');
		$latitude = $this->input->post('latitude');*/
		$data = array(
			'id'=> null,
			'temperature' => $temperature,
			'humidity' => $humidity,
			'PM25' => $PM25,
			'longitude' => $longitude,
			'latitude' => $latitude,
		);

		$this->db->insert($dev_id, $data);
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<>0){
			ret_json(0,"操作成功！影响记录数：".$nu."条");
			//echo "<script>alert('操作失败！');</script>";
		}else{
			ret_json(-1,"操作失败！");
		}

	}


	public function getDeviceDetail(){
		$dev_id = $this->input->post('dev_id');
		$this->db->where('dev_id', $dev_id);
		$this->db->get('devices');
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<=0){
			ret_json(-1,"操作失败！");
		}

		$this->db->order_by('timestamp DESC');
		$this->db->limit(1);
		$data = $this->db->get($dev_id);
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu > 0){
			$data = $data->result_array()[0];
			$currentTime = strtotime (date("Y-m-d H:i:s")); //当前时间  ,注意H 是24小时 h是12小时
			$lastTime = strtotime ($data['timestamp']);
			$timeDiff = ceil(($currentTime-$lastTime)); //60s*60min*24h
			if ($timeDiff > 15) $status = 0;
			else $status = 1;

			$jsonData = array(
				'id'=> $data['id'],
				'temperature' => $data['temperature'],
				'humidity' => $data['humidity'],
				'PM25' => $data['PM25'],
				'longitude' => $data['longitude'],
				'latitude' => $data['latitude'],
				'status' => $status,
			);

			ret_json(0,"操作成功！", $jsonData);
			//echo "<script>alert('操作失败！');</script>";
		}else{
			ret_json(-1,"操作失败！");
		}

	}

	public function getDeviceChart(){
		$dev_id = $this->input->get('dev_id');
		$type = $this->input->get('type');


		$this->db->where('dev_id', $dev_id);
		$this->db->get('devices');
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<=0){
			ret_json(-1,"操作失败！");
		}

		switch ($type){
			case '0':
				$this->db->where("DATE_SUB(CURDATE(), INTERVAL 1 HOUR) <= date(timestamp)");
				break;
			case '1':
				$this->db->where("DATE_SUB(CURDATE(), INTERVAL 1 DAY) <= date(timestamp)");
				break;
			case '2':
				$this->db->where("DATE_SUB(CURDATE(), INTERVAL 7 DAY) <= date(timestamp)");
				break;
			case '3':
				$this->db->where("DATE_SUB(CURDATE(), INTERVAL 1 MONTH) <= date(timestamp)");
				break;
			default:
				break;
		}


		$this->db->order_by('timestamp ASC');
		$data = $this->db->get($dev_id);
		$nu = $this->db->affected_rows();  //影响多少条记录

		$dataArray = $data->result_array();

		$xData = array();
		$dataSets = array();

		$temperatureArray = array();
		$humidityArray = array();
		$PM25Array = array();

		foreach ($dataArray as $data){
			$currentTime = strtotime (date("Y-m-d H:i:s")); //当前时间  ,注意H 是24小时 h是12小时
			$timestamp = strtotime ($data['timestamp']);

			$temperatureArray[] = floatval($data['temperature']);
			$humidityArray[] = floatval($data['humidity']);
			$PM25Array[] = floatval($data['PM25']);

			$xData[] = intval($timestamp) * 1000;
			/*$xData[] = "Date.UTC(".date("Y",$timestamp).",".date("m",$timestamp).",".
				date("d",$timestamp).",".date("H",$timestamp).",".
				date("i",$timestamp).",".date("s",$timestamp).")";*/

		}
		$dataSets[] = array(
			'name' => "温度",
			'data' => $temperatureArray,
			'unit'=> "℃",
			'min' => 0,
			'max' => 40,
			'type' => "line",
			'valueDecimals'=> 1,
		);
		$dataSets[] = array(
			'name' => "湿度",
			'data' => $humidityArray,
			'unit'=> "%",
			'min' => 0,
			'max' => 100,
			'type' => "line",
			'valueDecimals'=> 1,
		);
		$dataSets[] = array(
			'name' => "PM2.5浓度",
			'data' => $PM25Array,
			'unit'=> "μg/m3",
			'min' => 0,
			'max' => 400,
			'type' => "line",
			'valueDecimals'=> 1,
		);

		$this->ret_chart_json($xData, $dataSets);

	}

	public function addDevice(){
		$dev_id = "device_" . substr($this->uuid(), 0,15);
		$dev_secret = substr($this->uuid(), 0,20);
		$data = array(
			'id'=> null,
			'dev_id' => $dev_id,
			'dev_secret' => $dev_secret,
		);
		$this->db->insert('devices', $data);
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<>0){
			ret_json(0,"操作成功！影响记录数：".$nu."条");
			//echo "<script>alert('操作失败！');</script>";
		}else{
			ret_json(-1,"操作失败！");
		}
	}

	public function delDevice(){
		$dev_id= $this->input->post('dev_id');
		$this->db->where('dev_id', $dev_id);
		$this->db->delete('devices');
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<>0){
			ret_json(0,"操作成功！影响记录数：".$nu."条");
			//echo "<script>alert('操作失败！');</script>";
		}else{
			ret_json(-1,"操作失败！");
		}
	}


	public function uuid() {
		if (function_exists ( 'com_create_guid' )) {
			return com_create_guid ();
		} else {
			mt_srand ( ( double ) microtime () * 10000 ); //optional for php 4.2.0 and up.随便数播种，4.2.0以后不需要了。
			$charid = strtoupper ( md5 ( uniqid ( rand (), true ) ) ); //根据当前时间（微秒计）生成唯一id.
			$hyphen = chr ( 45 ); // "-"
			$uuid = '' . //chr(123)// "{"
				substr ( $charid, 0, 8 ) . $hyphen . substr ( $charid, 8, 4 ) . $hyphen . substr ( $charid, 12, 4 ) . $hyphen . substr ( $charid, 16, 4 ) . $hyphen . substr ( $charid, 20, 12 );
			//.chr(125);// "}"
			return strtolower($charid);
		}
	}

	public function ret_chart_json($xData = null, $dataSets = null){
		header('Content-Type: application/json');
		$jsonData = array(
			'xData' => $xData,
			'datasets' => $dataSets,
		);
		echo json_encode($jsonData);
		exit;
	}

}
