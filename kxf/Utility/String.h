#pragma once
#include "kxf/Common.hpp"
#include "kxf/Core/String.h"
#include "kxf/Core/UniChar.h"
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace kxf
{
	class IEncodingConverter;
}

namespace kxf::Utility
{
	struct StringCompareIC
	{
		template<class T>
		std::strong_ordering Compare(const T& left, const T& right) const noexcept
		{
			return String::Compare(left, right, StringActionFlag::IgnoreCase);
		}
	};

	struct StringEqualToIC: public StringCompareIC
	{
		template<class T>
		bool operator()(const T& left, const T& right) const noexcept
		{
			return Compare(left, right) == 0;
		}
	};

	struct StringLessThanIC: public StringCompareIC
	{
		template<class T>
		bool operator()(const T& left, const T& right) const noexcept
		{
			return Compare(left, right) < 0;
		}
	};

	struct StringGreaterThanIC: public StringCompareIC
	{
		template<class T>
		bool operator()(const T& left, const T& right) const noexcept
		{
			return Compare(left, right) > 0;
		}
	};

	struct StringHashIC
	{
		// From Boost
		template<class T>
		static void hash_combine(size_t& seed, const T& v) noexcept
		{
			std::hash<T> hasher;
			seed ^= hasher(v) + static_cast<size_t>(0x9e3779b9u) + (seed << 6) + (seed >> 2);
		}

		template<class T>
		size_t operator()(const T& value) const noexcept
		{
			size_t hash = 0;
			for (UniChar c: value)
			{
				hash_combine(hash, c.ToLowerCase().GetValue());
			}
			return hash;
		}
	};

	template<class TKey, class TValue>
	using MapIC = std::map<TKey, TValue, StringLessThanIC>;

	template<class TValue>
	using SetIC = std::set<TValue, StringLessThanIC>;

	template<class TKey, class TValue>
	using UnorderedMapIC = std::unordered_map<TKey, TValue, StringHashIC, StringEqualToIC>;
	
	template<class TValue>
	using UnorderedSetIC = std::unordered_set<TValue, StringHashIC, StringEqualToIC>;
}

namespace kxf::Utility
{
	class StringBuffer final
	{
		private:
			enum class Type
			{
				None = -1,

				NarrowChars,
				WideChars
			};

		private:
			String& m_Value;
			size_t m_Length = 0;
			std::vector<std::byte> m_Buffer;
			IEncodingConverter* m_EncodingConverter = nullptr;
			Type m_Type = Type::None;
			bool m_NullTerminated = false;

		private:
			char* PrepareNarrowChars();
			wchar_t* PrepareWideChars();
			void Finalize();

		public:
			StringBuffer(String& value, size_t length, bool nullTerminated = false) noexcept
				:m_Value(value), m_Length(length), m_NullTerminated(nullTerminated)
			{
			}
			StringBuffer(String& value, size_t length, IEncodingConverter& encondigConverter, bool nullTerminated = false) noexcept
				:m_Value(value), m_Length(length), m_EncodingConverter(&encondigConverter), m_NullTerminated(nullTerminated)
			{
			}
			~StringBuffer()
			{
				Finalize();
			}

		public:
			char* nc_str()
			{
				return PrepareNarrowChars();
			}
			wchar_t* wc_str()
			{
				return PrepareWideChars();
			}
			size_t length() const noexcept
			{
				return m_Length;
			}

		public:
			operator char*()
			{
				return PrepareNarrowChars();
			}
			operator wchar_t*()
			{
				return PrepareWideChars();
			}
	};
}

namespace kxf::Utility
{
	std::optional<bool> ParseBool(const String& value);
}
