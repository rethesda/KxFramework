#include "stdafx.h"
#include "CoreApplication.h"
#include "kxf/EventSystem/IEventExecutor.h"
#include "kxf/EventSystem/IdleEvent.h"
#include "kxf/System/DynamicLibrary.h"
#include "kxf/Utility/Container.h"
#include "kxf/Utility/CallAtScopeExit.h"
#include "kxf/wxWidgets/Application.h"
#include "Private/NativeApp.h"

namespace
{
	template<class TContainer>
	bool IsWindowInContainer(const TContainer& container, wxWindow& window)
	{
		return kxf::Utility::Contains(container, [&](const auto& item)
		{
			return item.get() == &window;
		});
	};
}

namespace kxf
{
	CoreApplication::CoreApplication()
		:m_NativeApp(std::make_unique<Private::NativeApp>(*this))
	{
	}
	CoreApplication::~CoreApplication() = default;

	// IObject
	void* CoreApplication::QueryInterface(const IID& iid) noexcept
	{
		if (iid.IsOfType<wxWidgets::Application>())
		{
			return static_cast<wxWidgets::Application*>(m_NativeApp.get());
		}
		return TBaseClass::QueryInterface(iid);
	}

	// ICoreApplication
	bool CoreApplication::OnCreate()
	{
		return true;
	}
	void CoreApplication::OnDestroy()
	{
		m_MainLoop = nullptr;

		// Clean up any still pending objects. Normally there shouldn't any as we
		// already do this in 'OnExit', but this could happen if the user code has
		// somehow managed to create more of them since then or just forgot to call
		// the base class 'OnExit'.
		FinalizeScheduledForDestruction();
	}

	void CoreApplication::OnExit()
	{
		// Finalize scheduled objects if there are anything left
		FinalizeScheduledForDestruction();
	}
	int CoreApplication::OnRun()
	{
		if (auto mainLoop = CreateMainLoop())
		{
			// Save the old main loop pointer (if any), create a new loop and run it.
			// At the end of the loop, restore the original main loop.
			std::swap(m_MainLoop, mainLoop);
			Utility::CallAtScopeExit atExit = [&]()
			{
				m_MainLoop = std::move(mainLoop);
			};

			// Here we're running our newly created main loop saved in place of 'm_MainLoop' pointer.
			return m_MainLoop->Run();
		}
		return m_ExitCode.value_or(-1);
	}

	void CoreApplication::Exit(int exitCode)
	{
		if (m_MainLoop)
		{
			ExitMainLoop(exitCode);
		}
		else
		{
			m_ExitCode = exitCode;
			std::terminate();
		}
	}

	void CoreApplication::AddEventFilter(IEventFilter& eventFilter)
	{
		WriteLockGuard lock(m_EventFiltersLock);
		m_EventFilters.push_back(&eventFilter);
	}
	void CoreApplication::RemoveEventFilter(IEventFilter& eventFilter)
	{
		WriteLockGuard lock(m_EventFiltersLock);
		m_EventFilters.remove(&eventFilter);
	}
	IEventFilter::Result CoreApplication::FilterEvent(Event& event)
	{
		using Result = IEventFilter::Result;

		ReadLockGuard lock(m_EventFiltersLock);
		for (IEventFilter* eventFilter: m_EventFilters)
		{
			const Result result = eventFilter->FilterEvent(event);
			if (result != Result::Skip)
			{
				return result;
			}
		}

		// Do nothing by default if there are no event filters
		return Result::Skip;
	}

	// Application::IBasicInfo
	String CoreApplication::GetName() const
	{
		if (!m_Name.IsEmpty())
		{
			return m_Name;
		}
		return DynamicLibrary::GetCurrentModule().GetFilePath().GetName();
	}
	String CoreApplication::GetDisplayName() const
	{
		if (!m_DisplayName.IsEmpty())
		{
			return m_DisplayName;
		}
		else if (!m_Name.IsEmpty())
		{
			return m_Name;
		}
		return GetName().MakeCapitalized();
	}

	String CoreApplication::GetClassName() const
	{
		return m_NativeApp->GetClassName();
	}
	void CoreApplication::SetClassName(const String& name)
	{
		m_NativeApp->SetClassName(name);
	}

	// Application::IMainEoventLoop
	void CoreApplication::ExitMainLoop(int exitCode)
	{
		// We should exit from the main event loop, not just any currently active (e.g. modal dialog) event loop
		m_ExitCode = exitCode;
		if (m_MainLoop && m_MainLoop->IsRunning())
		{
			m_MainLoop->Exit(exitCode);
		}
	}

	// Application::IActiveEventLoop
	IEventLoop* CoreApplication::GetActiveEventLoop()
	{
		return m_ActiveEventLoop;
	}
	void CoreApplication::SetActiveEventLoop(IEventLoop* eventLoop)
	{
		m_ActiveEventLoop = eventLoop;
		if (eventLoop)
		{
			IActiveEventLoop::CallOnEnterEventLoop(*eventLoop);
		}
	}

	void CoreApplication::WakeUp()
	{
		if (IEventLoop* eventLoop = GetActiveEventLoop())
		{
			eventLoop->WakeUp();
		}
	}
	bool CoreApplication::Pending()
	{
		if (IEventLoop* eventLoop = GetActiveEventLoop())
		{
			return eventLoop->Pending();
		}
		return false;
	}
	bool CoreApplication::Dispatch()
	{
		if (IEventLoop* eventLoop = GetActiveEventLoop())
		{
			return eventLoop->Dispatch();
		}
		return false;
	}
	bool CoreApplication::DispatchIdle()
	{
		// Synthesize an idle event and check if more of them are needed
		IdleEvent& event = BuildProcessEvent(IdleEvent::EvtIdle)
			.SetSourceToSelf()
			.Execute()
			.GetEvent();

		// Flush the logged messages if any (do this after processing the events which could have logged new messages)
		wxLog::FlushActive();

		// Garbage collect all objects previously scheduled for destruction
		FinalizeScheduledForDestruction();

		return event.IsMoreRequested();
	}
	bool CoreApplication::Yield(FlagSet<EventYieldFlag> flags)
	{
		if (IEventLoop* activeEventLoop = GetActiveEventLoop())
		{
			return activeEventLoop->Yield(flags);
		}
		else if (auto tempEventLoop = CreateMainLoop())
		{
			return tempEventLoop->Yield(flags);
		}
		return false;
	}

	// Application::IPendingEvents
	bool CoreApplication::IsScheduledForDestruction(const wxObject& object) const
	{
		ReadLockGuard lock(m_ScheduledForDestructionLock);

		return Utility::Contains(m_ScheduledForDestruction, [&](const auto& item)
		{
			return item.get() == &object;
		});
	}
	void CoreApplication::ScheduleForDestruction(std::unique_ptr<wxObject> object)
	{
		// We need an active event loop to schedule deletion. If no active loop present
		// the object is deleted immediately.
		// TODO: Should we still add it and delete when the application object gets destroyed?
		if (m_ActiveEventLoop && object)
		{
			WriteLockGuard lock(m_ScheduledForDestructionLock);

			if (!Utility::Contains(m_ScheduledForDestruction, [&](const auto& item)
			{
				return item.get() == object.get();
			}))
			{
				m_ScheduledForDestruction.emplace_back(std::move(object));
			}
		}
	}
	void CoreApplication::FinalizeScheduledForDestruction()
	{
		WriteLockGuard lock(m_ScheduledForDestructionLock);
		m_ScheduledForDestruction.clear();
	}

	void CoreApplication::AddPendingEventHandler(EvtHandler& evtHandler)
	{
		WriteLockGuard lock(m_PendingEvtHandlersLock);

		if (!Utility::Contains(m_PendingEvtHandlers, [&](const EvtHandler* item)
		{
			return item == &evtHandler;
		}))
		{
			m_PendingEvtHandlers.emplace_back(&evtHandler);
		}
	}
	bool CoreApplication::RemovePendingEventHandler(EvtHandler& evtHandler)
	{
		size_t count = 0;
		auto Find = [&](const EvtHandler* item)
		{
			if (item == &evtHandler)
			{
				count++;
				return true;
			}
			return false;
		};

		// Try to remove the handler from both lists 
		if (WriteLockGuard lock(m_PendingEvtHandlersLock); true)
		{
			Utility::RemoveSingleIf(m_PendingEvtHandlers, Find);
			Utility::RemoveSingleIf(m_DelayedPendingEvtHandlers, Find);
		}
		return count != 0;
	}
	void CoreApplication::DelayPendingEventHandler(EvtHandler& evtHandler)
	{
		// Move the handler from the list of handlers with processable pending events
		// to the list of handlers with pending events which needs to be processed later.
		auto Find = [&](const EvtHandler* item)
		{
			return item == &evtHandler;
		};

		if (WriteLockGuard lock(m_PendingEvtHandlersLock); true)
		{
			Utility::RemoveSingleIf(m_PendingEvtHandlers, Find);
			if (!Utility::Contains(m_DelayedPendingEvtHandlers, Find))
			{
				m_DelayedPendingEvtHandlers.emplace_back(&evtHandler);
			}
		}
	}

	bool CoreApplication::ProcessPendingEvents()
	{
		if (m_PendingEventsProcessingEnabled)
		{
			WriteLockGuard lock(m_PendingEvtHandlersLock);

			// This helper list should be empty
			if (!m_DelayedPendingEvtHandlers.empty())
			{
				return false;
			}

			// Iterate until the list becomes empty: the handlers remove themselves from it when they don't have any more pending events
			size_t count = 0;
			while (!m_PendingEvtHandlers.empty())
			{
				// In 'EvtHandler::ProcessPendingEvents', new handlers might be added and we are required to unlock the lock here.
				EvtHandler* evtHandler = m_PendingEvtHandlers.front();
				
				// This inner unlock-relock must be consistent with the outer guard.
				m_PendingEvtHandlersLock.UnlockWrite();
				Utility::CallAtScopeExit relock = [&]()
				{
					m_PendingEvtHandlersLock.LockWrite();
				};

				// We always call 'EvtHandler::ProcessPendingEvents' on the first event handler with pending events because handlers
				// auto-remove themselves from this list (see 'RemovePendingEventHandler') if they have no more pending events.
				if (evtHandler->ProcessPendingEvents())
				{
					count++;
				}
			}

			// Now the 'm_PendingEvtHandlers' is surely empty, however some event handlers may have moved themselves into
			// 'm_DelayedPendingEvtHandlers' because of a selective 'Yield' call in progress. Now we need to move them back
			// to 'm_PendingEvtHandlersLock' so the next call to this function has the chance of processing them.
			if (!m_DelayedPendingEvtHandlers.empty())
			{
				m_PendingEvtHandlers.insert(m_PendingEvtHandlers.end(), m_DelayedPendingEvtHandlers.begin(), m_DelayedPendingEvtHandlers.end());
				m_DelayedPendingEvtHandlers.clear();
			}
			return count != 0;
		}
		return false;
	}
	size_t CoreApplication::DiscardPendingEvents()
	{
		WriteLockGuard lock(m_PendingEvtHandlersLock);

		size_t count = 0;
		for (EvtHandler* evtHandler: m_PendingEvtHandlers)
		{
			if (evtHandler->DiscardPendingEvents() != 0)
			{
				count++;
			}
		}
		m_PendingEvtHandlers.clear();

		return count;
	}

	// Application::IExceptionHandler
	bool CoreApplication::OnMainLoopException()
	{
		throw;
	}
	void CoreApplication::OnFatalException()
	{
		// Nothing to do
	}
	void CoreApplication::OnUnhandledException()
	{
		// We're called from an exception handler so we can re-throw the exception  to recover its type
		String what;
		try
		{
			throw;
		}
		catch (std::exception& e)
		{
			what = String::Format(wxS("standard exception of type \"%s\" with message \"%s\""), typeid(e).name(), e.what());
		}
		catch (...)
		{
			what = wxS("unknown exception");
		}
		wxMessageOutputBest().Printf(wxS("Unhandled %s; terminating %s.\n"), what.wx_str(), wxIsMainThread() ? wxS("the application") : wxS("the thread in which it happened"));
	}

	bool CoreApplication::StoreCurrentException()
	{
		if (!m_StoredException)
		{
			m_StoredException = std::current_exception();
			return true;
		}
		else
		{
			// We can't store more than one exception currently: while we could  support this by just
			// using a 'std::vector<std::exception_ptr>', it shouldn't be actually necessary because
			// we should never have more than one active exception anyhow.
			return false;
		}
	}
	void CoreApplication::RethrowStoredException()
	{
		if (std::exception_ptr storedException = std::move(m_StoredException))
		{
			std::rethrow_exception(storedException);
		}
	}

	// Application::IDebugHandler
	void CoreApplication::OnAssertFailure(String file, int line, String function, String condition, String message)
	{
		if (wxLog* log = wxLog::GetActiveTarget(); log && log->IsEnabled() && log->IsLevelEnabled(wxLOG_Debug, wxLOG_COMPONENT))
		{
			log->LogTextAtLevel(wxLOG_Debug, String::Format(wxS("File '%1'@%2; Function '%3'; When [%4]; %5"), file, line, function, condition, message));
		}
	}

	// Application::ICommandLine
	size_t CoreApplication::EnumCommandLineArgs(std::function<bool(String)> func) const
	{
		auto DoEnum = [&](auto&& argv, size_t argc)
		{
			size_t count = 0;
			for (size_t i = 0; i < argc; i++)
			{
				count++;
				if (!std::invoke(func, argv[i]))
				{
					break;
				}
			}
			return count;
		};

		if (m_ArgVW)
		{
			return DoEnum(m_ArgVW, m_ArgC);
		}
		else if (m_ArgVA)
		{
			return DoEnum(m_ArgVA, m_ArgC);
		}
		return 0;
	}
	void CoreApplication::OnCommandLineInit(wxCmdLineParser& parser)
	{
		// Nothing to do
	}
	bool CoreApplication::OnCommandLineParsed(wxCmdLineParser& parser)
	{
		return true;
	}
	bool CoreApplication::OnCommandLineError(wxCmdLineParser& parser)
	{
		parser.Usage();
		return false;
	}
	bool CoreApplication::OnCommandLineHelp(wxCmdLineParser& parser)
	{
		parser.Usage();
		return false;
	}
}