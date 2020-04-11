#include "KxStdAfx.h"
#include "SecretValue.h"

namespace KxFramework
{
	String SecretValue::GetAsString(const wxMBConv& conv) const
	{
		return wxString(m_Storage.data(), conv, m_Storage.size());
	}
	void SecretValue::Wipe() noexcept
	{
		if (!m_Storage.empty())
		{
			::RtlSecureZeroMemory(m_Storage.data(), m_Storage.size());
			m_Storage.clear();
		}
	}
}
