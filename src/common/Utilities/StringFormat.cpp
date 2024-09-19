#include "StringFormat.h"
#include "Define.h"
#include <locale>

template<typename Str>
POOL_COMMON_API Str mpool::String::Trim(const Str& s, const std::locale& loc)
{
	typename Str::const_iterator first = s.begin();
	typename Str::const_iterator end = s.end();

	while (first != end && std::isspace(*first, loc))
	{
		++first;
	}

	if (first == end) 
	{
		return Str();
	}

	typename Str::const_iterator last = end;

	do 
	{
		--last;
	} while (std::isspace(*last, loc));

	if (first != s.begin() || last + 1 != end)
	{
		return Str(first, last + 1);
	}

	return s;
}

std::string mpool::String::TrimRightInPlace(std::string& str)
{
	int pos = int(str.size()) - 1;

	while (pos >= 0 && std::isspace(str[pos]))
	{
		--pos;
	}

	str.resize(static_cast<std::basic_string<char, std::char_traits<char>, std::allocator<char>>::size_type>(pos) + 1);

	return str;
}

template POOL_COMMON_API std::string mpool::String::Trim<std::string>(const std::string& s, const std::locale& loc);
