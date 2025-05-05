#pragma once
#include "../Common.h"
#include "kxf/Core/CallbackFunction.h"
#include "kxf/Localization/Locale.h"

namespace kxf::System::Private
{
	String FormatMessage(const void* source, uint32_t messageID, FlagSet<uint32_t> flags = {}, const Locale& locale = {}) noexcept;
	CallbackResult<size_t> EnumWindows(CallbackFunction<void*, uint32_t, uint32_t> func, std::optional<uint32_t> pid = {}, std::optional<uint32_t> tid = {});
	CallbackResult<size_t> EnumThreads(CallbackFunction<uint32_t, uint32_t> func, std::optional<uint32_t> pid = {}, std::optional<uint32_t> tid = {});

	String ResourceTypeToName(size_t id);
	String ResourceTypeToName(const wchar_t* id);
	const wchar_t* MakeIntResource(int resID);

	FlagSet<uint32_t> MapSystemStandardAccess(FlagSet<SystemStandardAccess> access) noexcept;
	FlagSet<uint32_t> MapSystemThreadAccess(FlagSet<SystemThreadAccess> access) noexcept;
	FlagSet<uint32_t> MapSystemProcessAccess(FlagSet<SystemProcessAccess> access) noexcept;

	std::optional<uint32_t> MapSystemProcessPriority(SystemProcessPriority priority) noexcept;
	SystemProcessPriority MapSystemProcessPriority(uint32_t priority) noexcept;

	String GetKernelObjectName(const String& name, KernelObjectNamespace ns);
}
