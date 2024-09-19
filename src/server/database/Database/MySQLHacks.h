#ifndef MYSQL_HACKS_H_
#define MYSQL_HACKS_H_

#include <type_traits>

#include "MySQLWorkaround.h"

struct MySQLHandle : MYSQL {};
struct MySQLResult : MYSQL_RES {};
struct MySQLField : MYSQL_FIELD {};
struct MySQLBind : MYSQL_BIND {};
struct MySQLStmt : MYSQL_STMT {};

//compatibility mysql8 it use bool directly
using MySQLBool = std::remove_pointer_t<decltype(std::declval<MYSQL_BIND>().is_null)>;

#endif