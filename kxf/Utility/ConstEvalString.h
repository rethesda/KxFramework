#pragma once
#include "kxf/Common.hpp"
#include <string_view>
#include <utility>

namespace kxf::Utility
{
	template<class TChar, size_t N>
	struct ConstEvalString final
	{
		public:
			TChar m_Value[N];

		public:
			consteval ConstEvalString(const TChar (&str)[N])
			{
				std::copy_n(str, N, m_Value);
			}

		public:
			consteval const TChar* data() const noexcept
			{
				return m_Value;
			}
			consteval std::basic_string_view<TChar> view() const noexcept
			{
				return {m_Value, size()};
			}
			consteval size_t size() const noexcept
			{
				return N != 0 ? N - 1 : N;
			}

			consteval TChar at(size_t i) const noexcept
			{
				return m_Value[i];
			}
			consteval TChar operator[](size_t i) const noexcept
			{
				return m_Value[i];
			}

		public:
			consteval bool operator==(const ConstEvalString& other) const noexcept
			{
				return std::equal(other.m_Value, other.m_Value + N, m_Value);
			}

			template<size_t size>
			consteval bool operator==(const ConstEvalString<TChar, size> other) const
			{
				if (N == size)
				{
					return std::equal(other.m_Value, other.m_Value + N, m_Value);
				}
				return false;
			}
	};

	template<size_t N>
	using ConstEvalStringN = ConstEvalString<char, N>;

	template<size_t N>
	using ConstEvalStringW = ConstEvalString<wchar_t, N>;
}
