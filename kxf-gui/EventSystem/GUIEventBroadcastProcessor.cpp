#include "kxf-pch.h"
#include "GUIEventBroadcastProcessor.h"
#include "kxf-gui/Widgets/IWidgetEvent.h"

namespace kxf
{
	void GUIEventBroadcastReceiver::StopPropagation(IEvent& event)
	{
		if (auto widgetEvent = event.QueryInterface<IWidgetEvent>())
		{
			widgetEvent->StopPropagation();
		}
	}

	bool GUIEventBroadcastReceiver::PreProcessEvent(IEvent& event)
	{
		StopPropagation(event);
		return EventBroadcastReceiver::PreProcessEvent(event);
	}
	void GUIEventBroadcastReceiver::PostProcessEvent(IEvent& event)
	{
		EventBroadcastReceiver::PostProcessEvent(event);

		event.Skip();
		StopPropagation(event);
	}
}
