-- phpMyAdmin SQL Dump
-- version phpStudy 2014
-- http://www.phpmyadmin.net
--
-- 主机: localhost
-- 生成日期: 2017 �?09 �?05 �?08:33
-- 服务器版本: 5.5.53
-- PHP 版本: 5.6.31

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- 数据库: `air_sensor`
--

-- --------------------------------------------------------

--
-- 表的结构 `devices`
--

CREATE TABLE IF NOT EXISTS `devices` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `dev_id` text NOT NULL,
  `dev_secret` text NOT NULL,
  `dev_status` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=17 ;

--
-- 转存表中的数据 `devices`
--

INSERT INTO `devices` (`id`, `dev_id`, `dev_secret`, `dev_status`) VALUES
(9, 'device_f4557af1e63c355', '7f5d6ff0fdf2d40e952a', 0);

-- --------------------------------------------------------

--
-- 表的结构 `device_f4557af1e63c355`
--

CREATE TABLE IF NOT EXISTS `device_f4557af1e63c355` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `temperature` float DEFAULT NULL,
  `humidity` float DEFAULT NULL,
  `PM25` float DEFAULT NULL,
  `longitude` double DEFAULT NULL,
  `latitude` double DEFAULT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=20 ;

--
-- 转存表中的数据 `device_f4557af1e63c355`
--

INSERT INTO `device_f4557af1e63c355` (`id`, `temperature`, `humidity`, `PM25`, `longitude`, `latitude`, `timestamp`) VALUES
(1, 22.5, 80.4, 12.2, 120, 32, '2017-09-03 09:42:14'),
(2, 22.5, 82.5, 56.6, 22.5, 123.5, '2017-09-03 10:15:32'),
(3, 22.5, 82.5, 56.6, 22.5, NULL, '2017-09-03 10:15:54'),
(4, 22.5, 82.5, 56.6, 22.5, NULL, '2017-09-03 10:16:19'),
(5, 22.5, 82.5, 56.6, 22.5, NULL, '2017-09-03 10:16:30'),
(6, 12.5, 82.5, 56.6, 22.5, NULL, '2017-09-03 10:16:38'),
(7, 12.5, 0, 56.6, 22.5, NULL, '2017-09-03 10:17:02'),
(18, 22.5, 12.5, 11.5, 120.5, 32.5, '2017-09-05 06:02:53'),
(17, 22.5, 12.5, 11.5, 222.5, 112.5, '2017-09-05 06:02:27'),
(16, 22.5, 12.5, 11.5, 222.5, 112.5, '2017-09-05 06:01:41'),
(15, 12.3, 32.1, 23.1, NULL, NULL, '2017-09-05 05:44:45'),
(12, 22.5, 12.5, 11.5, 222.5, 112.5, '2017-09-03 11:35:44'),
(13, 22.5, 12.7, 11.5, 120.9, 30, '2017-09-03 14:27:28'),
(19, 22.5, 12.5, 11.5, 120.5, 32.5, '2017-09-05 06:03:26');

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
