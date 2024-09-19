#ifndef DEFINE_H_
#define DEFINE_H_

//std
#include <cinttypes>
#include <climits>
#include <cstddef>

//custom
#include "CompilerDefs.h"


//大小端检测
#define POOL_LITTLEENDIAN	0
#define POOL_BIGENDIAN		1

#if !defined(POOL_ENDIAN)
#	if defined(BOOST_BIG_ENDIAN)
#	define POOL_ENDIAN POOL_BIGENDIAN
#	else
#	define POOL_ENDIAN POOL_LITTLEENDIAN
#	endif
#endif

//平台检测
#if POOL_PLATFORM == POOL_PLATFORM_WINDOWS
#	define POOL_PATH_MAX MAX_PATH
#	define _USE_MATH_DEFINES
#else//none win
#	define POOL_PATH_MAX PATH_MAX
#endif

//DEBUG
#if !defined(POOL_DEBUG)
#	define POOL_INLINE inline
#else
#	if !defined(POOL_DEBUG)
#	define POOL_DEBUG
#	endif
#	define POOL_INLINE
#endif

#if POOL_COMPILER == POOL_COMPILER_GNU
#	define ATTR_PRINTF(F,V) __attribute__((format(printf, F, V)))
#else
#	define ATTR_PRINTF(F,V)
#endif

//动态连接符号导出
#ifdef POOL_API_USE_DYNAMIC_LINKING
#	if POOL_COMPILER == POOL_COMPILER_MICROSOFT
#	define POOL_API_EXPORT __declspec(dllexport)
#	define POOL_API_IMPORT __declspec(dllimport)
#	elif POOL_COMPILER == POOL_COMPILER_GNU
#	define POOL_API_EXPORT __attribute__((visibility("default")))
#	define POOL_API_IMPORT
#	else
#		error compiler not supported
#	endif
#else
#	define POOL_API_EXPORT
#	define POOL_API_IMPORT
#endif

#ifdef POOL_API_EXPORT_COMMON
#	define POOL_COMMON_API POOL_API_EXPORT
#else
#	define POOL_COMMON_API POOL_API_IMPORT
#endif

#ifdef POOL_API_EXPORT_DATABASE
#	define POOL_DATABASE_API POOL_API_EXPORT
#else
#	define POOL_DATABASE_API POOL_API_IMPORT
#endif

#define UI64LIT(N) UINT64_C(N)
#define SI64LIT(N) INT64_C(N)

#define STRING_VIEW_FMT_ARG(str) static_cast<int>((str).length()), (str).data()

typedef std::int64_t int64;
typedef std::int32_t int32;
typedef std::int16_t int16;
typedef std::int8_t int8;
typedef std::uint64_t uint64;
typedef std::uint32_t uint32;
typedef std::uint16_t uint16;
typedef std::uint8_t uint8;

#endif