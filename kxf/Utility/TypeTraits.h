#pragma once
#include <type_traits>

namespace kxf::Utility
{
	template<class>
	struct MethodTraits;

	template<class Rx, class Tx, class... Args>
	struct MethodTraits<Rx(Tx::*)(Args...)>
	{
		using TReturn = Rx;
		using TInstance = Tx;
		using TArgsTuple = typename std::tuple<Args...>;

		inline static constexpr size_t ArgumentCount = sizeof...(Args);
	};

	template<class>
	struct FunctionPtrTraits;

	template<class Rx, class... Args>
	struct FunctionPtrTraits<Rx(*)(Args...)>
	{
		using TReturn = Rx;
		using TArgsTuple = typename std::tuple<Args...>;

		inline static constexpr size_t ArgumentCount = sizeof...(Args);
	};

	template<size_t N, class... Args>
	using NthTypeOf = typename std::tuple_element<N, std::tuple<Args...>>::type;
}

namespace kxf::Utility
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

namespace kxf::Utility
{
	template<class T>
	struct is_unique_ptr: std::false_type {};

	template<class T, class D>
	struct is_unique_ptr<std::unique_ptr<T, D>>: std::true_type {};

	template<class T, class D>
	struct is_unique_ptr<const std::unique_ptr<T, D>>: std::true_type {};

	template<class T, class D>
	struct is_unique_ptr<std::unique_ptr<T, D>&>: std::true_type {};

	template<class T, class D>
	struct is_unique_ptr<const std::unique_ptr<T, D>&>: std::true_type {};

	template<class T>
	inline constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;
}

namespace kxf::Utility
{
	template<class T>
	struct is_optional: std::false_type {};

	template<class T>
	struct is_optional<std::optional<T>>: std::true_type {};

	template<class T>
	inline constexpr bool is_optional_v = is_optional<T>::value;
}

namespace kxf::Utility
{
	template<class T, bool isEnum = std::is_enum_v<T>, bool isInteger = std::is_integral_v<T>>
	struct any_underlying_type
	{
		using type = typename std::underlying_type<T>::type;
	};

	template<class T>
	struct any_underlying_type<T, false, true>
	{
		using type = typename T;
	};

	template<class T>
	using any_underlying_type_t = typename any_underlying_type<T>::type;
}

namespace kxf::Utility
{
	template<class, class = void>
	constexpr bool is_type_complete_v = false;

	template<class T>
	constexpr bool is_type_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;
}
