#pragma once
#include "../Common.h"
#include "kxf/IO/IStream.h"
#include "../XDocument.h"
#include "../BinarySerializer.h"
#include <nlohmann/json.hpp>

namespace kxf
{
	class KXF_API JSONDocument final: public RTTI::DynamicImplementation<JSONDocument, IXDocument>
	{
		kxf_RTTI_DeclareIID(JSONDocument, {0x8a798487, 0x31c, 0x467d, {0x9a, 0x66, 0xb, 0x28, 0xf4, 0x63, 0x74, 0xaa}});

		private:
			// IObject
			RTTI::QueryInfo DoQueryInterface(const IID& iid) noexcept override;

		private:
			nlohmann::json m_Impl;

		public:
			JSONDocument() = default;
			JSONDocument(const String& json)
			{
				LoadDocument(json);
			}

		public:
			// IXDocument
			bool IsNull() const override;
			String GetDocumentMeta() const override;

			bool LoadDocument(IInputStream& stream) override;
			bool SaveDocument(IOutputStream& stream) const override;

			// JSONDocument
			const nlohmann::json& json() const noexcept
			{
				return m_Impl;
			}
			nlohmann::json& json() noexcept
			{
				return m_Impl;
			}

			bool LoadDocument(const String& json);
			String SaveDocument() const;

			void ClearDocument();
	};
}

namespace nlohmann
{
	template<>
	struct adl_serializer<kxf::String> final
	{
		static void to_json(json& json, const kxf::String& value)
		{
			auto utf8 = value.utf8_str();
			json = std::string_view(utf8.data(), utf8.length());
		}
		static void from_json(const json& json, kxf::String& value)
		{
			if (json.is_null())
			{
				value.clear();
			}
			else
			{
				auto& string = json.get_ref<const json::string_t&>();
				value = kxf::String::FromUTF8({string.data(), string.length()});
			}
		}
	};
}

namespace kxf
{
	template<>
	struct BinarySerializer<JSONDocument> final
	{
		uint64_t Serialize(IOutputStream& stream, const JSONDocument& value) const
		{
			return BinarySerializer<String>().Serialize(stream, value.SaveDocument());
		}
		uint64_t Deserialize(IInputStream& stream, JSONDocument& value) const
		{
			String buffer;
			auto read = BinarySerializer<String>().Deserialize(stream, buffer);

			value.LoadDocument(buffer);
			return read;
		}
	};
}
