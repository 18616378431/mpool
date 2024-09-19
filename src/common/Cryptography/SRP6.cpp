#include "SRP6.h"
#include "CryptoRandom.h"
#include "Util.h"

#include <functional>
#include <iostream>
#include <string>

using SHA1 = mpool::Crypto::SHA1;
using SRP6 = mpool::Crypto::SRP6;

std::array<uint8, 1> const SRP6::g = {7};
std::array<uint8, 32> const SRP6::N = HexStrToByteArray<32>("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7", true);
BigNumber const SRP6::_g(SRP6::g);
BigNumber const SRP6::_N(N);

std::pair<SRP6::Salt, SRP6::Verifier> SRP6::MakeRegistrationData(const std::string &username, const std::string &password)
{
    std::pair<SRP6::Salt, SRP6::Verifier> res;
    Crypto::GetRandomBytes(res.first);
    res.second = CalculateVerifier(username, password, res.first);
    return res;
}

SRP6::Verifier SRP6::CalculateVerifier(const std::string &username, const std::string &password, const Salt &salt)
{
    return _g.ModExp(SHA1::GetDigestOf(salt, SHA1::GetDigestOf(username, ":", password)),_N).ToByteArray<32>();
}

SessionKey SRP6::SHA1Interleave(const SRP6::EphemeralKey &S)
{
    std::array<uint8, EPHEMERAL_KEY_LENGTH / 2> buf0{}, buf1{};
    for (size_t i = 0; i < EPHEMERAL_KEY_LENGTH / 2; i++)
    {
        buf0[i] = S[2 * i + 0];
        buf1[i] = S[2 * i + 1];
    }

    size_t p = 0;
    while (p < EPHEMERAL_KEY_LENGTH && !S[p])
        ++p;

    if (p & 1)
        ++p;

    p /= 2;

    SHA1::Digest const hash0 = SHA1::GetDigestOf(buf0.data() + p, EPHEMERAL_KEY_LENGTH / 2 - p);
    SHA1::Digest const hash1 = SHA1::GetDigestOf(buf1.data() + p, EPHEMERAL_KEY_LENGTH / 2 - p);

    SessionKey K;

    for (size_t i = 0; i < SHA1::DIGEST_LENGTH; i++)
    {
        K[2 * i + 0] = hash0[i];
        K[2 * i + 1] = hash1[i];
    }

    return K;
}

SRP6::SRP6(const std::string &username, const Salt &salt, const Verifier &verifier)
    : _I(SHA1::GetDigestOf(username)), _b(Crypto::GetRandomBytes<32>()), _v(verifier), s(salt), B(_B(_b, _v))
{

}

std::optional<SessionKey> SRP6::VerifyChallengeResponse(const EphemeralKey &A, const SHA1::Digest &clientM)
{
    ASSERT(!_used, "used only once");
    _used = true;

    BigNumber const _A(A);

    if ((_A % _N).IsZero())
        return std::nullopt;

    BigNumber const u(SHA1::GetDigestOf(A, B));
    EphemeralKey const S = (_A * (_v.ModExp(u, _N))).ModExp(_b, N).ToByteArray<32>();

    SessionKey K = SHA1Interleave(S);

    SHA1::Digest const NHash = SHA1::GetDigestOf(N);
    SHA1::Digest const gHash = SHA1::GetDigestOf(g);
    SHA1::Digest NgHash;
    std::transform(NHash.begin(), NHash.end(), gHash.begin(), NgHash.begin(), std::bit_xor<>());

    SHA1::Digest const ourM = SHA1::GetDigestOf(NgHash, _I, s, A, B, K);

    if (ourM == clientM)
        return K;

    return std::nullopt;
}