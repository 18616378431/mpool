#include "Util.h"
#include "Errors.h"

bool StringEqualI(std::string_view a, std::string_view b)
{
	return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) {
		return std::tolower(c1) == std::tolower(c2);
		});
}

void mpool::Impl::HexStrToByteArray(std::string_view str, uint8* out, size_t outlen, bool reverse)
{
    ASSERT(str.size() == (2 * outlen));

    int32 init = 0;
    int32 end = int32(str.length());
    int8 op = 1;

    if (reverse)
    {
        init = int32(str.length() - 2);
        end = -2;
        op = -1;
    }

    uint32 j = 0;
    for (int32 i = init; i != end; i+= 2 * op)
    {
        char buffer[3] = {str[i], str[i + 1], '\0'};
        out[j++] = uint8(strtoul(buffer, nullptr, 16));
    }
}