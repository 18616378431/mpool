#ifndef MPOOL_CRYPTOCONSTANTS_H
#define MPOOL_CRYPTOCONSTANTS_H

#include "Define.h"

namespace mpool::Crypto
{
    struct Constants
    {
        static constexpr size_t MD5_DIGEST_LENGTH_BYTES = 16;
        static constexpr size_t SHA1_DIGEST_LENGTH_BYTES = 20;
        static constexpr size_t SHA256_DIGEST_LENGTH_BYTES = 32;
    };
}

#endif //MPOOL_CRYPTOCONSTANTS_H
