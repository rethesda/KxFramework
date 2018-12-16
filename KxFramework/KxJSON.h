/*
Copyright � 2018 Kerber. All rights reserved.

You should have received a copy of the GNU LGPL v3
along with KxFramework. If not, see https://www.gnu.org/licenses/lgpl-3.0.html.
*/
#pragma once
#include "KxFramework/KxFramework.h"
#include "KxFramework/nlohmann/json.hpp"

using KxJSONObject = nlohmann::json;
class KX_API KxJSON: public KxJSONObject
{
	public:
		static wxString GetVersion();
		static wxString GetLibraryName();

	public:
		static KxJSONObject Load(const wxString& json);
		static KxJSONObject Load(wxInputStream& stream);

		static wxString Save(const KxJSONObject& json);
		static bool Save(const KxJSONObject& json, wxOutputStream& stream);

	public:
		KxJSON() = delete;
		~KxJSON() = delete;
};

namespace nlohmann
{
	template<> struct KX_API adl_serializer<wxString>
	{
		static void to_json(json& j, const wxString& value)
		{
			auto utf8 = value.ToUTF8();
			j = std::string_view(utf8.data(), utf8.length());
		}
		static void from_json(const json& j, wxString& value)
		{
			if (j.is_null())
			{
				value.clear();
			}
			else
			{
				const json::string_t& string = j.get_ref<const json::string_t&>();
				value = wxString::FromUTF8Unchecked(string.data(), string.length());
			}
		}
	};
}
