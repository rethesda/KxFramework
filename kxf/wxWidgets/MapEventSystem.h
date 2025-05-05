#pragma once
#include "kxf/EventSystem/Common.h"
enum wxEventCategory;

namespace kxf::wxWidgets
{
	FlagSet<wxEventCategory> MapEventCategory(FlagSet<EventCategory> eventCategory) noexcept;
	FlagSet<EventCategory> MapEventCategory(FlagSet<wxEventCategory> seekMode) noexcept;
}
