#include "kxf-pch.h"
#include "MasterStylesheetStorage.h"
#include "../SciterAPI.h"
#include "../Widget.h"
#include "../Private/Conversion.h"

namespace kxf::Sciter
{
	bool MasterStylesheetStorage::ApplyGlobally() const
	{
		// Reset the master CSS
		constexpr BYTE null[sizeof(void*)] = {};
		GetSciterAPI()->SciterSetMasterCSS(null, 0);
		
		// Apply new master styles
		auto result = EnumItems([](const String& css)
		{
			auto utf8 = ToSciterUTF8(css);
			if (!GetSciterAPI()->SciterAppendMasterCSS(utf8.data(), utf8.size()))
			{
				return CallbackCommand::Terminate;
			}
			return CallbackCommand::Continue;
		});
		return !result.RequestedToTerminate();
	}
}
