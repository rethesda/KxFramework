#pragma once
#include "Common.h"

namespace kxf
{
	class String;

	enum class IPAddressType: int32_t
	{
		Unknown = -1,
		None = 0,

		IPv4 = 4,
		IPv6 = 6
	};
}

namespace kxf
{
	class KXF_API_NETWORK IPAddress final
	{
		public:
			static IPAddress FromIPv4(uint32_t IPv4_32bit, NetworkByteOrder byteOrder = NetworkByteOrder::Network) noexcept;
			static IPAddress FromIPv4(uint8_t IPv4_8x0, uint8_t IPv4_8x1, uint8_t IPv4_8x2, uint8_t IPv4_8x3, NetworkByteOrder byteOrder = NetworkByteOrder::Network) noexcept;
			static IPAddress FromIPv4(const String& IPv4);

			static IPAddress FromIPv6(const std::array<uint8_t, 16>& IPv6_128bit) noexcept;
			static IPAddress FromIPv6(const String& IPv6);

		private:
			#pragma pack(push)
			#pragma pack(1)
			union
			{
				// IPv4
				union
				{
					struct
					{
						uint8_t _8x0;
						uint8_t _8x1;
						uint8_t _8x2;
						uint8_t _8x3;
					};
					uint32_t _32;
				} IPv4;

				// IPv6
				struct
				{
					union
					{
						std::array<uint8_t, 16> _8x16;
						std::array<uint16_t, 8> _16x8;
						struct
						{
							uint64_t _64x0;
							uint64_t _64x1;
						};
					};
				} IPv6;

				// IPvUnknown
				std::array<uint8_t, 32 - sizeof(IPAddressType)> IPvUnknown = {0};
			};
			#pragma pack(pop)

			IPAddressType m_Type = IPAddressType::None;

		private:
			bool IsSameAs(const IPAddress& other) const noexcept;
			void SwapByteOrderIPv4() noexcept;
			void SwapByteOrderIPv6() noexcept;

		public:
			IPAddress() noexcept = default;

		public:
			bool IsNull() const noexcept;
			IPAddressType GetType() const noexcept;
			String ToString() const;

			uint32_t GetIPv4_32bit(NetworkByteOrder byteOrder = NetworkByteOrder::Network) const noexcept;

		public:
			bool operator==(const IPAddress& other) const noexcept
			{
				return IsSameAs(other);
			}

			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}
	};
}
