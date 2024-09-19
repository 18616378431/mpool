#ifndef MPOOL_MPOOLDEFINES_H
#define MPOOL_MPOOLDEFINES_H

#include "Define.h"
#include <array>

constexpr size_t SESSION_KEY_LENGTH = 40;
using SessionKey = std::array<uint8, SESSION_KEY_LENGTH>;

#endif //MPOOL_MPOOLDEFINES_H
