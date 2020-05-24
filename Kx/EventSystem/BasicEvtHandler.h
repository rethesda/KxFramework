#pragma once
#include "Event.h"
#include "EventBuilder.h"
#include "Events/IndirectCallEvent.h"
#include <optional>

namespace kxf
{
	class KX_API BasicEvtHandler
	{
		friend class EventSystem::EventBuilderBase;

		public:
			static void AddFilter(wxEventFilter& filter)
			{
				wxEvtHandler::AddFilter(&filter);
			}
			static void RemoveFilter(wxEventFilter& filter)
			{
				wxEvtHandler::RemoveFilter(&filter);
			}

		private:
			wxEvtHandler& m_EvtHandler;

		private:
			void ConfigureEvent(wxEvent& event, std::optional<EventID> eventID = {})
			{
				event.SetEventObject(&m_EvtHandler);
				if (eventID)
				{
					event.SetEventType(*eventID);
				}
			}
			bool IsValidEventID(EventID eventID) const
			{
				return eventID != Event::EvtNull && eventID != Event::EvtAny;
			}

			bool DoProcessEvent(wxEvent& event, std::optional<EventID> eventID = {})
			{
				ConfigureEvent(event, eventID);
				return GetTargetHandler().ProcessEvent(event);
			}
			void DoQueueEvent(std::unique_ptr<wxEvent> event, std::optional<EventID> eventID = {})
			{
				ConfigureEvent(*event, eventID);
				GetTargetHandler().QueueEvent(event.release());
			}

		public:
			BasicEvtHandler(wxEvtHandler& evtHandler)
				:m_EvtHandler(evtHandler)
			{
			}
			virtual ~BasicEvtHandler() = default;

		public:
			wxEvtHandler& GetThisHandler()
			{
				return m_EvtHandler;
			}
			const wxEvtHandler& GetThisHandler() const
			{
				return m_EvtHandler;
			}

			virtual wxEvtHandler& GetTargetHandler() = 0;
			const wxEvtHandler& GetTargetHandler() const
			{
				return const_cast<BasicEvtHandler*>(this)->GetTargetHandler();
			}
			
			bool IsEvtHandlerEnabled() const
			{
				return m_EvtHandler.GetEvtHandlerEnabled();
			}
			void EnableEvtHandler(bool enabled = true)
			{
				m_EvtHandler.SetEvtHandlerEnabled(enabled);
			}

		public:
			// EvtHandlers chain
			wxEvtHandler* GetPreviousHandler() const
			{
				return m_EvtHandler.GetPreviousHandler();
			}
			wxEvtHandler* GetNextHandler() const
			{
				return m_EvtHandler.GetNextHandler();
			}
		
			void SetPrevHandler(wxEvtHandler* evtHandler)
			{
				m_EvtHandler.SetPreviousHandler(evtHandler);
			}
			void SetNextHandler(wxEvtHandler* evtHandler)
			{
				m_EvtHandler.SetNextHandler(evtHandler);
			}

			void Unlink()
			{
				m_EvtHandler.Unlink();
			}
			bool IsUnlinked() const
			{
				return m_EvtHandler.IsUnlinked();
			}

		public:
			// User-supplied data
			void* GetClientData() const
			{
				return m_EvtHandler.GetClientData();
			}
			void SetClientData(void* clientData)
			{
				m_EvtHandler.SetClientData(clientData);
			}
		
			wxClientData* GetClientObject() const
			{
				return m_EvtHandler.GetClientObject();
			}
			void SetClientObject(std::unique_ptr<wxClientData> clientObject)
			{
				m_EvtHandler.SetClientObject(clientObject.release());
			}

		public:
			// Bind free or static function
			template<class TEvent, class TEventArg>
			bool Bind(EventTag<TEvent> eventTag, void(*func)(TEventArg&))
			{
				if (IsValidEventID(eventTag))
				{
					m_EvtHandler.Bind(eventTag, func);
					return true;
				}
				return false;
			}
		
			// Unbind free or static function
			template<class TEvent, class TEventArg>
			bool Unbind(EventTag<TEvent> eventTag, void(*func)(TEventArg&))
			{
				if (IsValidEventID(eventTag))
				{
					return m_EvtHandler.Unbind(eventTag, func);
				}
				return false;
			}

			// Bind functor or a lambda function
			template<class TEvent, class TFunctor>
			bool Bind(EventTag<TEvent> eventTag, const TFunctor& func)
			{
				if (IsValidEventID(eventTag))
				{
					m_EvtHandler.Bind(eventTag, func);
					return true;
				}
				return false;
			}

			// Unbind functor or lambda function
			template<class TEvent, class TFunctor>
			bool Unbind(EventTag<TEvent> eventTag, const TFunctor& func)
			{
				if (IsValidEventID(eventTag))
				{
					return m_EvtHandler.Unbind(eventTag, func);
				}
				return false;
			}

			// Bind a member function
			template<class TEvent, class TClass, class TEventArg, class TEventHandler>
			bool Bind(EventTag<TEvent> eventTag, void(TClass::* method)(TEventArg&), TEventHandler* handler)
			{
				if (IsValidEventID(eventTag))
				{
					m_EvtHandler.Bind(eventTag, method, handler);
					return true;
				}
				return false;
			}

			template<class TEvent, class TClass, class TEventArg>
			bool Bind(EventTag<TEvent> eventTag, void(TClass::* method)(TEventArg&))
			{
				if (IsValidEventID(eventTag))
				{
					m_EvtHandler.Bind(eventTag, method, static_cast<TClass*>(&m_EvtHandler));
					return false;
				}
				return false;
			}

			// Unbind a member function
			template<class TEvent, class TClass, class TEventArg, class TEventHandler>
			bool Unbind(EventTag<TEvent> eventTag, void(TClass::* method)(TEventArg&), TEventHandler* handler)
			{
				if (IsValidEventID(eventTag))
				{
					return m_EvtHandler.Unbind(eventTag, method, handler);
				}
				return false;
			}

			template<class TEvent, class TClass, class TEventArg>
			bool Unbind(EventTag<TEvent> eventTag, void(TClass::* method)(TEventArg&))
			{
				if (IsValidEventID(eventTag))
				{
					return m_EvtHandler.Unbind(eventTag, method, static_cast<TClass*>(&m_EvtHandler));
				}
				return false;
			}

		public:
			// Regular event sending functions
			bool ProcessEvent(wxEvent& event, std::optional<EventID> eventID = {})
			{
				return DoProcessEvent(event, eventID);
			}
			bool ProcessEventLocally(wxEvent& event, std::optional<EventID> eventID = {})
			{
				ConfigureEvent(event, eventID);
				return GetTargetHandler().ProcessEventLocally(event);
			}
			bool SafelyProcessEvent(wxEvent& event, std::optional<EventID> eventID = {})
			{
				ConfigureEvent(event, eventID);
				return GetTargetHandler().SafelyProcessEvent(event);
			}
			
			template<class TEvent, class... Args> bool ProcessEvent(Args&&... arg)
			{
				TEvent event(std::forward<Args>(arg)...);
				return DoProcessEvent(event);
			}
			template<class TEvent, class... Args> bool ProcessEvent(EventTag<TEvent> eventTag, Args&&... arg)
			{
				TEvent event(std::forward<Args>(arg)...);
				return DoProcessEvent(event, eventTag);
			}

			void QueueEvent(std::unique_ptr<Event> event, std::optional<EventID> eventID = {})
			{
				DoQueueEvent(std::move(event), eventID);
			}
			template<class TEvent, class... Args> void QueueEvent(Args&&... arg)
			{
				DoQueueEvent(std::make_unique<TEvent>(std::forward<Args>(arg)...));
			}
			template<class TEvent, class... Args> void QueueEvent(EventTag<TEvent> eventTag, Args&&... arg)
			{
				auto event = std::make_unique<TEvent>(std::forward<Args>(arg)...);
				DoQueueEvent(std::move(event), eventTag);
			}

			void AddPendingEvent(const wxEvent& event, std::optional<EventID> eventID = {})
			{
				ConfigureEvent(const_cast<wxEvent&>(event), eventID);
				GetTargetHandler().AddPendingEvent(event);
			}
			void ProcessPendingEvents()
			{
				GetTargetHandler().ProcessPendingEvents();
			}
			void DeletePendingEvents()
			{
				GetTargetHandler().DeletePendingEvents();
			}

			// Construct and send the event using event builder
			template<class TEvent, class... Args>
			auto ProcessEventEx(EventTag<TEvent> eventTag, Args&&... arg)
			{
				TEvent event(std::forward<Args>(arg)...);
				return EventSystem::EventBuilder<TEvent>(*this, event, eventTag);
			}

			template<class TEvent, class... Args>
			auto QueueEventEx(EventTag<TEvent> eventTag, Args&&... arg)
			{
				auto event = std::make_unique<TEvent>(std::forward<Args>(arg)...);
				return EventSystem::EventBuilder<TEvent>(*this, std::move(event), eventTag);
			}

			// Queue execution of a class member, free, static, lambda function or a class which implements 'operator()'
			template<class TCallable, class... Args>
			void CallAfter(TCallable&& callable, Args&&... arg)
			{
				using namespace kxf;
				using TCallableTraits = typename Utility::CallableTraits<TCallable, Args...>;

				if constexpr(TCallableTraits::IsMemberFunction)
				{
					DoQueueEvent(std::make_unique<EventSystem::MethodIndirectCall<TCallable, Args...>>(m_EvtHandler, std::forward<TCallable>(callable), std::forward<Args>(arg)...));
				}
				else if constexpr(TCallableTraits::IsFreeFunction)
				{
					DoQueueEvent(std::make_unique<EventSystem::FunctionIndirectCall<TCallable, Args...>>(m_EvtHandler, std::forward<TCallable>(callable), std::forward<Args>(arg)...));
				}
				else if constexpr(TCallableTraits::IsFunctor)
				{
					DoQueueEvent(std::make_unique<EventSystem::FunctorIndirectCall<TCallable, Args...>>(m_EvtHandler, std::forward<TCallable>(callable), std::forward<Args>(arg)...));
				}
				else
				{
					static_assert(false, "Unsupported callable type or the type is not callable");
				}
			}
	};
}
