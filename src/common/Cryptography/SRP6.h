#ifndef MPOOL_SRP6_H
#define MPOOL_SRP6_H

#include "MpoolDefines.h"
#include "BigNumber.h"
#include "CryptoHash.h"
#include <optional>
#include <iostream>
#include <iomanip>

namespace mpool::Crypto
{
    class POOL_COMMON_API SRP6
    {
    public:
        static constexpr size_t SALT_LENGTH = 32;
        using Salt = std::array<uint8, SALT_LENGTH>;

        static constexpr size_t VERIFIER_LENGTH = 32;
        using Verifier = std::array<uint8, VERIFIER_LENGTH>;

        static constexpr size_t EPHEMERAL_KEY_LENGTH = 32;
        using EphemeralKey = std::array<uint8, EPHEMERAL_KEY_LENGTH>;

        static std::array<uint8, 1> const g;
        static std::array<uint8, 32> const N;

        static std::pair<Salt, Verifier> MakeRegistrationData(std::string const& username, std::string const& password);

        static bool CheckLogin(std::string const& username, std::string const& password, Salt const& salt, Verifier const& verifier)
        {
            return (verifier == CalculateVerifier(username, password, salt));
        }

        static SHA1::Digest GetSessionVerifier(EphemeralKey const& A, SHA1::Digest const& clientM, SessionKey const& K)
        {
            return SHA1::GetDigestOf(A, clientM, K);
        }

        void printHex(const unsigned char* data, size_t size)
        {
            for (size_t i = 0; i < size; ++i)
            {
                // 使用std::hex和std::setw来格式化输出
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
            }

            std::cout << std::endl;
        }

        SRP6(std::string const& username, Salt const& salt, Verifier const& verifier);
        std::optional<SessionKey> VerifyChallengeResponse(EphemeralKey const& A, SHA1::Digest const& clientM);

    private:
        bool _used = false;

        static Verifier CalculateVerifier(std::string const& username, std::string const& password, Salt const& salt);
        static SessionKey SHA1Interleave(EphemeralKey const& S);

        static BigNumber const _g;
        static BigNumber const _N;

        static EphemeralKey _B(BigNumber const& b, BigNumber const& v) { return ((_g.ModExp(b, _N) + (v * 3)) % N).ToByteArray<EPHEMERAL_KEY_LENGTH>(); }
        SHA1::Digest const _I;
        BigNumber const _b;
        BigNumber const _v;
    public:
        Salt const s;
        EphemeralKey const B;

    };
}

#endif //MPOOL_SRP6_H
