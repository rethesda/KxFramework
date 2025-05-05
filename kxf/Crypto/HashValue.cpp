#include "kxf-pch.h"
#include "HashValue.h"

namespace kxf::Crypto::Private
{
	String HashValueToString(std::span<const std::byte> hashData)
	{
		if (!hashData.empty())
		{
			String result;
			result.reserve(hashData.size() * 2);

			for (auto c: hashData)
			{
				result.Format(kxfS("{:02x}"), c);
			}
			return result;
		}
		return {};
	}
}
