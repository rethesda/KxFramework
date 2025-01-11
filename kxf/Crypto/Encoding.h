#pragma once
#include "Common.h"

namespace kxf::Crypto
{
	KXF_API_CRYPTO size_t Rot13(String& data) noexcept;
	KXF_API_CRYPTO size_t UwUize(String& data) noexcept;

	KXF_API_CRYPTO bool Base64Encode(IInputStream& inputStream, IOutputStream& outputStream);
	KXF_API_CRYPTO bool Base64Decode(IInputStream& inputStream, IOutputStream& outputStream);
};
