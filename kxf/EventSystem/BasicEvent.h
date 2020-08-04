#pragma once
#include "Common.h"
#include "IEvent.h"
#include "kxf/Utility/Common.h"
#include <condition_variable>

namespace kxf::EventSystem
{
	enum class EventPublicState: uint32_t
	{
		None = 0,

		Skipped = 1 << 0,
		Allowed = 1 << 1
	};
	enum class EventPrivateState: uint32_t
	{
		None = 0,

		// Set after a call to 'OnStartProcess'.
		Started = 1 << 0,

		// Indicates that the event is executed asynchronously. On other words it was sent from 'IEvtHandler::Queue[Unique][Event|Signal]'.
		Async = 1 << 1,

		// Set as soon as 'WasQueued' is called for the first time when event is going to be queued from 'DoProcessEvent'.
		ReQueued = 1 << 2,

		// Set if the event is invoked with thread-blocking semantics.
		Waitable = 1 << 3,

		// Set after 'DoProcessEvent' was called at least once for this event.
		ProcessedOnce = 1 << 4,

		// Can be set to indicate that the event will be passed to another handler if it's not processed in this one.
		WillBeProcessedAgain = 1 << 5,
	};
}
namespace kxf
{
	KxDeclareFlagSet(EventSystem::EventPublicState);
	KxDeclareFlagSet(EventSystem::EventPrivateState);
}

namespace kxf
{
	class KX_API BasicEvent: public RTTI::ImplementInterface<BasicEvent, IEvent>, private IEventInternal
	{
		private:
			using EventPublicState = EventSystem::EventPublicState;
			using EventPrivateState = EventSystem::EventPrivateState;

		private:
			EventID m_EventID;
			IEvtHandler* m_EventSource = nullptr;
			UniversallyUniqueID m_UniqueID;
			TimeSpan m_Timestamp;

			FlagSet<EventPublicState> m_PublicState = EventPublicState::Allowed;
			mutable FlagSet<EventPrivateState> m_PrivateState;
			FlagSet<ProcessEventFlag> m_ProcessFlags;

			struct WaitInfo
			{
				std::mutex Mutex;
				std::condition_variable Condition;
				std::atomic<bool> Flag = false;
				std::unique_ptr<IEvent> Self;
			};
			std::unique_ptr<WaitInfo> m_WaitInfo;
			std::unique_ptr<IEvent> m_WaitResult;

		private:
			bool TestAndSetPrivateState(EventPrivateState flag) const
			{
				if (!m_PrivateState.Contains(flag))
				{
					m_PrivateState.Add(flag);
					return false;
				}
				return true;
			}

			// IEventInternal
			bool IsAsync() const override
			{
				return m_PrivateState.Contains(EventPrivateState::Async);
			}
			bool WasReQueued() const override
			{
				return TestAndSetPrivateState(EventPrivateState::ReQueued);
			}
			bool WasProcessed() const override
			{
				return TestAndSetPrivateState(EventPrivateState::ProcessedOnce);
			}
			bool WillBeProcessedAgain() const override
			{
				return TestAndSetPrivateState(EventPrivateState::WillBeProcessedAgain);
			}

			void OnStartProcess(const EventID& eventID, const UniversallyUniqueID& uuid, FlagSet<ProcessEventFlag> flags, bool isAsync) override
			{
				if (!m_PrivateState.Contains(EventPrivateState::Started))
				{
					m_PrivateState.Add(EventPrivateState::Started);
					m_PrivateState.Mod(EventPrivateState::Async, isAsync);

					m_EventID = eventID;
					m_UniqueID = std::move(uuid);
					m_Timestamp = TimeSpan::Now(SteadyClock());
					m_ProcessFlags = flags;
				}
			}
			FlagSet<ProcessEventFlag> GetProcessFlags() const override
			{
				return m_ProcessFlags;
			}

			void SignalProcessed(std::unique_ptr<IEvent> event) override
			{
				if (m_PrivateState.Contains(EventPrivateState::Waitable))
				{
					m_WaitInfo->Self = std::move(event);
					m_WaitInfo->Flag = true;

					m_WaitInfo->Condition.notify_one();
				}
			}
			std::unique_ptr<IEvent> WaitProcessed() override
			{
				if (!m_WaitInfo)
				{
					m_PrivateState.Add(EventPrivateState::Waitable);

					m_WaitInfo = std::make_unique<WaitInfo>();
					m_WaitInfo->Flag = false;

					std::unique_lock lock(m_WaitInfo->Mutex);
					m_WaitInfo->Condition.wait(lock, [&]()
					{
						return m_WaitInfo->Flag == true;
					});

					return std::move(m_WaitInfo->Self);
				}
				return nullptr;
			}

			void PutWaitResult(std::unique_ptr<IEvent> event) override
			{
				m_WaitResult = std::move(event);
			}
			std::unique_ptr<IEvent> GetWaitResult()
			{
				return std::move(m_WaitResult);
			}

		public:
			BasicEvent() = default;
			BasicEvent(const BasicEvent&) noexcept = default;
			BasicEvent(BasicEvent&& other) noexcept
			{
				*this = std::move(other);
			}

		public:
			// IObject
			using IObject::QueryInterface;
			void* QueryInterface(const IID& iid) noexcept override
			{
				if (iid.IsOfType<IEventInternal>())
				{
					return static_cast<IEventInternal*>(this);
				}
				return TBaseClass::QueryInterface(iid);
			}

			// IEvent
			std::unique_ptr<IEvent> Move() noexcept override
			{
				return std::make_unique<BasicEvent>(std::move(*this));
			}

			EventID GetEventID() const override
			{
				return m_EventID;
			}
			TimeSpan GetTimestamp() const override
			{
				return m_Timestamp;
			}
			UniversallyUniqueID GetUniqueID() const override
			{
				return m_UniqueID;
			}
			FlagSet<EventCategory> GetEventCategory() const override
			{
				return EventCategory::None;
			}

			IEvtHandler* GetEventSource() const override
			{
				return m_EventSource;
			}
			void SetEventSource(IEvtHandler* evtHandler) override
			{
				m_EventSource = evtHandler;
			}
			
			bool IsSkipped() const override
			{
				return m_PublicState.Contains(EventPublicState::Skipped);
			}
			void Skip(bool skip = true) override
			{
				m_PublicState.Mod(EventPublicState::Skipped, skip);
			}

			bool IsAllowed() const override
			{
				return m_PublicState.Contains(EventPublicState::Allowed);
			}
			void Allow(bool allow = true) override
			{
				m_PublicState.Mod(EventPublicState::Allowed, allow);
			}

		public:
			BasicEvent& operator=(const BasicEvent&) noexcept = default;
			BasicEvent& operator=(BasicEvent&& other) noexcept
			{
				m_EventID = std::move(other.m_EventID);
				m_EventSource = Utility::ExchangeResetAndReturn(other.m_EventSource, nullptr);
				m_UniqueID = std::move(other.m_UniqueID);
				m_Timestamp = std::move(other.m_Timestamp);

				m_PublicState = Utility::ExchangeResetAndReturn(other.m_PublicState, EventPublicState::Allowed);
				m_PrivateState = Utility::ExchangeResetAndReturn(other.m_PrivateState, EventPrivateState::None);

				m_WaitInfo = std::move(m_WaitInfo);
				m_WaitResult = std::move(m_WaitResult);

				return *this;
			}
	};
}
