#pragma once
#include "Common.h"
#include "SecretValue.h"
#include "kxf/Core/Version.h"

namespace kxf::Crypto
{
	KXF_API_CRYPTO String GetLibraryName();
	KXF_API_CRYPTO std::array<Version, 2> GetLibraryVersion();
}

namespace kxf::Crypto
{
	KXF_API_CRYPTO SecretValue CreateSecret(uint64_t seed);
	KXF_API_CRYPTO SecretValue CreateSecret(std::span<const std::byte> seed, size_t desiredLength = std::numeric_limits<size_t>::max());
}
