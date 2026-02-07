#pragma once
#include "Common.h"
#include "kxf/Core/String.h"
#include "kxf/Core/UniChar.h"
#include "kxf/Core/CallbackFunction.h"

namespace kxf
{
	class StringTokenizer final
	{
		private:
			StringView m_Source;
			StringView::iterator m_CurrentPosition;
			StringView::iterator m_PreviousPosition;
			StringView m_CurrentToken;

		private:
			void SavePrevPosition()
			{
				m_PreviousPosition = m_CurrentPosition;
			}
			void UpdateCurrentToken();

		public:
			StringTokenizer(StringView ref) noexcept
				:m_Source(ref), m_CurrentPosition(ref.begin())
			{
			}

		public:
			bool IsEmpty() const noexcept
			{
				return m_Source.empty();
			}
			bool EndReached() const noexcept
			{
				return m_Source.empty() || m_CurrentPosition >= m_Source.end();
			}
			void Reset() noexcept
			{
				m_CurrentPosition = m_Source.begin();
				m_PreviousPosition = m_CurrentPosition;
			}

			size_t GetLength() const noexcept
			{
				return m_Source.length();
			}
			size_t GetCurrentPosition() const noexcept
			{
				return std::distance(m_Source.begin(), m_CurrentPosition);
			}
			size_t GetPreviousPosition() const noexcept
			{
				return std::distance(m_Source.begin(), m_PreviousPosition);
			}

			UniChar PeekPrevious() const noexcept;
			UniChar PeekCurrent() const noexcept;
			UniChar PeekNext() const noexcept;

			void Advance(size_t count = 1);
			bool ScanUntil(CallbackFunction<UniChar> func) noexcept;
			bool ScanUntil(UniChar c) noexcept;
			bool ScanUntil(StringView pattern) noexcept;
			void Skip(UniChar c) noexcept;
			void SkipWhitespace() noexcept;

			StringView GetCurrentToken() const noexcept
			{
				return m_CurrentToken;
			}
	};
}
