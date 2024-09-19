#include "CryptoRandom.h"
#include "Errors.h"

#include <openssl/rand.h>

void mpool::Crypto::GetRandomBytes(uint8 *buf, size_t len)
{
    int result = RAND_bytes(buf, len);
    ASSERT(result == 1, "GetRandomBytes error!");
}
