#include "kxf-pch.h"
#include "Crypto.h"

#include <OpenSSL/opensslv.h>

#define XXH_STATIC_LINKING_ONLY
#include <xxhash.h>

#include "kxf/Win32/LinkLibs-Crypto.h"
#include "kxf/Win32/LinkLibs-Network.h"
#include "kxf/Win32/UndefMacros.h"

namespace kxf::Crypto
{
	String GetLibraryName()
	{
		return "OpenSSL, xxHash";
	}
	std::array<Version, 2> GetLibraryVersion()
	{
		return
		{
			Version({OPENSSL_VERSION_MAJOR, OPENSSL_VERSION_MINOR, OPENSSL_VERSION_PATCH}),
			Version({XXH_VERSION_MAJOR, XXH_VERSION_MINOR, XXH_VERSION_RELEASE})
		};
	}
}

namespace kxf::Crypto
{
	SecretValue CreateSecret(uint64_t seed)
	{
		std::array<std::byte, XXH3_SECRET_SIZE_MIN> buffer;
		buffer.fill(std::byte{0});
		XXH3_generateSecret_fromSeed(buffer.data(), seed);

		return SecretValue::FromBytes(buffer);
	}
	SecretValue CreateSecret(std::span<const std::byte> seed, size_t desiredLength)
	{
		if (!seed.empty())
		{
			if (desiredLength == std::numeric_limits<size_t>::max())
			{
				desiredLength = seed.size_bytes();
			}

			if (desiredLength <= XXH3_SECRET_SIZE_MIN)
			{
				std::array<std::byte, XXH3_SECRET_SIZE_MIN> buffer;
				buffer.fill(std::byte{0});

				if (XXH3_generateSecret(buffer.data(), buffer.size(), seed.data(), seed.size_bytes()) == XXH_errorcode::XXH_OK)
				{
					return SecretValue::FromBytes(buffer);
				}
			}
			else
			{
				std::vector<std::byte> buffer(desiredLength, std::byte{0});
				if (XXH3_generateSecret(buffer.data(), buffer.size(), seed.data(), seed.size_bytes()) == XXH_errorcode::XXH_OK)
				{
					return SecretValue::FromVector(std::move(buffer));
				}
			}
		}
		return {};
	}
}
