#pragma once
#include "Common.h"
#include "String.h"
#include "CallbackFunction.h"
#include "kxf/Utility/Common.h"

namespace kxf
{
	template<class TEnum, class TChar>
	class IndexedEnumItem final
	{
		private:
			TEnum m_Value;
			std::basic_string_view<TChar> m_Name;
			std::basic_string_view<TChar> m_Message;

		public:
			constexpr IndexedEnumItem(TEnum value, std::basic_string_view<TChar> name, std::basic_string_view<TChar> message = {})
				:m_Value(value), m_Name(std::move(name)), m_Message(std::move(message))
			{
			}

		public:
			constexpr TEnum GetValue() const
			{
				return m_Value;
			}
			constexpr std::basic_string_view<TChar> GetName() const
			{
				return m_Name;
			}
			constexpr std::basic_string_view<TChar> GetMessage() const
			{
				return m_Message;
			}
	};

	template<class TDerived_, class TEnum_, class TChar_ = char>
	class IndexedEnumDefinition
	{
		public:
			using TDerived = TDerived_;
			using TItem = IndexedEnumItem<TEnum_, TChar_>;
			using TEnum = TEnum_;

		protected:
			constexpr static const auto& GetItems() noexcept
			{
				return TDerived::Items;
			}
			constexpr static const TItem* FindByName(const String& string)
			{
				for (const auto& item: GetItems())
				{
					if (item.GetName() == string)
					{
						return &item;
					}
				}
				return nullptr;
			}
			constexpr static const TItem* FindByValue(TEnum value)
			{
				for (const auto& item: GetItems())
				{
					if (item.GetValue() == value)
					{
						return &item;
					}
				}
				return nullptr;
			}
			
		public:
			constexpr IndexedEnumDefinition() noexcept = delete;

		public:
			constexpr static size_t GetItemCount()
			{
				return std::size(GetItems());
			}
			constexpr static std::optional<TEnum> QueryValue(const String& name)
			{
				if (auto item = FindByName(name))
				{
					return item->GetValue();
				}
				return {};
			}
			constexpr static std::optional<String> QueryName(TEnum value)
			{
				if (auto item = FindByValue(value))
				{
					return item->GetName();
				}
				return {};
			}
			constexpr static std::optional<String> QueryMessage(TEnum value)
			{
				if (auto item = FindByValue(value))
				{
					return item->GetMessage();
				}
				return {};
			}
			
			template<class TFunc>
			constexpr static size_t EnumItems(TFunc&& func)
			{
				size_t count = 0;
				for (const auto& item: GetItems())
				{
					count++;
					if (std::invoke(func, item) == CallbackCommand::Terminate)
					{
						break;
					}
				}
				return count;
			}

			static FlagSet<TEnum> FromBitwiseOrExpression(const String& bitwiseExpression, FlagSet<TEnum> initialValue = {})
			{
				if (!bitwiseExpression.IsEmpty())
				{
					auto flags = initialValue;
					String::SplitBySeparator(bitwiseExpression, '|', [&](StringView name)
					{
						if (auto item = FindByName(name))
						{
							flags.Add(item->GetValue());
						}
						return true;
					});
					return flags;
				}
				return {};
			}
			static String ToBitwiseOrExpression(FlagSet<TEnum> flags)
			{
				if (!flags.IsNull())
				{
					String bitwiseExpression;
					for (const auto& item: GetItems())
					{
						if (flags.Contains(item.GetValue()))
						{
							if (!bitwiseExpression.empty())
							{
								bitwiseExpression += '|';
							}
							bitwiseExpression += item.GetName();
						}
					}

					return bitwiseExpression;
				}
				return {};
			}
	};
}
