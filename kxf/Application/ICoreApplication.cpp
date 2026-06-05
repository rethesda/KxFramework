#include "kxf-pch.h"
#include "ICoreApplication.h"

#include "kxf/wxWidgets/Common.h"
#include <wx/module.h>

namespace
{
	std::atomic<kxf::ICoreApplication*> g_AppInstance = nullptr;
}

namespace kxf
{
	ICoreApplication* ICoreApplication::GetInstance() noexcept
	{
		return g_AppInstance;
	}
	void ICoreApplication::SetInstance(ICoreApplication* instance) noexcept
	{
		g_AppInstance = instance;
	}

	void ICoreApplication::RegisterModules()
	{
		wxModule::RegisterModules();
	}
	bool ICoreApplication::InitializeModules()
	{
		return wxModule::InitializeModules();
	}

	// ICoreApplication -> Active Event Loop
	void ICoreApplication::CallOnEnterEventLoop(IEventLoop& eventLoop)
	{
		eventLoop.OnEnter();
	}
	void ICoreApplication::CallOnExitEventLoop(IEventLoop& eventLoop)
	{
		eventLoop.OnExit();
	}
}
