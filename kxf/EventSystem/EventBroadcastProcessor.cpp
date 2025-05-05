#include "kxf-pch.h"
#include "EventBroadcastProcessor.h"
#include "kxf/Core/Enumerator.h"

namespace kxf::EventSystem
{
	bool BroadcastProcessorHandler::TryBefore(IEvent& event)
	{
		for (IEvtHandler& evtHandler: m_Processor.EnumReceiveres())
		{
			evtHandler.ProcessEvent(event, event.GetEventID(), ProcessEventFlag::Locally);
			return true;
		};
		return true;
	}
}

namespace kxf::EventSystem
{
	bool BroadcastReceiverHandler::TryBefore(IEvent& event)
	{
		TryHereOnly(event);
		return true;
	}
}

namespace kxf
{
	bool EventBroadcastProcessor::AddReceiver(EventBroadcastReceiver& reciever)
	{
		return m_Stack.Push(reciever.GetEvtHandler());
	}
	bool EventBroadcastProcessor::RemoveReceiver(EventBroadcastReceiver& reciever)
	{
		return m_Stack.Remove(reciever.GetEvtHandler());
	}

	Enumerator<IEvtHandler&> EventBroadcastProcessor::EnumReceiveres(Order order) const
	{
		return m_Stack.EnumItems(order, true);
	}
	Enumerator<IEvtHandler&> EventBroadcastProcessor::EnumReceiveres() const
	{
		return m_Stack.EnumItems(m_Order, true);
	}

	bool EventBroadcastReceiver::PreProcessEvent(IEvent& event)
	{
		return m_Processor.PreProcessEvent(event);
	}
	void EventBroadcastReceiver::PostProcessEvent(IEvent& event)
	{
		m_Processor.PostProcessEvent(event);
		event.Skip();
	}
}
