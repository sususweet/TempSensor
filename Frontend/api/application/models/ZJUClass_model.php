<?php

/**
 * @property mixed|null cli_post
 * @property mixed $CLASSROOM_HOST
 * @property array $CLASSROOM_HOSTS
 * @property string $CLASSROOM_URL
 */
class ZJUClass_model extends CI_Model{
	// curl 连接
	private $ch;
	private $__EVENTTARGET = '';
	private $__EVENTARGUMENT = '';
	private $__VIEWSTATE = '';
	private $__VIEWSTATEGENERATOR = '';
	private $__EVENTVALIDATION = '';

	private $cookie_content, $cookie_arr, $cli_post;

	static function echo_jsonp ($str){
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

	public function __construct(){
		parent::__construct();
		$this->load->library('Simple_html_dom');

		$this->ch = curl_init();
		$this->CLASSROOM_HOSTS = array(
			'jxzygl.zju.edu.cn',
			//'10.202.78.13',
			//'172.20.30.3:8001',    // down now!
			//'172.20.1.61:8001',
			// '172.20.217.1:8001', // 217 network temporarily down
//            '192.168.76.55:8001',
//            '192.168.76.68:8001',
		);

		$this->CLASSROOM_HOST = $this->CLASSROOM_HOSTS[intval(rand(0, count($this->CLASSROOM_HOSTS) - 1))];
		$this->cli_post = isset($_POST['user_state']) ? json_decode($_POST['user_state']) : null; // re-enable
	}

	static function my_show_error($msg){
		$json = json_encode(
			array(
				'code' => 0,
				'msg' => $msg
			));
		self::echo_jsonp($json);
	}

	static function my_show_msg($msg){
		$json = json_encode(
			array(
				'code' => 1,
				'msg' => $msg
			));
		self::echo_jsonp($json);
	}

	public function get_cookie($data) {
		// 获取、更新 Cookie
		$data_lines = explode("\n\n", $data);
		$data_lines = explode("\n", $data_lines[0]);
		$cookies_arr = [];
		foreach (explode(';', $this->cookie_content) as $cookie) {
			$cookie_per = explode('=', $cookie);
			if (trim($cookie_per[0]) == '') continue;
			$cookies_arr[trim($cookie_per[0])] = @trim($cookie_per[1]);
		}
		foreach ($data_lines as $data_line) {
			$data_line = trim($data_line);
			if (preg_match('/Set-Cookie: ([^;]+)/i', $data_line, $matches)) {
				$cookie_per = explode('=', $matches[1]);
				if (trim($cookie_per[0]) == '') continue;
				$cookies_arr[trim($cookie_per[0])] = @trim($cookie_per[1]);
			}
		}
		$cookies = '';
		foreach ($cookies_arr as $key => $value) {
			$cookies .= "$key=$value; ";
		}
		$this->cookie_content = $cookies;
	}


}
