#include "kxf-pch.h"
#include "EventBroadcastProcessor.h"

namespace
{
	constexpr kxf::EvtHandlerStack::Order g_DefaultOrder = kxf::EvtHandlerStack::Order::LastToFirst;
}

namespace kxf::EventSystem
{
	bool BroadcastProcessorHandler::TryBefore(IEvent& event)
	{
		m_Processor.EnumReceivers([&](IEvtHandler& evtHandler)
		{
			evtHandler.ProcessEvent(event, event.GetEventID(), ProcessEventFlag::Locally);
			return CallbackCommand::Terminate; // Really terminate though? Check later
		});
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
	EventBroadcastProcessor::EventBroadcastProcessor()
		:EvtHandlerDelegate(m_EvtHandler), m_EvtHandler(*this), m_Stack(m_EvtHandler), m_Order(g_DefaultOrder)
	{
	}

	bool EventBroadcastProcessor::AddReceiver(EventBroadcastReceiver& reciever)
	{
		return m_Stack.Push(reciever.GetEvtHandler());
	}
	bool EventBroadcastProcessor::RemoveReceiver(EventBroadcastReceiver& reciever)
	{
		return m_Stack.Remove(reciever.GetEvtHandler());
	}

	CallbackResult<void> EventBroadcastProcessor::EnumReceivers(CallbackFunction<IEvtHandler&> func, Order order) const
	{
		switch (order)
		{
			case Order::FirstToLast:
			{
				return m_Stack.EnumItems(std::move(func), EvtHandlerStack::Order::FirstToLast, true);
			}
			case Order::LastToFirst:
			{
				return m_Stack.EnumItems(std::move(func), EvtHandlerStack::Order::LastToFirst, true);
			}
			default:
			{
				return m_Stack.EnumItems(std::move(func), m_Order, true);
			}
		};
	}

	auto EventBroadcastProcessor::GetReceiversOrder() const -> Order
	{
		switch (m_Order)
		{
			case EvtHandlerStack::Order::FirstToLast:
			{
				return Order::FirstToLast;
			}
			default:
			{
				return Order::LastToFirst;
			}
		};
	}
	void EventBroadcastProcessor::SetReceiversOrder(Order order)
	{
		switch (order)
		{
			case Order::FirstToLast:
			{
				m_Order = EvtHandlerStack::Order::FirstToLast;
				break;
			}
			case Order::LastToFirst:
			{
				m_Order = EvtHandlerStack::Order::LastToFirst;
				break;
			}
			default:
			{
				m_Order = g_DefaultOrder;
			}
		};
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
