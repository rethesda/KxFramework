#include "kxf-pch.h"
#include "WidgetFactory.h"
#include "Widget.h"
#include "Host.h"
#include "Element.h"
#include "Stylesheets/MasterStylesheetStorage.h"

namespace
{
	std::vector<kxf::optional_ptr<kxf::Sciter::WidgetFactory>> g_RegisteredFactories;
}

namespace kxf::Sciter
{
	std::unique_ptr<Widget> WidgetFactory::NewWidget(Host& host, const Element& element, const String& fullyQualifiedClassName)
	{
		std::unique_ptr<Widget> result;
		EnumFactories([&](WidgetFactory& factory)
		{
			if (factory.GetFullyQualifiedClassName() == fullyQualifiedClassName)
			{
				result = factory.CreateWidget(host, element);
				return CallbackCommand::Terminate;
			}
			return CallbackCommand::Continue;
		});

		return result;
	}

	CallbackResult<void> WidgetFactory::EnumFactories(CallbackFunction<WidgetFactory&> func)
	{
		for (auto& factory: g_RegisteredFactories)
		{
			if (func.Invoke(*factory).ShouldTerminate())
			{
				break;
			}
		}
		return func.Finalize();
	}
	void WidgetFactory::RegisterFactory(WidgetFactory& factory)
	{
		g_RegisteredFactories.emplace_back(factory);
	}
	void WidgetFactory::RegisterFactory(std::unique_ptr<WidgetFactory> factory)
	{
		g_RegisteredFactories.emplace_back(std::move(factory));
	}
	StylesheetStorage& WidgetFactory::GetStylesheetStorage()
	{
		return MasterStylesheetStorage::GetInstance();
	}
}
