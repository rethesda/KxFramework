#pragma once
#include "../Common.h"
#include "kxf/Core/CallbackFunction.h"
#include "kxf/Utility/Container.h"

namespace kxf::Sciter
{
	class Host;
}

namespace kxf::Sciter
{
	class KXF_API StylesheetStorage
	{
		private:
			std::vector<String> m_Items;

		public:
			StylesheetStorage() noexcept = default;
			virtual ~StylesheetStorage() = default;

		public:
			bool AddItem(String stylesheet);
			void ClearItems() noexcept;
			size_t GetItemCount() const noexcept
			{
				return m_Items.size();
			}
			size_t CopyItems(const StylesheetStorage& other);
			size_t TakeItems(StylesheetStorage&& other);

			CallbackResult<void> EnumItems(CallbackFunction<const String&> func) const&;
			CallbackResult<void> EnumItems(CallbackFunction<String> func) &&;

			bool Apply(Host& host, const FSPath& basePath = {}) const;
			bool Apply(Host& host, const URI& baseURI) const;
	};
}
