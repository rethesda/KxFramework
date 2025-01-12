#pragma once
#include "Common.h"
#include "kxf/Core/String.h"

namespace kxf::Crypto::Private
{
	template<size_t bitLength, class T = void>
	constexpr bool IsHashConvertibleToInteger() noexcept
	{
		return (bitLength % 8 == 0 || bitLength <= 64) && (std::is_void_v<T> || (std::is_unsigned_v<T> && sizeof(T) >= bitLength / 8));
	}

	KXF_API String HashValueToString(std::span<const std::byte> data);
}

namespace kxf::Crypto
{
	template<size_t bitLength>
	class HashValue final
	{
		static_assert(bitLength >= 8 && bitLength % 8 == 0, "hash value length should be >= 8 bits and evenly divisible by 8");

		public:
			static constexpr size_t BitLength() noexcept
			{
				return bitLength;
			}

		private:
			std::array<std::byte, bitLength / 8> m_Hash;

		public:
			constexpr HashValue() noexcept
			{
				m_Hash.fill(std::byte{0});
			}
			constexpr HashValue(const void* data, size_t length) noexcept
			{
				if (length == m_Hash.size())
				{
					std::memcpy(m_Hash.data(), data, length);
				}
				else
				{
					m_Hash.fill(std::byte{0});
				}
			}

			template<class T>
			requires(Private::IsHashConvertibleToInteger<bitLength, T>())
			constexpr HashValue(T value) noexcept
				:HashValue(&value, sizeof(value))
			{
			}

		public:
			constexpr bool IsNull() const noexcept
			{
				for (std::byte c: m_Hash)
				{
					if (c != std::byte{0})
					{
						return false;
					}
				}
				return true;
			}
			constexpr bool IsConvertibleToInteger() const noexcept
			{
				return Private::IsHashConvertibleToInteger<bitLength>();
			}
			
			constexpr std::span<std::byte> as_span() noexcept
			{
				return m_Hash;
			}
			constexpr std::span<const std::byte> as_span() const noexcept
			{
				return m_Hash;
			}

			constexpr std::byte* data() noexcept
			{
				return m_Hash.data();
			}
			constexpr const std::byte* data() const noexcept
			{
				return m_Hash.data();
			}
			constexpr size_t length() const noexcept
			{
				return m_Hash.size();
			}

			String ToString() const
			{
				return Private::HashValueToString({m_Hash.data(), m_Hash.size()});
			}

			template<class = void>
			requires(Private::IsHashConvertibleToInteger<bitLength>())
			auto ToInt() const noexcept
			{
				auto Convert = [](auto& value)
				{
					std::memcpy(&value, m_Hash.data(), sizeof(value));
					return value;
				};

				if constexpr(bitLength == 8)
				{
					uint8_t value = 0;
					return Convert(value);
				}
				else if constexpr(bitLength == 16)
				{
					uint16_t value = 0;
					return Convert(value);
				}
				else if constexpr(bitLength == 32)
				{
					uint32_t value = 0;
					return Convert(value);
				}
				else if constexpr(bitLength == 64)
				{
					uint64_t value = 0;
					return Convert(value);
				}
			}

			template<class T>
			requires(Private::IsHashConvertibleToInteger<bitLength, T>())
			void FromInt(T value) noexcept
			{
				if (sizeof(value) != m_Hash.size())
				{
					m_Hash.fill(std::byte{0});
				}
				std::memcpy(m_Hash.data(), &value, sizeof(value));
			}

			constexpr auto begin() noexcept
			{
				return m_Hash.begin();
			}
			constexpr auto begin() const noexcept
			{
				return m_Hash.begin();
			}

			constexpr auto end() noexcept
			{
				return m_Hash.end();
			}
			constexpr auto end() const noexcept
			{
				return m_Hash.end();
			}

		public:
			explicit constexpr operator bool() const noexcept
			{
				return !IsNull();
			}
			constexpr bool operator!() const noexcept
			{
				return IsNull();
			}

			constexpr bool operator==(const HashValue& other) const noexcept
			{
				return m_Hash == other.m_Hash;
			}
	};
}
