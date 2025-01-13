#pragma once
#include "Common.h"
#include "kxf/Utility/Memory.h"

namespace kxf
{
	template<class TValue_, size_t Size_ = sizeof(TValue_), size_t Alignment_ = alignof(TValue_)>
	class UninitializedBuffer final
	{
		public:
			using TValue = typename std::conditional_t<std::is_reference_v<TValue_>, std::reference_wrapper<std::remove_reference_t<TValue_>>, TValue_>;

		private:
			alignas(Alignment_) uint8_t m_Buffer[Size_] = {};

		public:
			UninitializedBuffer() noexcept = default;
			UninitializedBuffer(const UninitializedBuffer&) = delete;
			UninitializedBuffer(UninitializedBuffer&&) = delete;
			~UninitializedBuffer() noexcept = default;

		public:
			void* data() noexcept
			{
				return static_cast<void*>(m_Buffer);
			}
			const void* data() const noexcept
			{
				return static_cast<const void*>(m_Buffer);
			}

			TValue* get() noexcept
			{
				return static_cast<TValue*>(data());
			}
			const TValue* get() const noexcept
			{
				return static_cast<const TValue*>(data());
			}

			size_t size() const noexcept
			{
				return Size_;
			}
			size_t alignment() const noexcept
			{
				return Alignment_;
			}

			template<class... Args>
			requires(Alignment_ != 0 && std::is_constructible_v<TValue, Args...>)
			TValue* Construct(Args&&... arg) noexcept(std::is_nothrow_constructible_v<TValue, Args...>)
			{
				static_assert(Size_ >= sizeof(TValue), "insufficient buffer size");
				static_assert(Alignment_ == alignof(TValue), "alignment doesn't match");

				return Utility::ConstructAt<TValue>(data(), std::forward<Args>(arg)...);
			}

			void Destroy() noexcept(std::is_nothrow_destructible_v<TValue>)
			{
				Utility::DestroyAt<TValue>(data());
			}

			void ZeroBuffer() noexcept
			{
				std::memset(m_Buffer, 0, Size_);
			}
			void CopyBuffer(const UninitializedBuffer& other) noexcept
			{
				std::memcpy(m_Buffer, other.m_Buffer, Size_);
			}
			void MoveBuffer(UninitializedBuffer& other) noexcept
			{
				CopyBuffer(other);
				other.ZeroBuffer();
			}

		public:
			TValue* operator&() noexcept
			{
				return get();
			}
			const TValue* operator&() const noexcept
			{
				return get();
			}

			TValue& operator*() & noexcept
			{
				return *get();
			}
			const TValue& operator*() const& noexcept
			{
				return *get();
			}

			TValue* operator->() noexcept
			{
				return get();
			}
			const TValue* operator->() const noexcept
			{
				return get();
			}

			UninitializedBuffer& operator=(const UninitializedBuffer&) = delete;
			UninitializedBuffer& operator=(UninitializedBuffer&&) = delete;
	};
}

namespace kxf
{
	template<class TValue_, size_t Size_ = sizeof(TValue_), size_t Alignment_ = alignof(TValue_)>
	class UninitializedStorage final
	{
		public:
			using TValue = TValue_;

		private:
			UninitializedBuffer<TValue_, Size_, Alignment_> m_Buffer;
			TValue* m_Value = nullptr;

		public:
			UninitializedStorage() noexcept = default;
			UninitializedStorage(const UninitializedStorage&) = delete;
			UninitializedStorage(UninitializedStorage&&) = delete;
			~UninitializedStorage() noexcept = default;

		public:
			template<class... Args>
			requires(Alignment_ != 0 && std::is_constructible_v<TValue, Args...>)
			TValue* Construct(Args&&... arg) noexcept(std::is_nothrow_constructible_v<TValue, Args...>)
			{
				if (!m_Value)
				{
					m_Value = m_Buffer.Construct(std::forward<Args>(arg)...);
				}
				return m_Value;
			}

			template<class... Args>
			requires(Alignment_ == 0 && std::is_constructible_v<TValue, Args...>)
			TValue* ConstructAligned(Args&&... arg) noexcept(std::is_nothrow_constructible_v<TValue, Args...>)
			{
				if (!m_Value)
				{
					static_assert(Size_ >= sizeof(TValue), "insufficient buffer size");

					m_Value = Utility::ConstructAtAlignedWith<TValue>(m_Buffer.data(), m_Buffer.size(), std::forward<Args>(arg)...);
				}
				return m_Value;
			}

			template<class... Args>
			requires(Alignment_ == 0 && std::is_constructible_v<TValue, Args...>)
			TValue* ConstructAlignedTo(size_t alignment, Args&&... arg) noexcept(std::is_nothrow_constructible_v<TValue, Args...>)
			{
				if (!m_Value)
				{
					static_assert(Size_ >= sizeof(TValue), "insufficient buffer size");

					m_Value = Utility::ConstructAtAlignedTo<TValue>(m_Buffer.data(), m_Buffer.size(), alignment, std::forward<Args>(arg)...);
				}
				return m_Value;
			}

			bool IsConstructed() const noexcept
			{
				return m_Value != nullptr;
			}
			void Destroy() noexcept(std::is_nothrow_destructible_v<TValue>)
			{
				if (m_Value)
				{
					Utility::DestroyAt(m_Value);
					m_Value = nullptr;
				}
			}

			void CopyFrom(const UninitializedStorage& other) noexcept(std::is_nothrow_copy_constructible_v<TValue>)
			{
				if (other.IsConstructed())
				{
					Destroy();
					Construct(*other.m_Value);
				}
				else if (IsConstructed())
				{
					Destroy();
				}
			}
			void MoveFrom(UninitializedStorage&& other) noexcept(std::is_nothrow_move_constructible_v<TValue>)
			{
				if (other.IsConstructed())
				{
					Destroy();
					Construct(std::move(*other.m_Value));
				}
				else if (IsConstructed())
				{
					Destroy();
				}
			}

			auto& GetBuffer() noexcept
			{
				return m_Buffer;
			}
			const auto& GetBuffer() const noexcept
			{
				return m_Buffer;
			}

			size_t size() const noexcept
			{
				return m_Buffer.size();
			}
			size_t alignment() const noexcept
			{
				return m_Buffer.alignment();
			}

			TValue* get() noexcept
			{
				return m_Value;
			}
			const TValue* get() const noexcept
			{
				return m_Value;
			}
			void set(TValue* ptr) noexcept
			{
				if (!m_Value)
				{
					m_Value = ptr;
				}
			}

		public:
			explicit operator bool() const noexcept
			{
				return IsConstructed();
			}
			bool operator!() const noexcept
			{
				return !IsConstructed();
			}

			TValue* operator&() noexcept
			{
				return get();
			}
			const TValue* operator&() const noexcept
			{
				return get();
			}

			TValue& operator*() noexcept
			{
				return *get();
			}
			const TValue& operator*() const noexcept
			{
				return *get();
			}

			TValue* operator->() noexcept
			{
				return get();
			}
			const TValue* operator->() const noexcept
			{
				return get();
			}

			UninitializedStorage& operator=(const UninitializedStorage&) = delete;
			UninitializedStorage& operator=(UninitializedStorage&&) = delete;
	};
}

namespace kxf
{
	template<class TValue_, size_t t_Size = sizeof(TValue_), size_t t_Alignment = alignof(TValue_)>
	class GuardedUninitializedStorage final
	{
		public:
			using TValue = TValue_;

		private:
			UninitializedStorage<TValue_, t_Size, t_Alignment> m_Storage;

		public:
			template<class... Args>
			requires(std::is_constructible_v<TValue, Args...>)
			GuardedUninitializedStorage(Args&&... arg) noexcept(std::is_nothrow_constructible_v<TValue, Args...>)
			{
				if constexpr(t_Alignment != 0)
				{
					m_Storage.Construct(std::forward<Args>(arg)...);
				}
				else
				{
					m_Storage.AlignAndConstruct(std::forward<Args>(arg)...);
				}
			}

			GuardedUninitializedStorage(const GuardedUninitializedStorage&) = delete;
			GuardedUninitializedStorage(GuardedUninitializedStorage&&) = delete;

			~GuardedUninitializedStorage() noexcept(std::is_nothrow_destructible_v<TValue>)
			{
				m_Storage.Destroy();
			}

		public:
			auto& GetBuffer() noexcept
			{
				return m_Storage.GetBuffer();
			}
			const auto& GetBuffer() const noexcept
			{
				return m_Storage.GetBuffer();
			}

			auto& GetStorage() noexcept
			{
				return m_Storage;
			}
			const auto& GetStorage() const noexcept
			{
				return m_Storage;
			}

			size_t size() const noexcept
			{
				return m_Storage.size();
			}
			size_t alignment() const noexcept
			{
				return m_Storage.alignment();
			}

			TValue* get() noexcept
			{
				return m_Storage.get();
			}
			const TValue* get() const noexcept
			{
				return m_Storage.get();
			}

			TValue* operator&() noexcept
			{
				return get();
			}
			const TValue* operator&() const noexcept
			{
				return get();
			}

			TValue& operator*() noexcept
			{
				return *get();
			}
			const TValue& operator*() const noexcept
			{
				return *get();
			}

			TValue* operator->() noexcept
			{
				return get();
			}
			const TValue* operator->() const noexcept
			{
				return get();
			}

		public:
			GuardedUninitializedStorage& operator=(const GuardedUninitializedStorage&) = delete;
			GuardedUninitializedStorage& operator=(GuardedUninitializedStorage&&) = delete;

			explicit operator bool() const noexcept
			{
				return m_Storage.IsConstructed();
			}
			bool operator!() const noexcept
			{
				return !m_Storage.IsConstructed();
			}
	};
}
