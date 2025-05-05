#include "kxf-pch.h"
#include "MapEventSystem.h"
#include <wx/event.h>
#include <wx/windowid.h>

namespace kxf::wxWidgets
{
	FlagSet<wxEventCategory> MapEventCategory(FlagSet<EventCategory> eventCategory) noexcept
	{
		FlagSet<wxEventCategory> result;
		result.Add(wxEventCategory::wxEVT_CATEGORY_UI, eventCategory.Contains(EventCategory::UserInterface));
		result.Add(wxEventCategory::wxEVT_CATEGORY_USER_INPUT, eventCategory.Contains(EventCategory::UserInput));
		result.Add(wxEventCategory::wxEVT_CATEGORY_SOCKET, eventCategory.Contains(EventCategory::Socket));
		result.Add(wxEventCategory::wxEVT_CATEGORY_TIMER, eventCategory.Contains(EventCategory::Timer));
		result.Add(wxEventCategory::wxEVT_CATEGORY_THREAD, eventCategory.Contains(EventCategory::Thread));
		result.Add(wxEventCategory::wxEVT_CATEGORY_UNKNOWN, eventCategory.Contains(EventCategory::Unknown));
		result.Add(wxEventCategory::wxEVT_CATEGORY_CLIPBOARD, eventCategory.Contains(EventCategory::Clipboard));

		return result;
	}
	FlagSet<EventCategory> MapEventCategory(FlagSet<wxEventCategory> eventCategory) noexcept
	{
		FlagSet<EventCategory> result;
		result.Add(EventCategory::UserInterface, eventCategory.Contains(wxEventCategory::wxEVT_CATEGORY_UI));
		result.Add(EventCategory::UserInput, eventCategory.Contains(wxEventCategory::wxEVT_CATEGORY_USER_INPUT));
		result.Add(EventCategory::Socket, eventCategory.Contains(wxEventCategory::wxEVT_CATEGORY_SOCKET));
		result.Add(EventCategory::Timer, eventCategory.Contains(wxEventCategory::wxEVT_CATEGORY_TIMER));
		result.Add(EventCategory::Thread, eventCategory.Contains(wxEventCategory::wxEVT_CATEGORY_THREAD));
		result.Add(EventCategory::Unknown, eventCategory.Contains(wxEventCategory::wxEVT_CATEGORY_UNKNOWN));
		result.Add(EventCategory::Clipboard, eventCategory.Contains(wxEventCategory::wxEVT_CATEGORY_CLIPBOARD));

		return result;
	}
}
