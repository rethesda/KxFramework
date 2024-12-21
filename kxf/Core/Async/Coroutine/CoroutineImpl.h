#pragma once
#include "../Common.h"
#include "kxf/RTTI/RTTI.h"
#include "kxf/DateTime/SimpleTimer.h"
#include "kxf/EventSystem/IndirectInvocationEvent.h"
#include "YieldInstruction.h"
#include <utility>
#include <optional>
#include <type_traits>

namespace kxf
{
	class Coroutine;

	class ICoroutine: public RTTI::Interface<ICoroutine>
	{
		kxf_RTTI_DeclareIID(ICoroutine, {0x948e00f7, 0xc620, 0x4c7d, { 0xb6, 0x95, 0x96, 0x67, 0x71, 0xd5, 0xd9, 0x83}});

		protected:
			virtual Async::YieldInstruction Execute() = 0;

		public:
			virtual void Terminate() = 0;

			virtual TimeSpan GetTimeDelta() const = 0;
			virtual TimeSpan GetElapsedTime() const = 0;
	};
}

namespace kxf::Async
{
	class CoroutineBase;

	class KXF_API CoroutineTimer final: public SimpleTimer
	{
		private:
			std::shared_ptr<CoroutineBase> m_Coroutine;

		protected:
			void OnNotify() override;

		public:
			void Wait(std::shared_ptr<CoroutineBase> coroutine, const TimeSpan& time);
			std::shared_ptr<CoroutineBase> Relinquish();
	};

	class KXF_API CoroutineExecutor final: public EventSystem::IndirectInvocationEvent
	{
		private:
			std::shared_ptr<CoroutineBase> m_Coroutine;

		public:
			CoroutineExecutor(std::shared_ptr<CoroutineBase> coroutine);

		public:
			// IEvent
			std::unique_ptr<IEvent> Move() noexcept override
			{
				return std::make_unique<CoroutineExecutor>(std::move(*this));
			}

			// IIndirectInvocationEvent
			void Execute() override;
	};
}

namespace kxf::Async
{
	class KXF_API CoroutineBase: RTTI::Implementation<CoroutineBase, ICoroutine>
	{
		friend class CoroutineTimer;
		friend class CoroutineExecutor;

		public:
			static CoroutineBase* Run(std::shared_ptr<CoroutineBase> coroutine);

			template<class T = intptr_t>
			static YieldInstruction Yield(const T& nextState = 0)
			{
				return YieldInstruction(InstructionType::Continue, nextState);
			}
			
			template<class T = intptr_t>
			static YieldInstruction YieldWait(const TimeSpan& interval, const T& nextState = 0)
			{
				YieldInstruction instruction(InstructionType::Delay, nextState);
				instruction.m_Delay = interval;
				return instruction;
			}
			
			template<class T = intptr_t>
			static YieldInstruction YieldStop(const T& nextState = 0)
			{
				return YieldInstruction(InstructionType::Terminate, nextState);
			}

		private:
			static void QueueExecution(std::shared_ptr<CoroutineBase> coroutine);
			static void DelayExecution(std::shared_ptr<CoroutineBase> coroutine, const TimeSpan& time);
			static void AbortExecution(std::shared_ptr<CoroutineBase> coroutine);

		private:
			CoroutineTimer m_DelayTimer;
			YieldInstruction m_Instruction;
			TimeSpan m_TimeStampStart;
			TimeSpan m_TimeStampBefore;
			TimeSpan m_TimeStampAfter;

		private:
			void BeforeExecute();
			void AfterExecute();
			void RunExecute(std::shared_ptr<CoroutineBase> coroutine);

			TimeSpan GetCurrentExecutionTime() const;

		public:
			CoroutineBase();
			virtual ~CoroutineBase();

		public:
			// ICoroutine
			void Terminate() override;

			TimeSpan GetTimeDelta() const override;
			TimeSpan GetElapsedTime() const override;

			template<class T = intptr_t>
			std::optional<T> GetNextState() const noexcept
			{
				return m_Instruction.GetNextState<T>();
			}
	};
}
