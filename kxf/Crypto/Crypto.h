#pragma once
#include "Common.h"
#include "kxf/Core/Version.h"

namespace kxf::Crypto
{
	KXF_API_CRYPTO String GetLibraryName();
	KXF_API_CRYPTO std::array<Version, 2> GetLibraryVersion();
}
