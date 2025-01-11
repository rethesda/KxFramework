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
