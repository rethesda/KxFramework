/*
Copyright � 2018 Kerber. All rights reserved.

You should have received a copy of the GNU LGPL v3
along with KxFramework. If not, see https://www.gnu.org/licenses/lgpl-3.0.html.
*/
#pragma once
#include "KxFramework/KxFramework.h"
#include "KxFramework/KxUtility.h"

template<class T, T defaultFlags = (T)0>
class KxWithOptions
{
	public:
		using EnumT = T;

	private:
		EnumT m_OptionFlags = defaultFlags;

	protected:
		EnumT GetOptionFlags() const
		{
			return m_OptionFlags;
		}
		void SetOptionFlags(EnumT options)
		{
			m_OptionFlags = options;
		}

	public:
		bool IsOptionEnabled(EnumT option) const
		{
			return m_OptionFlags & option;
		}
		void SetOptionEnabled(EnumT option, bool enable = true)
		{
			if (enable)
			{
				m_OptionFlags = static_cast<EnumT>(m_OptionFlags | option);
			}
			else
			{
				m_OptionFlags = static_cast<EnumT>(m_OptionFlags & (~option));
			}
		}
};
