#pragma once
#include <type_traits>

namespace KxFramework::Utility
{
	template<class>
	struct MethodTraits;

	template<class Return, class Object, class... Args>
	struct MethodTraits<Return(Object::*)(Args...)>
	{
		using TReturn = Return;
		using TInstance = Object;

		inline static constexpr size_t ArgumentCount = sizeof...(Args);
	};

	template<size_t N, typename... T>
	using NthTypeOf = typename std::tuple_element<N, std::tuple<T...>>::type;
}

namespace KxFramework::Utility
{
	template<class TCallable, class... Args>
	struct CallableTraits
	{
		inline static constexpr bool IsInvokable = std::is_invocable_v<TCallable, Args...>;
		inline static constexpr bool IsFreeFunction = std::is_function_v<std::remove_pointer_t<TCallable>>;
		inline static constexpr bool IsMemberFunction = std::is_member_function_pointer_v<TCallable>;
		inline static constexpr bool IsFunctor = IsInvokable && (!IsFreeFunction && !IsMemberFunction);
	};
}

namespace KxFramework::Utility
{
	template<class T>
	struct is_unique_ptr: std::false_type
	{
	};

	template<class T, class D>
	struct is_unique_ptr<std::unique_ptr<T, D>>: std::true_type
	{
	};

	template<class T, class D>
	struct is_unique_ptr<const std::unique_ptr<T, D>>: std::true_type
	{
	};

	template<class T, class D>
	struct is_unique_ptr<std::unique_ptr<T, D>&>: std::true_type
	{
	};

	template<class T, class D>
	struct is_unique_ptr<const std::unique_ptr<T, D>&>: std::true_type
	{
	};

	template<class T>
	inline constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;
}
