#pragma once
#include "IEvtHandler.h"
#include "EvtHandler.h"
#include "EventHandlerStack.h"
#include "EvtHandlerDelegate.h"

namespace kxf
{
	class EventBroadcastProcessor;
	class EventBroadcastReceiver;
}

namespace kxf::EventSystem
{
	class KXF_API BroadcastProcessorHandler final: public EvtHandler
	{
		private:
			EventBroadcastProcessor& m_Processor;

		protected:
			bool TryBefore(IEvent& event) override;

		public:
			BroadcastProcessorHandler(EventBroadcastProcessor& processor)
				:m_Processor(processor)
			{
			}
	};

	class KXF_API BroadcastReceiverHandler final: public EvtHandler
	{
		friend class EventBroadcastReceiver;

		protected:
			bool TryBefore(IEvent& event) override;
	};
}

namespace kxf
{
	class KXF_API EventBroadcastProcessor: public EvtHandlerDelegate
	{
		friend class EventBroadcastReceiver;

		public:
			enum class Order
			{
				Default = -1,
				LastToFirst,
				FirstToLast
			};

		private:
			EventSystem::BroadcastProcessorHandler m_EvtHandler;
			EvtHandlerStack m_Stack;
			EvtHandlerStack::Order m_Order;

		protected:
			virtual bool PreProcessEvent(IEvent& event)
			{
				return true;
			}
			virtual void PostProcessEvent(IEvent& event)
			{
			}

		public:
			EventBroadcastProcessor();
			virtual ~EventBroadcastProcessor() = default;

		public:
			bool AddReceiver(EventBroadcastReceiver& reciever);
			bool RemoveReceiver(EventBroadcastReceiver& reciever);

			bool HasReceivers() const
			{
				return m_Stack.HasChainedItems();
			}
			size_t GetReceiversCount() const
			{
				return m_Stack.GetCount();
			}

			CallbackResult<void> EnumReceivers(CallbackFunction<IEvtHandler&> func, Order order = Order::Default) const;

			Order GetReceiversOrder() const;
			void SetReceiversOrder(Order order);
	};
}

namespace kxf
{
	class KXF_API EventBroadcastReceiver: public EvtHandlerDelegate
	{
		friend class EventBroadcastProcessor;

		private:
			EventSystem::BroadcastReceiverHandler m_EvtHandler;
			EventBroadcastProcessor& m_Processor;

		protected:
			virtual bool PreProcessEvent(IEvent& event);
			virtual void PostProcessEvent(IEvent& event);

		protected:
			IEvtHandler& GetEvtHandler()
			{
				return m_EvtHandler;
			}

		public:
			EventBroadcastReceiver(EventBroadcastProcessor& processor)
				:EvtHandlerDelegate(m_EvtHandler), m_Processor(processor)
			{
				m_Processor.AddReceiver(*this);
			}
			virtual ~EventBroadcastReceiver()
			{
				m_Processor.RemoveReceiver(*this);
			}

		public:
			EventBroadcastProcessor& GetProcessor()
			{
				return m_Processor;
			}
			const EventBroadcastProcessor& GetProcessor() const
			{
				return m_Processor;
			}
	};
}
