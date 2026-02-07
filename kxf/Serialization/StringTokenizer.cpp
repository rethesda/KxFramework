#include "kxf-pch.h"
#include "StringTokenizer.h"
#include "kxf/Utility/ScopeGuard.h"

namespace kxf
{
	void StringTokenizer::UpdateCurrentToken()
	{
		if (m_PreviousPosition < m_Source.end())
		{
			m_CurrentToken = StringView(m_PreviousPosition, m_CurrentPosition);
		}
		else
		{
			m_CurrentToken = {};
		}
	}

	UniChar StringTokenizer::PeekPrevious() const noexcept
	{
		if (m_CurrentPosition != m_Source.begin())
		{
			return *(m_CurrentPosition - 1);
		}
		return {};
	}
	UniChar StringTokenizer::PeekCurrent() const noexcept
	{
		if (!m_Source.empty() && !EndReached())
		{
			return *m_CurrentPosition;
		}
		return {};
	}
	UniChar StringTokenizer::PeekNext() const noexcept
	{
		if (!EndReached() && (m_CurrentPosition + 1) != m_Source.end())
		{
			return *(m_CurrentPosition + 1);
		}
		return {};
	}

	void StringTokenizer::Advance(size_t count)
	{
		if (!EndReached())
		{
			Utility::ScopeGuard atExit = [&]()
			{
				UpdateCurrentToken();
			};
			m_CurrentPosition += count;
		}
	}
	bool StringTokenizer::ScanUntil(CallbackFunction<UniChar> func) noexcept
	{
		SavePrevPosition();
		Utility::ScopeGuard atExit = [&]()
		{
			UpdateCurrentToken();
		};

		for (; m_CurrentPosition != m_Source.end(); ++m_CurrentPosition)
		{
			func.Invoke(*m_CurrentPosition);
			if (func.ShouldTerminate())
			{
				return true;
			}
		}
		return false;
	}
	bool StringTokenizer::ScanUntil(UniChar c) noexcept
	{
		return ScanUntil([&](UniChar a)
		{
			return a == c ? CallbackCommand::Terminate : CallbackCommand::Continue;
		});
	}
	bool StringTokenizer::ScanUntil(StringView pattern) noexcept
	{
		if (!pattern.empty())
		{
			return ScanUntil([&](UniChar a)
			{
				size_t index = m_Source.find(pattern, GetCurrentPosition());
				if (index != StringView::npos)
				{
					m_CurrentPosition = m_Source.begin() + index + pattern.size();
					return CallbackCommand::Terminate;
				}
				return CallbackCommand::Continue;
			});
		}
		return false;
	}
	void StringTokenizer::Skip(UniChar c) noexcept
	{
		for (; m_CurrentPosition != m_Source.end(); ++m_CurrentPosition)
		{
			if (*m_CurrentPosition != c)
			{
				break;
			}
		}
		m_PreviousPosition = m_CurrentPosition;
	}
	void StringTokenizer::SkipWhitespace() noexcept
	{
		for (; m_CurrentPosition != m_Source.end(); ++m_CurrentPosition)
		{
			if (!UniChar(*m_CurrentPosition).IsWhitespace())
			{
				break;
			}
		}
		m_PreviousPosition = m_CurrentPosition;
	}
}
