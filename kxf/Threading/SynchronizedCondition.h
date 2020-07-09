#pragma once
#include "Common.h"

namespace kxf
{
	class KX_API SynchronizedCondition final
	{
		private:
			void* m_Handle = nullptr;

		public:
			SynchronizedCondition() noexcept;
			SynchronizedCondition(const SynchronizedCondition&) = delete;
			~SynchronizedCondition() noexcept;

		public:
			bool IsNull() const noexcept
			{
				return m_Handle == nullptr;
			}
			void* GetHandle() const noexcept
			{
				return m_Handle;
			}

			bool Signal() noexcept;

		public:
			SynchronizedCondition& operator=(const SynchronizedCondition&) = delete;
	};
}
