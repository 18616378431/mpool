c++实现的MySQL pool SRP6服务端
=

依赖安装

简介:
该c++服务端实现了配置扫描(基于c++无序hash-`unordered_map`)、mysql同步异步连接池并实现了即时预编译sql、boost.asio异步网络功能

`openssl3.0
boost1.74
mysql:8.0`

### mac:
`安装xcode命令行工具
xcode-select --install
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
`

`brew update
brew install openssl@3 cmake boost bash bash-completion mysql-client@8.0
`

### ubuntu22.04:
`apt-get update && apt-get install git cmake make gcc g++ clang libmysqlclient-dev libssl-dev libboost-all-dev`

*arm版本的ubuntu boost会报编译时错误 注释掉这几句*
*vim /usr/include/boost/predef/hardware/simd.h line 123*

```c++
118 // We check if SIMD extension of multiples architectures have been detected,
119 // if yes, then this is an error!
120 //
121 // NOTE: _X86_AMD implies _X86, so there is no need to check for it here!
122 //
123 //#if defined(BOOST_HW_SIMD_ARM_AVAILABLE) && defined(BOOST_HW_SIMD_PPC_AVAILABLE) ||\
124 //    defined(BOOST_HW_SIMD_ARM_AVAILABLE) && defined(BOOST_HW_SIMD_X86_AVAILABLE) ||\
125 //    defined(BOOST_HW_SIMD_PPC_AVAILABLE) && defined(BOOST_HW_SIMD_X86_AVAILABLE)
126 //#   error "Multiple SIMD architectures detected, this cannot happen!"
127 //#endif
```

### win:
`手动安装boost库及mysql客户端(集成环境一般带有mysql开发库)、openssl3.0`

win下设置BOOST_ROOT环境变量

eg:

BOOST_ROOT

D:/local/boost_1_74_0(boost根目录)


*(解决vs2019无法索引头文件问题:解决方案ALL_BUILD上右键,重新扫描解决方案)*

vs提示错误文件起始位置 notepad++将文件修改位utf8-bom

### 编译命令

```cmake
[1]cd mpool
[2]mkdir build
[3]cd build

ubuntu
cmake ../ -DCMAKE_INSTALL_PREFIX=/www/mpool/build/bin

macos
cmake ../ -DCMAKE_INSTALL_PREFIX=/www/mpool/build/bin 
    -DMYSQL_ADD_INCLUDE_PATH=/opt/homebrew/include/mysql 
    -DMYSQL_LIBRARY=/opt/homebrew/lib/libmysqlclient.dylib 
    -DREADLINE_INCLUDE_DIR=/opt/homebrew/opt/readline/include 
    -DREADLINE_LIBRARY=/opt/homebrew/opt/readline/lib/libreadline.dylib 
    -DOPENSSL_INCLUDE_DIR="$OPENSSL_ROOT_DIR/include" 
    -DOPENSSL_SSL_LIBRARIES="$OPENSSL_ROOT_DIR/lib/libssl.dylib" 
    -DOPENSSL_CRYPTO_LIBRARIES="$OPENSSL_ROOT_DIR/lib/libcrypto.dylib" 

### *windows下使用cmakegui工具进行设置*
```

### 数据库表结构及测试数据

[数据库](./db/account.sql)

```
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
```


## 运行日志
![运行日志](./img/serverlog.png)

### 相关项目

项目  |  名称  |  地址
----  ----  ----
mpool | SRP6服务端 | [mpool](https://github.com/18616378431/mpool)

SRP6ClientForQt6 | SRP6客户端 | [SRP6ClientForQt6](https://github.com/18616378431/SRP6ClientForQt6)

SRP6Register | SRP6注册 ｜  [SRP6Register](https://github.com/18616378431/SRP6Register)

