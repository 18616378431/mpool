/*
 Navicat Premium Data Transfer

 Source Server         : docker_mysql8
 Source Server Type    : MySQL
 Source Server Version : 80029
 Source Host           : 127.0.0.1:3506
 Source Schema         : mpool

 Target Server Type    : MySQL
 Target Server Version : 80029
 File Encoding         : 65001

 Date: 02/09/2024 19:47:27
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `id` int unsigned NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `username` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `salt` binary(32) NOT NULL,
  `verifier` binary(32) NOT NULL,
  `session_key` binary(40) DEFAULT NULL,
  `last_ip` varchar(15) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '127.0.0.1',
  `last_login` timestamp NULL DEFAULT NULL,
  `locale` varchar(30) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '0',
  `os` varchar(3) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_username` (`username`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Account System';

-- ----------------------------
-- Records of account
-- ----------------------------
BEGIN;
INSERT INTO `mpool`.`account` (`id`, `username`, `salt`, `verifier`, `session_key`, `last_ip`, `last_login`, `locale`, `os`) VALUES (5, 'TEST', 0x97305573B0F10A7A50FAE905838DBB392B56783E389ED9FE95CB9AF6091635CE, 0x1F62F808B7A7F3AA5F2198D28A553E85F061E7138DF24120F1DA077B05DF4019, 0xA85B8B226B8D16D50C3D4BA89E2D4B233FEA8F6056C9EAF3924856B94E88FF9FC9E7DEEB179A158E, '127.0.0.1', '2024-09-19 11:27:21', 'zhCN', 'OSX');
COMMIT;

SET FOREIGN_KEY_CHECKS = 1;
