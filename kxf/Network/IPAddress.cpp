#include "kxf-pch.h"
#include "IPAddress.h"
#include "kxf/Core/String.h"

#include "kxf/Win32/Include-Network.h"
#include "kxf/Win32/LinkLibs-Network.h"
#include "kxf/Win32/UndefMacros.h"

namespace kxf
{
	IPAddress IPAddress::FromIPv4(uint32_t IPv4_32bit, NetworkByteOrder byteOrder) noexcept
	{
		static_assert(sizeof(IPAddress::IPv4) == 4, "IPAddress::IPv4 size must be 4 bytes");

		IPAddress result;
		result.m_Type = IPAddressType::IPv4;
		result.IPv4._32 = IPv4_32bit;

		if (byteOrder == NetworkByteOrder::Native)
		{
			result.SwapByteOrderIPv4();
		}
		return result;
	}
	IPAddress IPAddress::FromIPv4(uint8_t IPv4_8x0, uint8_t IPv4_8x1, uint8_t IPv4_8x2, uint8_t IPv4_8x3, NetworkByteOrder byteOrder) noexcept
	{
		IPAddress result;
		result.m_Type = IPAddressType::IPv4;
		result.IPv4._8x0 = IPv4_8x0;
		result.IPv4._8x1 = IPv4_8x1;
		result.IPv4._8x2 = IPv4_8x2;
		result.IPv4._8x3 = IPv4_8x3;

		if (byteOrder == NetworkByteOrder::Native)
		{
			result.SwapByteOrderIPv4();
		}
		return result;
	}
	IPAddress IPAddress::FromIPv4(const String& IPv4)
	{
		IPAddress result;
		if (::InetPtonW(AF_INET, IPv4.wc_str(), &result.IPv4) == 1)
		{
			result.m_Type = IPAddressType::IPv4;
			return result;
		}
		return {};
	}

	IPAddress IPAddress::FromIPv6(const std::array<uint8_t, 16>& IPv6_128bit) noexcept
	{
		static_assert(sizeof(IPAddress::IPv6) == 16, "IPAddress::IPv6 size must be 16 bytes");

		IPAddress result;
		result.m_Type = IPAddressType::IPv6;
		std::memcpy(&result.IPv6, IPv6_128bit.data(), IPv6_128bit.size());

		return result;
	}
	IPAddress IPAddress::FromIPv6(const String& IPv6)
	{
		IPAddress result;
		if (::InetPtonW(AF_INET6, IPv6.wc_str(), &result.IPv6) == 1)
		{
			result.m_Type = IPAddressType::IPv6;
			return result;
		}
		return {};
	}

	bool IPAddress::IsSameAs(const IPAddress& other) const noexcept
	{
		if (m_Type == other.m_Type)
		{
			switch (m_Type)
			{
				case IPAddressType::IPv4:
				{
					return IPv4._32 == other.IPv4._32;
				}
				case IPAddressType::IPv6:
				{
					return IPv6._64x0 == other.IPv6._64x0 && IPv6._64x1 == other.IPv6._64x1;
				}
				case IPAddressType::Unknown:
				{
					return IPvUnknown == other.IPvUnknown;
				}
			};
		}
		return false;
	}
	void IPAddress::SwapByteOrderIPv4() noexcept
	{
		std::swap(IPv4._8x0, IPv4._8x3);
		std::swap(IPv4._8x1, IPv4._8x2);
	}
	void IPAddress::SwapByteOrderIPv6() noexcept
	{
		auto& items = IPv6._8x16;
		for (size_t i = 0; i < items.size();)
		{
			std::swap(items[i], items[i + 1]);
			i += 2;
		}
	}

	bool IPAddress::IsNull() const noexcept
	{
		switch (m_Type)
		{
			case IPAddressType::IPv4:
			{
				return IPv4._32 == 0;
			}
			case IPAddressType::IPv6:
			{
				return IPv6._64x0 == 0 && IPv6._64x1;
			}
			case IPAddressType::Unknown:
			{
				for (auto& x: IPvUnknown)
				{
					if (x != 0)
					{
						return false;
					}
				}
				return true;
			}
		};
		return true;
	}
	IPAddressType IPAddress::GetType() const noexcept
	{
		if (m_Type == IPAddressType::None)
		{
			return IPAddressType::Unknown;
		}
		return m_Type;
	}
	String IPAddress::ToString() const
	{
		switch (m_Type)
		{
			case IPAddressType::IPv4:
			{
				return Format("{}.{}.{}.{}", IPv4._8x0, IPv4._8x1, IPv4._8x2, IPv4._8x3);
			}
			case IPAddressType::IPv6:
			{
				// We need to swap byte order to print it properly
				auto temp = *this;
				temp.SwapByteOrderIPv6();

				return Format("{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}:{:04x}",
							  temp.IPv6._16x8[0],
							  temp.IPv6._16x8[1],
							  temp.IPv6._16x8[2],
							  temp.IPv6._16x8[3],
							  temp.IPv6._16x8[4],
							  temp.IPv6._16x8[5],
							  temp.IPv6._16x8[6],
							  temp.IPv6._16x8[7]
				);
			}
			case IPAddressType::Unknown:
			{
				auto str = String::FromUnknownEncoding({std::bit_cast<const char*>(IPvUnknown.data()), IPvUnknown.size()});
				if (str.IsEmpty())
				{
					str.reserve(IPvUnknown.size() * 5);
					for (auto& x: IPvUnknown)
					{
						if (!str.IsEmpty())
						{
							str += ';';
						}
						str.Format("0x{:02x}", x);
					}
				}

				return str;
			}
		};
		return {};
	}

	uint32_t IPAddress::GetIPv4_32bit(NetworkByteOrder byteOrder) const noexcept
	{
		if (m_Type == IPAddressType::IPv4)
		{
			if (byteOrder == NetworkByteOrder::Network)
			{
				return IPv4._32;
			}
			else
			{
				auto temp = *this;
				temp.SwapByteOrderIPv4();
				return temp.IPv4._32;
			}
		}
		return 0;
	}
}
