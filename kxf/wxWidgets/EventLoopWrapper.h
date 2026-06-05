#pragma once
#include "MapEventSystem.h"
#include "kxf/Application/IEventLoop.h"
#include "kxf/Core/OptionalPtr.h"
#include "kxf/RTTI/RTTI.h"

#include <wx/evtloop.h>
#include "kxf/Win32/UndefMacros.h"

namespace kxf::wxWidgets
{
	class EventLoopBase: public wxEventLoopBase
	{
		public:
			using wxEventLoopBase::wxEventLoopBase;
			using wxEventLoopBase::OnExit;
			using wxEventLoopBase::DoRun;
			using wxEventLoopBase::DoYieldFor;
	};
	
	class EventLoopManual: public wxEventLoopManual
	{
		public:
			using wxEventLoopManual::wxEventLoopManual;
			using wxEventLoopManual::OnNextIteration;
	};
}

namespace kxf::wxWidgets
{
	class EventLoopWrapper: public RTTI::Implementation<EventLoopWrapper, IEventLoop>
	{
		private:
			optional_ptr<EventLoopBase> m_EventLoop;
			EventLoopManual* m_ManualLoop = nullptr;

		private:
			void OnAssign()
			{
				m_ManualLoop = dynamic_cast<EventLoopManual*>(m_EventLoop.get());
			}

		protected:
			void OnEnter() override
			{
				// There is no 'OnEnter' for 'wxEventLoopBase'
			}
			void OnExit() override
			{
				m_EventLoop->OnExit();
			}
			int OnRun() override
			{
				return m_EventLoop->DoRun();
			}

			void OnYieldFor(FlagSet<EventCategory> toProcess) override
			{
				m_EventLoop->DoYieldFor(wxWidgets::MapEventCategory(toProcess).ToInt());
			}
			void OnNextIteration() override
			{
				if (m_ManualLoop)
				{
					m_ManualLoop->OnNextIteration();
				}
			}

		public:
			EventLoopWrapper(EventLoopBase& eventLoop)
				:m_EventLoop(eventLoop)
			{
			}
			EventLoopWrapper(std::unique_ptr<EventLoopBase> eventLoop)
				:m_EventLoop(std::move(eventLoop))
			{
			}
			EventLoopWrapper(EventLoopWrapper&& other) noexcept
			{
				*this = std::move(other);
			}
			EventLoopWrapper(const EventLoopWrapper&) = delete;

		public:
			int Run() override
			{
				return m_EventLoop->Run();
			}
			void Exit(int exitCode = 0) override
			{
				m_EventLoop->Exit(exitCode);
			}
			void ScheduleExit(int exitCode = 0) override
			{
				m_EventLoop->ScheduleExit(exitCode);
			}

			void WakeUp() override
			{
				m_EventLoop->WakeUp();
			}
			bool Pending() override
			{
				return m_EventLoop->Pending();
			}
			bool Dispatch() override
			{
				return m_EventLoop->Dispatch();
			}
			DispatchTimeout Dispatch(TimeSpan timeout) override
			{
				switch (m_EventLoop->DispatchTimeout(timeout.GetMilliseconds()))
				{
					case -1:
					{
						return DispatchTimeout::Expired;
					}
					case 0:
					{
						return DispatchTimeout::ShouldExit;
					}
					case 1:
					{
						return DispatchTimeout::Success;
					}
				};
				return DispatchTimeout::ShouldExit;
			}
			bool DispatchIdle() override
			{
				return m_EventLoop->ProcessIdle();
			}

			bool IsYielding() const override
			{
				return m_EventLoop->IsYielding();
			}
			bool IsEventAllowedInsideYield(FlagSet<EventCategory> eventCategory) const override
			{
				return m_EventLoop->IsEventAllowedInsideYield(eventCategory.ToInt<wxEventCategory>());
			}

			bool Yield(FlagSet<EventYieldFlag> flags) override
			{
				return m_EventLoop->Yield(flags.Contains(EventYieldFlag::OnlyIfRequired));
			}
			bool YieldFor(FlagSet<EventCategory> toProcess) override
			{
				return m_EventLoop->YieldFor(wxWidgets::MapEventCategory(toProcess).ToInt());
			}

		public:
			EventLoopWrapper& operator=(EventLoopWrapper&& other) noexcept
			{
				m_ManualLoop = std::exchange(other.m_ManualLoop, nullptr);
				m_EventLoop = std::move(other.m_EventLoop);

				return *this;
			}
			EventLoopWrapper& operator=(const EventLoopWrapper&) = delete;
	};
}

namespace kxf::wxWidgets
{
	class EventLoopWrapperWx: public wxEventLoopBase
	{
		private:
			optional_ptr<IEventLoop> m_EventLoop;

		private:
			void Create(FlagSet<EventCategory> allowedToYield)
			{
				m_eventsToProcessInsideYield = wxWidgets::MapEventCategory(allowedToYield).ToInt();
			}

		protected:
			int DoRun() override
			{
				return m_EventLoop->OnRun();
			}
			void DoYieldFor(long eventsToProcess) override
			{
				return m_EventLoop->OnYieldFor(static_cast<EventCategory>(eventsToProcess));
			}
			void OnExit() override
			{
				m_EventLoop->OnExit();
			}

		public:
			EventLoopWrapperWx(IEventLoop& eventLoop, FlagSet<EventCategory> allowedToYield = EventCategory::Everything)
				:m_EventLoop(eventLoop)
			{
				Create(allowedToYield);
			}
			EventLoopWrapperWx(std::unique_ptr<IEventLoop> eventLoop, FlagSet<EventCategory> allowedToYield = EventCategory::Everything)
				:m_EventLoop(std::move(eventLoop))
			{
				Create(allowedToYield);
			}
			EventLoopWrapperWx(EventLoopWrapperWx&&) noexcept = default;
			EventLoopWrapperWx(const EventLoopWrapperWx&) = delete;

		public:
			void Update(FlagSet<EventCategory> allowedToYield, bool shouldExit, size_t yieldLevel, bool isInsideRun)
			{
				m_shouldExit = shouldExit;
				m_yieldLevel = yieldLevel;
				m_eventsToProcessInsideYield = wxWidgets::MapEventCategory(allowedToYield).ToInt();
				//m_isInsideRun = isInsideRun; // It's private unfortunately
			}

			// wxEventLoopBase
			int Run() override
			{
				return m_EventLoop->Run();
			}
			void Exit(int exitCode = 0) override
			{
				m_EventLoop->Exit(exitCode);
			}
			void DoStop(int exitCode = 0) override
			{
				m_EventLoop->ScheduleExit(exitCode);
			}

			void WakeUp() override
			{
				m_EventLoop->WakeUp();
			}
			bool Pending() const override
			{
				return m_EventLoop->Pending();
			}
			bool Dispatch() override
			{
				return m_EventLoop->Dispatch();
			}
			int DispatchTimeout(unsigned long timeout) override
			{
				switch (m_EventLoop->Dispatch(TimeSpan::Milliseconds(timeout)))
				{
					case IEventLoop::DispatchTimeout::Success:
					{
						return 1;
					}
					case IEventLoop::DispatchTimeout::Expired:
					{
						return -1;
					}
					case IEventLoop::DispatchTimeout::ShouldExit:
					{
						return 0;
					}
				};
				return 0;
			}
			bool ProcessIdle() override
			{
				return m_EventLoop->DispatchIdle();
			}

			bool YieldFor(long eventsToProcess) override
			{
				return m_EventLoop->YieldFor(wxWidgets::MapEventCategory(FromInt<wxEventCategory>(eventsToProcess)));
			}
			bool IsYielding() const override
			{
				return m_EventLoop->IsYielding();
			}
			bool IsEventAllowedInsideYield(wxEventCategory category) const override
			{
				return m_EventLoop->IsEventAllowedInsideYield(wxWidgets::MapEventCategory(category));
			}

		public:
			EventLoopWrapperWx& operator=(EventLoopWrapperWx&&) noexcept = default;
			EventLoopWrapperWx& operator=(const EventLoopWrapperWx&) = delete;
	};
}
