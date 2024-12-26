#include "kxf-pch.h"
#include "IdleEvent.h"
#include "wx/event.h"

namespace kxf
{
	kxf::IdleEventMode IdleEvent::GetMode() noexcept
	{
		switch (wxIdleEvent::GetMode())
		{
			case wxIDLE_PROCESS_ALL:
			{
				return IdleEventMode::ProcessAll;
			}
			case wxIDLE_PROCESS_SPECIFIED:
			{
				return IdleEventMode::ProcessSpecific;
			}
		};
		return IdleEventMode::ProcessAll;
	}
	void IdleEvent::SetMode(IdleEventMode mode) noexcept
	{
		switch (mode)
		{
			case IdleEventMode::ProcessAll:
			{
				wxIdleEvent::SetMode(wxIDLE_PROCESS_ALL);
				break;
			}
			case IdleEventMode::ProcessSpecific:
			{
				wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);
				break;
			}
			default:
			{
				wxIdleEvent::SetMode(wxIDLE_PROCESS_ALL);
				break;
			}
		};
	}
}
