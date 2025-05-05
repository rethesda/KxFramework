#include "kxf-pch.h"
#include "Hash.h"
#include "kxf-crypto/Crypto/HashCalculator_OpenSSL.h"
#include "kxf-crypto/Crypto/HashCalculator_xxHash.h"
#include "kxf-crypto/Crypto/HashCalculator_CRC32.h"

namespace kxf::Crypto
{
	HashValue<32> CRC32(IInputStream& stream, uint32_t initialValue) noexcept
	{
		return HashCalculator_CRC32().Compute(stream, initialValue);
	}
	HashValue<128> MD5(IInputStream& stream) noexcept
	{
		return HashCalculator_MD5().Compute(stream);
	}
	HashValue<160> SHA1(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA1().Compute(stream);
	}

	HashValue<224> SHA2_224(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA2_224().Compute(stream);
	}
	HashValue<256> SHA2_256(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA2_256().Compute(stream);
	}
	HashValue<384> SHA2_384(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA2_384().Compute(stream);
	}
	HashValue<512> SHA2_512(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA2_512().Compute(stream);
	}

	HashValue<224> SHA3_224(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA3_224().Compute(stream);
	}
	HashValue<256> SHA3_256(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA3_256().Compute(stream);
	}
	HashValue<384> SHA3_384(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA3_384().Compute(stream);
	}
	HashValue<512> SHA3_512(IInputStream& stream) noexcept
	{
		return HashCalculator_SHA3_512().Compute(stream);
	}
}

namespace kxf::Crypto
{
	HashValue<32> xxHash_32(std::span<const std::byte> buffer, uint32_t seed) noexcept
	{
		return XXH32(buffer.data(), buffer.size_bytes(), seed);
	}
	HashValue<64> xxHash_64(std::span<const std::byte> buffer, uint64_t seed, const SecretValue& secret) noexcept
	{
		if (seed != 0 && secret)
		{
			return XXH3_64bits_withSecretandSeed(buffer.data(), buffer.size_bytes(), secret.GetData(), secret.GetSize(), seed);
		}
		else if (secret)
		{
			return XXH3_64bits_withSecret(buffer.data(), buffer.size_bytes(), secret.GetData(), secret.GetSize());
		}
		else if (seed != 0)
		{
			return XXH3_64bits_withSeed(buffer.data(), buffer.size_bytes(), seed);
		}
		else
		{
			return XXH3_64bits(buffer.data(), buffer.size_bytes());
		}
	}
	HashValue<128> xxHash_128(std::span<const std::byte> buffer, uint64_t seed, const SecretValue& secret) noexcept
	{
		XXH128_hash_t result = {};
		if (seed != 0 && secret)
		{
			result = XXH3_128bits_withSecretandSeed(buffer.data(), buffer.size_bytes(), secret.GetData(), secret.GetSize(), seed);
		}
		else if (secret)
		{
			result = XXH3_128bits_withSecret(buffer.data(), buffer.size_bytes(), secret.GetData(), secret.GetSize());
		}
		else if (seed != 0)
		{
			result = XXH3_128bits_withSeed(buffer.data(), buffer.size_bytes(), seed);
		}
		else
		{
			result = XXH3_128bits(buffer.data(), buffer.size_bytes());
		}
		return HashValue<128>(&result, sizeof(result));
	}

	HashValue<32> xxHash_32(IInputStream& stream, uint32_t seed) noexcept
	{
		return HashCalculator_xxHash1_32bit().Compute(stream, seed);
	}
	HashValue<64> xxHash_64(IInputStream& stream, uint64_t seed, const SecretValue& secret) noexcept
	{
		return HashCalculator_xxHash3_64bit().Compute(stream, seed, secret);
	}
	HashValue<128> xxHash_128(IInputStream& stream, uint64_t seed, const SecretValue& secret) noexcept
	{
		return HashCalculator_xxHash3_128bit().Compute(stream, seed, secret);
	}
}
