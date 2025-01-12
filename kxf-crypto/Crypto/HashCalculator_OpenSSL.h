#pragma once
#include "Common.h"
#include "kxf/Crypto/IHashCalculator.h"

#include <OpenSSL/opensslv.h>
#include <OpenSSL/md5.h>
#include <OpenSSL/sha.h>
#include <OpenSSL/evp.h>

namespace kxf::Crypto
{
	template<auto algorithm, size_t byteLength>
	class HashCalculator_OpenSSL final: public IHashCalculator
	{
		private:
			const EVP_MD* m_Algorithm = nullptr;
			EVP_MD_CTX* m_Context = nullptr;

		public:
			HashCalculator_OpenSSL() noexcept
				:m_Algorithm(std::invoke(algorithm))
			{
			}
			~HashCalculator_OpenSSL() noexcept
			{
				if (m_Context)
				{
					EVP_MD_CTX_destroy(m_Context);
				}
			}

		public:
			// IHashCalculator
			bool Initialize(uint64_t seed, const SecretValue& secret) noexcept override
			{
				if (!m_Algorithm)
				{
					return false;
				}

				if (m_Context = EVP_MD_CTX_create())
				{
					return EVP_DigestInit_ex(m_Context, m_Algorithm, nullptr) == 1;
				}
				return false;
			}
			bool Update(std::span<const std::byte> input) noexcept override
			{
				return EVP_DigestUpdate(m_Context, input.data(), input.size_bytes()) == 1;
			}
			bool Finalize(std::span<std::byte> hash) noexcept override
			{
				unsigned int length = EVP_MD_size(m_Algorithm);
				if (length <= hash.size_bytes())
				{
					return EVP_DigestFinal_ex(m_Context, reinterpret_cast<unsigned char*>(hash.data()), &length) == 1;
				}
				return false;
			}

			// HashCalculator_OpenSSL
			auto Compute(IInputStream& stream)
			{
				return IHashCalculator::ComputeDefault<byteLength * 8>(*this, stream, 0, {});
			}
	};

	using HashCalculator_MD5 = HashCalculator_OpenSSL<EVP_md5, MD5_DIGEST_LENGTH>;
	using HashCalculator_SHA1 = HashCalculator_OpenSSL<EVP_sha1, SHA_DIGEST_LENGTH>;
	using HashCalculator_SHA2_224 = HashCalculator_OpenSSL<EVP_sha224, SHA224_DIGEST_LENGTH>;
	using HashCalculator_SHA2_256 = HashCalculator_OpenSSL<EVP_sha256, SHA256_DIGEST_LENGTH>;
	using HashCalculator_SHA2_384 = HashCalculator_OpenSSL<EVP_sha384, SHA384_DIGEST_LENGTH>;
	using HashCalculator_SHA2_512 = HashCalculator_OpenSSL<EVP_sha512, SHA512_DIGEST_LENGTH>;
	using HashCalculator_SHA3_224 = HashCalculator_OpenSSL<EVP_sha3_224, SHA224_DIGEST_LENGTH>;
	using HashCalculator_SHA3_256 = HashCalculator_OpenSSL<EVP_sha3_256, SHA256_DIGEST_LENGTH>;
	using HashCalculator_SHA3_384 = HashCalculator_OpenSSL<EVP_sha3_384, SHA384_DIGEST_LENGTH>;
	using HashCalculator_SHA3_512 = HashCalculator_OpenSSL<EVP_sha3_512, SHA512_DIGEST_LENGTH>;
}
