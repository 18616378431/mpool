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
INSERT INTO `account` (`id`, `username`, `salt`, `verifier`, `session_key`, `last_ip`, `last_login`, `locale`, `os`) VALUES (1, 'WJK', 0xB0B45AC5B151E22FADE8A46FA268BF7B965B2A714BF2686052AB4536785DAE96, 0x37CCF9EEA338BE3924C4F0782F8B74B6F803C3724E2D52A678A4B067F6EA2A6A, 0xA601CEA64EF66740484CBF3AC4A98B2E4EE76DBCD7CAE2A3F8748B2FE19CEF5CD217680F1DFEE129, '127.0.0.1', '2024-09-02 11:46:24', 'zhCN', 'OSX');
INSERT INTO `account` (`id`, `username`, `salt`, `verifier`, `session_key`, `last_ip`, `last_login`, `locale`, `os`) VALUES (2, 'WJK1', 0x1D28AE27F2CF1DB274DF7C8D4858E00030993F8686FA2A9AF3E08F605361A9E5, 0xAA31530BEBCCF032080D998641FD8D25858514D1D1D07147E343BC185803F44A, 0xFF58457523EA57A6FDCE4AF33837546801A7AEBA91189325FC7574EE9F6E0E4E4E3C7F2B89D23E99, '192.168.65.1', '2024-07-14 13:53:35', '4', 'OSX');
INSERT INTO `account` (`id`, `username`, `salt`, `verifier`, `session_key`, `last_ip`, `last_login`, `locale`, `os`) VALUES (3, 'WJK2', 0x68B9E6392E87F1542C0388DA6D0B11E4F20283B06CBF54DFC9E2AAE20B68CB5E, 0xDC703D9397F79E1CDB3B0429FB51A96F8B5C2205705099BBB7AD0F5C09BE0A49, 0xBFD23DE2A6FC2D92CFA12D23D1C67D504C4595779674014A61C51E9A7414FB54C37069E49BB854C3, '192.168.65.1', '2024-07-22 09:03:42', '4', 'Win');
INSERT INTO `account` (`id`, `username`, `salt`, `verifier`, `session_key`, `last_ip`, `last_login`, `locale`, `os`) VALUES (4, 'WJK3', 0xE0044A1540B6CAB6CAB7A211DBF8A5EEB52A9D45919204A1C595600EB06A669D, 0xCF5C9BF640C86EB62FDD1C0795B91D8A752268ED1D9DDF5343811A6BFE83C50D, 0xC009B2FC596CB1AE71C9EE09BFD39D8D998CF4EA7294FE9F78C8182B9F5DDBE25BD9F310A1993030, '192.168.65.1', '2024-07-22 09:15:41', '4', 'Win');
COMMIT;

SET FOREIGN_KEY_CHECKS = 1;
