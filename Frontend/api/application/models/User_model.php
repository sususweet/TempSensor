<?php
class User_model extends CI_Model {

	private $ch;

	public function __construct(){
		parent::__construct();
		$this->load->database();
		$this->ch = curl_init();
	}


	public function get_devices(){
		$query = $this->db->query("SELECT * FROM devices ORDER BY id ASC");
		return $query->result_array();
	}











	public function ret_json($err, $msg = null, $data = null) {
		header('Content-Type: application/json');
		$retdata = array(
			'status' => $err,
			'msg' => $msg,
			'data' => $data,
		);
		echo json_encode($retdata);
		exit;
	}







	public function get_userTotal(){
		$query = $this->db->query("SELECT COUNT(*) FROM enrollment_users");
		$count = $query->row_array();
		$userTotal = 0;
		foreach ($count as $ckey=>$cvalue) {
			$userTotal=$cvalue;
		}
		return $userTotal;
	}

	public function get_versionKind(){
		$query = $this->db->query("SELECT version,COUNT(version) FROM enrollment_users group by version");
		$versionKind = $query->result_array();
		$versionCount = [];
		foreach ($versionKind as $versionKindItem){
			$versionCount[$versionKindItem['version']]=$versionKindItem['COUNT(version)'];
		}
		return $versionCount;
	}

	public function get_userKind(){
		$query = $this->db->query("SELECT distinct left(uid,3) FROM enrollment_users");
		$userKind = $query->result_array();
		$userCount = [];
		foreach ($userKind as $userKindItem){
			foreach ($userKindItem as $key=>$value) {
				$query = $this->db->query("SELECT count(*) FROM enrollment_users where left(uid,3)='$value'");
				$count = $query->row_array();
				foreach ($count as $ckey=>$cvalue) {
					$userCount[$value]=$cvalue;
				}
			}
		}
		return $userCount;
	}


	public function get_apply_users(){
		$query = $this->db->query("SELECT * FROM enrollment_users_apply ORDER BY addtime DESC");
		return $query->result_array();
	}

	public function get_show_update(){
		$query = $this->db->query("SELECT * FROM enrollment_update WHERE id = 1");
		return $query->result_array();
	}



	public function admit_apply_users(){
		$uid = $this->input->get('uid');
		$detail = $this->input->get('detail');
		$query = $this->db->query("SELECT * FROM enrollment_users WHERE uid = '".$uid."'");
		$user = $this->db->affected_rows();  //影响多少条记录
		if($user<>0){
			$sql="UPDATE enrollment_users SET valid = '1', detail = '$detail' WHERE uid = '".$uid."'";
		}else{
			$sql="INSERT INTO enrollment_users (uid, valid, detail, active, version) VALUES ('$uid', '1', '$detail', CURRENT_TIMESTAMP, '0');";
		}
		$query = $this->db->query($sql);
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<>0){
			$query = $this->db->query("DELETE FROM enrollment_users_apply WHERE uid = '".$uid."'");
			$nu2 = $this->db->affected_rows();  //影响多少条记录
			$this->ret_json(0,"添加成功！影响记录数：$nu,$nu2 条");
		}else{
			$this->ret_json(-1,"添加失败！");
		}
	}

	public function del_apply_users(){
		$uid = $this->input->get('uid');
		//if ($uid == null) return false;
		$query = $this->db->query("DELETE FROM enrollment_users_apply WHERE uid = '".$uid."'");
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<>0){
			$this->ret_json(0,"拒绝成功！");
		}else{
			$this->ret_json(-1,"拒绝失败！");
		}
	}

	public function control_users(){
		$uid = $this->input->get('uid');
		$valid = $this->input->get('valid');
		$query = $this->db->query("SELECT * FROM enrollment_users WHERE uid = '$uid'");
		$user = $this->db->affected_rows();
		if($user<>0){
			$sql="UPDATE enrollment_users SET valid = '$valid' WHERE uid = '$uid'";
		}else{
			$sql="INSERT INTO enrollment_users (uid, valid, detail, active, version) VALUES ('$uid', '$valid', 'null', CURRENT_TIMESTAMP, '0');";
		}
		$query = $this->db->query($sql);
		$nu = $this->db->affected_rows();
		if($nu<>0){
			$this->ret_json(0,"操作成功，刷新查看结果！影响记录数：".$nu."条");
		}else{
			$this->ret_json(-1,"操作失败！");
		}
	}

	public function del_users(){
		$uid = $this->input->get('uid');
		$query = $this->db->query("DELETE FROM enrollment_users WHERE uid = '$uid'");
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<>0){
			$this->ret_json(0,"操作成功！影响记录数：".$nu."条");
		}else{
			$this->ret_json(-1,"操作失败！");
		}
	}

	public function summit_update(){
		$this->db->set('version', $this->input->post('version'));
		$this->db->set('codebase', $this->input->post('uri'));
		$this->db->where('id', 1);
		$this->db->update('enrollment_update');
		$nu = $this->db->affected_rows();  //影响多少条记录
		if($nu<>0){
			echo "<script>alert('操作成功！');location.href='".$_SERVER["HTTP_REFERER"]."';</script>";
		}else{
			echo "<script>alert('操作失败！');</script>";
		}
	}


}
