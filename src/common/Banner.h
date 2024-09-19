#ifndef BANNER_H_
#define BANNER_H_

#include "Define.h"
#include <string_view>

namespace mpool::Banner
{
	POOL_COMMON_API void Show(std::string_view applicationName, void (*log)(std::string_view text), void (*logExtraInfo)());
}

#endif