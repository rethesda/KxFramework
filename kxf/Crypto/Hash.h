#pragma once
#include "Common.h"
#include "HashValue.h"
#include "SecretValue.h"

namespace kxf::Crypto
{
	KXF_API_CRYPTO HashValue<32> CRC32(IInputStream& stream, uint32_t initialValue = 0xFFFFFFFFu) noexcept;
	KXF_API_CRYPTO HashValue<128> MD5(IInputStream& stream) noexcept;
	KXF_API_CRYPTO HashValue<160> SHA1(IInputStream& stream) noexcept;

	KXF_API_CRYPTO HashValue<224> SHA2_224(IInputStream& stream) noexcept;
	KXF_API_CRYPTO HashValue<256> SHA2_256(IInputStream& stream) noexcept;
	KXF_API_CRYPTO HashValue<384> SHA2_384(IInputStream& stream) noexcept;
	KXF_API_CRYPTO HashValue<512> SHA2_512(IInputStream& stream) noexcept;

	KXF_API_CRYPTO HashValue<224> SHA3_224(IInputStream& stream) noexcept;
	KXF_API_CRYPTO HashValue<256> SHA3_256(IInputStream& stream) noexcept;
	KXF_API_CRYPTO HashValue<384> SHA3_384(IInputStream& stream) noexcept;
	KXF_API_CRYPTO HashValue<512> SHA3_512(IInputStream& stream) noexcept;
}

namespace kxf::Crypto
{
	KXF_API_CRYPTO HashValue<32> xxHash_32(std::span<const std::byte> buffer, uint32_t seed = 0) noexcept;
	KXF_API_CRYPTO HashValue<64> xxHash_64(std::span<const std::byte> buffer, uint64_t seed = 0, const SecretValue& secret = {}) noexcept;
	KXF_API_CRYPTO HashValue<128> xxHash_128(std::span<const std::byte> buffer, uint64_t seed = 0, const SecretValue& secret = {}) noexcept;

	KXF_API_CRYPTO HashValue<32> xxHash_32(IInputStream& stream, uint32_t seed = 0) noexcept;
	KXF_API_CRYPTO HashValue<64> xxHash_64(IInputStream& stream, uint64_t seed = 0, const SecretValue& secret = {}) noexcept;
	KXF_API_CRYPTO HashValue<128> xxHash_128(IInputStream& stream, uint64_t seed = 0, const SecretValue& secret = {}) noexcept;
};
