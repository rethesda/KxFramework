#include "kxf-pch.h"
#include "SystemOptions.h"
#include <wx/sysopt.h>
#include "kxf/Win32/UndefMacros.h"

namespace kxf::wxWidgets
{
	String SystemOptions::GetOptionString(const String& name)
	{
		return wxSystemOptions::GetOption(name);
	}
	Color SystemOptions::GetOptionColor(const String& name)
	{
		if (wxSystemOptions::HasOption(name))
		{
			return Color::FromCOLORREF(static_cast<uint32_t>(wxSystemOptions::GetOptionInt(name)));
		}
		return {};
	}
	std::optional<bool> SystemOptions::GetOptionBool(const String& name)
	{
		if (wxSystemOptions::HasOption(name))
		{
			return wxSystemOptions::GetOptionInt(name) != 0;
		}
		return {};
	}
	std::optional<int> SystemOptions::GetOptionInt(const String& name)
	{
		if (wxSystemOptions::HasOption(name))
		{
			return wxSystemOptions::GetOptionInt(name);
		}
		return {};
	}

	void SystemOptions::SetOption(const String& name, const String& value)
	{
		wxSystemOptions::SetOption(name, value);
	}
	void SystemOptions::SetOption(const String& name, const Color& value)
	{
		wxSystemOptions::SetOption(name, static_cast<int>(value.GetCOLORREF()));
	}
	void SystemOptions::SetOption(const String& name, bool value)
	{
		wxSystemOptions::SetOption(name, value ? 1 : 0);
	}
	void SystemOptions::SetOption(const String& name, int value)
	{
		wxSystemOptions::SetOption(name, value);
	}
}
