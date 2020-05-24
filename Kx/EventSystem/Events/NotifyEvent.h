#pragma once
#include "Kx/EventSystem/Event.h"

namespace kxf
{
	class NotifyEvent: public wxNotifyEvent
	{
		public:
			NotifyEvent() = default;

		public:
			NotifyEvent* Clone() const override
			{
				return new NotifyEvent(*this);
			}
	};
}
