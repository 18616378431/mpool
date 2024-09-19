#ifndef UTIL_H_
#define UTIL_H_


#include "Define.h"
#include "Optional.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <list>
#include <map>
#include <string>
#include <vector>

POOL_COMMON_API bool StringEqualI(std::string_view str1, std::string_view str2);

namespace mpool::Impl
{
    POOL_COMMON_API void HexStrToByteArray(std::string_view str, uint8* out, size_t outlen, bool reverse = false);
}

template <size_t Size>
void HexStrToByteArray(std::string_view str, std::array<uint8, Size>& buf, bool reverse = false)
{
    mpool::Impl::HexStrToByteArray(str, buf.data(), Size, reverse);
}

template <size_t Size>
std::array<uint8, Size> HexStrToByteArray(std::string_view str, bool reverse = false)
{
    std::array<uint8, Size> arr;
    HexStrToByteArray(str, arr, reverse);

    return arr;
}

#endif // !UTIL_H_
