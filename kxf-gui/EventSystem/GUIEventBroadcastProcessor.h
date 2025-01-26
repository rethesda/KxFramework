#pragma once
#include "kxf/EventSystem/EventBroadcastProcessor.h"

namespace kxf
{
	class KXF_API GUIEventBroadcastProcessor final: public EventBroadcastProcessor
	{
		public:
			using EventBroadcastProcessor::EventBroadcastProcessor;
	};
}

namespace kxf
{
	class KXF_API GUIEventBroadcastReceiver final: public EventBroadcastReceiver
	{
		private:
			void StopPropagation(IEvent& event);

		protected:
			bool PreProcessEvent(IEvent& event) override;
			void PostProcessEvent(IEvent& event) override;

		public:
			using EventBroadcastReceiver::EventBroadcastReceiver;
	};
}
