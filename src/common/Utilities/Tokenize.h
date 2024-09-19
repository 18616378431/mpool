#ifndef TOKENIZE_H_
#define TOKENIZE_H_

#include <string_view>
#include <vector>

namespace mpool
{
	std::vector<std::string_view> Tokenize(std::string_view str, char sep, bool keepEmpty);

	//deprecated
	std::vector<std::string_view> Tokenize(std::string&&, char, bool) = delete;
	std::vector<std::string_view> Tokenize(std::string const&&, char, bool) = delete;

	inline std::vector<std::string_view> Tokenize(char const* str, char sep, bool keepEmpty)
	{
		return Tokenize(std::string_view(str ? str : ""), sep, keepEmpty);
	}
}

#endif