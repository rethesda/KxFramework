#pragma once
#include "Common.h"

#define kxf_DefineLogCategory(name)						constexpr kxf::StringView name = kxfS(#name)
#define kxf_DefineLogModuleCategory(module, name)		constexpr kxf::StringView module##_##name = kxfS(#module "/" #name)

namespace kxf::LogCategory
{
	kxf_DefineLogCategory(kxf);
	kxf_DefineLogCategory(WinAPI);
	kxf_DefineLogCategory(wxWidgets);

	kxf_DefineLogModuleCategory(kxf, Application);
	kxf_DefineLogModuleCategory(kxf, Async);
	kxf_DefineLogModuleCategory(kxf, Compression);
	kxf_DefineLogModuleCategory(kxf, Core);
	kxf_DefineLogModuleCategory(kxf, Crypto);
	kxf_DefineLogModuleCategory(kxf, Drawing);
	kxf_DefineLogModuleCategory(kxf, EventSystem);
	kxf_DefineLogModuleCategory(kxf, FileSystem);
	kxf_DefineLogModuleCategory(kxf, IO);
	kxf_DefineLogModuleCategory(kxf, IPC);
	kxf_DefineLogModuleCategory(kxf, Localization);
	kxf_DefineLogModuleCategory(kxf, Network);
	kxf_DefineLogModuleCategory(kxf, RTTI);
	kxf_DefineLogModuleCategory(kxf, Sciter);
	kxf_DefineLogModuleCategory(kxf, Serialization);
	kxf_DefineLogModuleCategory(kxf, System);
	kxf_DefineLogModuleCategory(kxf, Threading);
	kxf_DefineLogModuleCategory(kxf, UI);
	kxf_DefineLogModuleCategory(kxf, Utility);
	kxf_DefineLogModuleCategory(kxf, wxWidgets);
}
