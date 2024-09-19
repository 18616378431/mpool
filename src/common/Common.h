#ifndef COMMON_H_
#define COMMON_H_

#include "Define.h"

#include <array>
#include <memory>
#include <string>
#include <utility>

#if POOL_PLATFORM == POOL_PLATFORM_WINDOWS
#include <WS2tcpip.h>
#if POOL_COMPILER == POOL_COMPILER_INTEL
#	if !defined(BOOST_ASIO_HAS_MOVE)
#		define BOOST_ASIO_HAS_MOVE
#	endif
#  endif
#else
#include <cstdlib>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if POOL_COMPILER == POOL_COMPILER_MICROSOFT
#define atoll _atoi64
#define llabs _abs64
#else
#define stricmp strcasecmp
#endif

#define STRINGIZE(a) #a

#define MAX_NETCLIENT_PACKET_SIZE (32767 - 1)

constexpr auto MINUTE = 60;
constexpr auto HOUR = MINUTE * 60;
constexpr auto DAY = HOUR * 24;
constexpr auto WEEK = DAY * 7;
constexpr auto MONTH = DAY * 30;
constexpr auto YEAR = MONTH * 12;
constexpr auto IN_MILLISECONDS = 1000;

enum AccountTypes
{
	SEC_PLAYER = 0,
	SEC_MODERATOR = 1,
	SEC_GAMEMASTER = 2,
	SEC_ADMINISTRATOR = 3,
	SEC_CONSOLE = 4
};

enum LocaleConstant
{
	LOCALE_enUS = 0,
	LOCALE_koKR = 1,
	LOCALE_frFR = 2,
	LOCALE_deDE = 3,
	LOCALE_zhCN = 4,
	LOCALE_zhTW = 5,
	LOCALE_esES = 6,
	LOCALE_esMX = 7,
	LOCALE_ruRU = 8,
	TOTAL_LOCALES
};

#define DEFAULT_LOCALE LOCALE_enUS

#define MAX_LOCALES 8
#define MAX_ACCOUNT_TUTORIAL_VALUES 8

POOL_COMMON_API extern char const* localeNames[TOTAL_LOCALES];

POOL_COMMON_API LocaleConstant GetLocaleByName(const std::string& name);
POOL_COMMON_API void CleanStringForMysqlQuery(std::string& str);

#define MAX_QUERY_LEN 32 * 1024

namespace mpool
{
	template<typename ArgumentType, typename ResultType>
	struct unary_function
	{
		typedef ArgumentType argument_type;
		typedef ResultType result_type;
	};
}

#endif