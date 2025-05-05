#pragma once
#include "EventWrapper.h"
#include "ClientObject.h"
#include "IWithEvent.h"
#include "kxf/EventSystem/EvtHandler.h"
#include "kxf/Core/OptionalPtr.h"
#include <wx/event.h>
#include "kxf/Win32/UndefMacros.h"

namespace kxf::wxWidgets
{
	inline bool ForwardBind(IEvtHandler& evtHandler, wxEvtHandler& evtHandlerWx, EventSystem::EventItem& eventItem)
	{
		if (EventID id = eventItem.GetEventID(); id.IsWXID())
		{
			evtHandlerWx.Bind(wxEventTypeTag<wxEvent>(id.AsInt()), [evtHandler = &evtHandler, executor = eventItem.GetExecutor()](wxEvent& event)
			{
				EventWrapper wrapper(event);
				executor->Execute(*evtHandler, wrapper);
			}, wxID_ANY, wxID_ANY, new ClientObject(eventItem.GetBindSlot()));
			return true;
		}
		return false;
	}
	inline bool InjectBeforeEvtHandler(IEvtHandler& evtHandler, wxEvent& event)
	{
		wxWidgets::EventWrapper wrapper(event);
		return evtHandler.ProcessEvent(wrapper, event.GetEventType(), ProcessEventFlag::Locally);
	}
}

namespace kxf::wxWidgets
{
	namespace Private
	{
		template<class TEvent, class TFunc>
		void DoCallWxEvent(IEvent& event, TFunc&& func) noexcept(std::is_nothrow_invocable_v<TFunc, TEvent&>)
		{
			if (auto withEvent = event.QueryInterface<IWithEvent>())
			{
				std::invoke(func, static_cast<TEvent&>(withEvent->GetEvent()));
			}
			else
			{
				event.Skip();
			}
		}
	}

	// Bind free or static function
	template<std::derived_from<wxEvent> TEvent, class TEventArg>
	LocallyUniqueID BindWXEvent(IEvtHandler& evtHandler, wxEventTypeTag<TEvent> eventTag, void(*func)(TEventArg&), FlagSet<BindEventFlag> flags = BindEventFlag::Direct)
	{
		return evtHandler.Bind(EventTag<IEvent>(eventTag), [func](IEvent& event)
		{
			Private::DoCallWxEvent<TEventArg>(event, func);
		}, flags);
	}

	// Bind a generic callable
	template<std::derived_from<wxEvent> TEvent, class TCallable>
	LocallyUniqueID BindWXEvent(IEvtHandler& evtHandler, wxEventTypeTag<TEvent> eventTag, TCallable&& callable, FlagSet<BindEventFlag> flags = BindEventFlag::Direct)
	{
		return evtHandler.Bind(EventTag<IEvent>(eventTag), [callable = std::move(callable)](IEvent& event) mutable
		{
			Private::DoCallWxEvent<TEvent>(event, callable);
		}, flags);
	}

	// Bind a member function
	template<std::derived_from<wxEvent> TEvent, class TClass, class TEventArg, class TEventHandler>
	LocallyUniqueID BindWXEvent(IEvtHandler& evtHandler, wxEventTypeTag<TEvent> eventTag, void(TClass::* method)(TEventArg&), TEventHandler* handler, FlagSet<BindEventFlag> flags = BindEventFlag::Direct)
	{
		return evtHandler.Bind(EventTag<IEvent>(eventTag), [method, handler](IEvent& event)
		{
			Private::DoCallWxEvent<TEventArg>(event, [&](TEventArg& event)
			{
				std::invoke(method, handler, event);
			});
		}, flags);
	}
}

namespace kxf::wxWidgets
{
	class EvtHandlerWrapper: public EvtHandler
	{
		private:
			optional_ptr<wxEvtHandler> m_EvtHandler;

		protected:
			bool OnDynamicBind(EventItem& eventItem) override
			{
				if (EvtHandler::OnDynamicBind(eventItem))
				{
					ForwardBind(*this, *m_EvtHandler, eventItem);
					return true;
				}
				return false;
			}
			bool OnDynamicUnbind(EventItem& eventItem) override
			{
				// We can't really reliably unbind event in 'wxEvtHandler' given our internals
				return EvtHandler::OnDynamicUnbind(eventItem);
			}

		public:
			EvtHandlerWrapper(wxEvtHandler& evtHandler)
				:m_EvtHandler(evtHandler)
			{
			}
			EvtHandlerWrapper(std::unique_ptr<wxEvtHandler> evtHandler)
				:m_EvtHandler(std::move(evtHandler))
			{
			}
			EvtHandlerWrapper(EvtHandlerWrapper&& other) noexcept = default;
			EvtHandlerWrapper(const EvtHandlerWrapper&) = delete;

		public:
			EvtHandlerWrapper& operator=(EvtHandlerWrapper&& other) noexcept = default;
			EvtHandlerWrapper& operator=(const EvtHandlerWrapper&) = delete;
	};
}
