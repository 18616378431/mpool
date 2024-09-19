#ifndef STRING_FORMAT_H_
#define STRING_FORMAT_H_

#include "Define.h"

#include <fmt/format.h>
#include <fmt/printf.h>

namespace mpool
{
	//default mpool string format function
	template<typename Format, typename... Args>
	inline std::string StringFormat(Format&& fmt, Args&&... args)
	{
		try
		{
			return fmt::sprintf(std::forward<Format>(fmt), std::forward<Args>(args)...);
		}
		catch (const fmt::format_error& formatError)
		{
			std::string error = "An error occurred formatting string \"" + std::string(fmt) + "\" : " + std::string(formatError.what());
			return error;
		}
	}

	//default string format function
	template<typename... Args>
	inline std::string StringFormatFmt(std::string_view fmt, Args&&... args)
	{
		try 
		{
			return fmt::format(fmt, std::forward<Args>(args)...);
		}
		catch (const fmt::format_error& formatError)
		{
			return fmt::format("An error occurred formatting string \"{}\" : {}", fmt, formatError.what());
		}
	}

	inline bool IsFormatEmptyOrNull(char const* fmt)
	{
		return fmt == nullptr;
	}

	inline bool IsFormatEmptyOrNull(std::string_view fmt)
	{
		return fmt.empty();
	}
}

namespace mpool::String
{
	template<typename Str>
	POOL_COMMON_API Str Trim(const Str& s, const std::locale& loc = std::locale());

	POOL_COMMON_API std::string TrimRightInPlace(std::string& str);
}

#endif