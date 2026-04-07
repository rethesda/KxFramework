#pragma once
#include "../Common.h"
#include "../XDocument.h"
#include "kxf/Core/Version.h"
#include "kxf/RTTI/RTTI.h"
#include "kxf/IO/IStream.h"

namespace kxf
{
	class INIDocument;
	class INIDocumentImpl;
	class INIDocumentSection;

	enum class INIDocumentOption: uint32_t
	{
		None = 0,

		Spaces = 1u << 0,
		Quotes = 1u << 1,
		MultiKey = 1u << 2,
		InlineComments = 1u << 3,
		IgnoreCase = 1u << 4,
		ExtendedSytnax = 1 << 5
	};
	kxf_FlagSet_Declare(INIDocumentOption);
}

namespace kxf
{
	class KXF_API INIDocumentValue final: public XDocumentNode,
										  public XDocument::RWValue<INIDocumentValue>,
										  public XDocument::RWAttribute<INIDocumentValue>,
										  private XDocument::DefaultConverter<INIDocumentValue>
	{
		friend class ROValue;
		friend class RWValue;
		friend class ROAttribute;
		friend class RWAttribute;
		friend class DefaultConverter;

		friend class INIDocument;
		friend class INIDocumentImpl;

		private:
			INIDocument* m_Ref = nullptr;
			std::map<String, String> m_Attributes;
			String m_SectionName;
			String m_KeyName;
			bool m_ExtendedSyntaxEnabled = false;

		private:
			// XDocument::RWValue
			std::optional<String> XDocument_QueryValue() const;
			bool XDocument_WriteValue(const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA);

			// XDocument::RWAttribute
			std::optional<String> XDocument_QueryAttribute(const String& name) const;
			bool XDocument_WriteAttribute(const String& name, const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA);

			void ReadAttributes();
			void WriteAttributes();

		public:
			INIDocumentValue() = default;
			INIDocumentValue(const INIDocumentValue&) = default;
			INIDocumentValue(INIDocumentValue&& other) noexcept
			{
				*this = std::move(other);
			}
		private:
			INIDocumentValue(INIDocument& document, String sectionName, String keyName);

		public:
			// XDocumentNode
			bool IsNull() const override
			{
				return m_Ref == nullptr;
			}
			String GetXPath() const override;

			size_t GetIndexWithinParent() const override;
			size_t GetRelativeIndexWithinParent() const override;
			String GetName() const override
			{
				return m_KeyName;
			}

			// INIDocumentSection
			bool DoesExist() const;
			bool SetName(const String& name)
			{
				m_KeyName = name;
				return true;
			}

			String GetSectionName() const
			{
				return m_SectionName;
			}
			INIDocumentSection GetSection() const;

			String GetComment() const;
			bool SetComment(const String& comment);

			bool HasAttributes() const
			{
				return GetAttributeCount() != 0;
			}
			size_t GetAttributeCount() const;
			CallbackResult<void> EnumAttributeNames(CallbackFunction<String> func) const;
			CallbackResult<void> EnumAttributeValues(const String& name, CallbackFunction<String> func) const;

			bool HasAttribute(const String& name) const;
			bool RemoveAttribute(const String& name);
			bool Remove();
			bool Clear();

			void CommitAttributes();

		public:
			INIDocumentValue& operator=(const INIDocumentValue&) = default;
			INIDocumentValue& operator=(INIDocumentValue&& other) noexcept
			{
				m_Ref = std::exchange(other.m_Ref, nullptr);
				m_Attributes = std::move(other.m_Attributes);
				m_SectionName = std::move(other.m_SectionName);
				m_KeyName = std::move(other.m_KeyName);
				m_ExtendedSyntaxEnabled = std::exchange(other.m_ExtendedSyntaxEnabled, false);

				return *this;
			}
	};
}

namespace kxf
{
	class KXF_API INIDocumentSection final: public XDocumentNode,
											public XDocument::RWValue<INIDocumentSection>,
											public XDocument::RWAttribute<INIDocumentSection>,
											private XDocument::DefaultConverter<INIDocumentSection>
	{
		friend class ROValue;
		friend class RWValue;
		friend class ROAttribute;
		friend class RWAttribute;
		friend class DefaultConverter;

		friend class INIDocument;
		friend class INIDocumentImpl;
		friend class INIDocumentValue;

		private:
			INIDocument* m_Ref = nullptr;
			String m_SectionName;

		private:
			// XDocument::RWValue
			std::optional<String> XDocument_QueryValue() const;
			bool XDocument_WriteValue(const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA);

			// XDocument::RWAttribute
			std::optional<String> XDocument_QueryAttribute(const String& name) const;
			bool XDocument_WriteAttribute(const String& name, const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA);

		public:
			INIDocumentSection() = default;
			INIDocumentSection(const INIDocumentSection&) = default;
			INIDocumentSection(INIDocumentSection&& other) noexcept
			{
				*this = std::move(other);
			}
		private:
			INIDocumentSection(INIDocument& document, String sectionName)
				:m_Ref(&document), m_SectionName(std::move(sectionName))
			{
			}

		public:
			// XDocumentNode
			bool IsNull() const override
			{
				return m_Ref == nullptr;
			}
			String GetXPath() const override;

			size_t GetIndexWithinParent() const override;
			size_t GetRelativeIndexWithinParent() const override;
			String GetName() const override
			{
				return m_SectionName;
			}

			// INIDocumentSection
			bool DoesExist() const;
			bool SetName(const String& name)
			{
				m_SectionName = name;
				return true;
			}

			String GetComment() const;
			bool SetComment(const String& comment);

			bool HasAttributes() const
			{
				return GetAttributeCount() != 0;
			}
			size_t GetAttributeCount() const;
			CallbackResult<void> EnumAttributeNames(CallbackFunction<String> func, bool uniqueOnly = false) const;
			CallbackResult<void> EnumAttributeValues(const String& name, CallbackFunction<String> func) const;
			INIDocumentValue QueryAttributeValue(const String& name) const;

			bool HasAttribute(const String& name) const;
			bool RemoveAttribute(const String& name);
			bool Remove();
			bool Clear();

		public:
			INIDocumentSection& operator=(const INIDocumentSection&) = default;
			INIDocumentSection& operator=(INIDocumentSection&& other) noexcept
			{
				m_Ref = std::exchange(other.m_Ref, nullptr);
				m_SectionName = std::move(other.m_SectionName);

				return *this;
			}
	};
}

namespace kxf
{
	class KXF_API INIDocument final: public RTTI::DynamicImplementation<INIDocument, IXDocument>,
									 public XDocument::RWAttribute<INIDocument>,
									 private XDocument::DefaultConverter<INIDocument>
	{
		kxf_RTTI_DeclareIID(INIDocument, {0xd8bae2c7, 0xd44d, 0x4073, {0x91, 0x53, 0x82, 0x5f, 0x38, 0xc2, 0x5, 0x3b}});

		friend class ROAttribute;
		friend class RWAttribute;
		friend class DefaultConverter;

		friend class INIDocumentValue;
		friend class INIDocumentSection;

		private:
			std::unique_ptr<INIDocumentImpl> m_Document;
			FlagSet<INIDocumentOption> m_Options;

		private:
			// XDocument::RWAttribute
			std::optional<String> XDocument_QueryAttribute(const String& name) const;
			bool XDocument_WriteAttribute(const String& name, const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA);

			// IObject
			RTTI::QueryInfo DoQueryInterface(const IID& iid) noexcept override;

			// INIDocument
			void Init();
			bool DoLoad(const char* ini, size_t length);
			void DoUnload();

			std::optional<String> IniDoGetValue(const String& sectionName, const String& keyName, String* comment = nullptr, size_t* order = nullptr) const;
			bool IniDoSetValue(const String& sectionName, const String& keyName, const String& value, const String& comment = {}, WriteEmpty writeEmpty = WriteEmpty::Always, AsCDATA asCDATA = AsCDATA::Auto);

			bool RemoveQuotes(String& value) const;
			bool RemoveInlineComments(String& value, String* comment = nullptr) const;
			bool StartsWithInlineComment(const String& value) const;
			std::optional<String> ProcessItem(const char* rawData, FlagSet<INIDocumentOption> options) const;

		public:
			INIDocument();
			INIDocument(const String& ini)
				:INIDocument()
			{
				if (!ini.IsEmpty())
				{
					LoadDocument(ini);
				}
			}
			INIDocument(IInputStream& stream)
				:INIDocument()
			{
				LoadDocument(stream);
			}
			INIDocument(const INIDocument&) = delete;
			INIDocument(INIDocument&& other) noexcept;
			~INIDocument();

		public:
			// IXDocument
			bool IsNull() const override;
			String GetDocumentMeta() const override;

			bool LoadDocument(IInputStream& stream);
			bool SaveDocument(IOutputStream& stream) const;

			// INIDocument
			bool LoadDocument(const String& ini);
			bool LoadDocument(std::span<const char8_t> utf8Data);
			String SaveDocument() const;

			INIDocument Clone() const;
			void Clear();

			FlagSet<INIDocumentOption> GetOptions() const;
			void SetOptions(FlagSet<INIDocumentOption> options);

			String GetComment() const;
			bool SetComment(String comment);

			// INIDocument: Sections
			INIDocumentSection QuerySection(const String& sectionName) const;
			CallbackResult<void> EnumSections(CallbackFunction<INIDocumentSection> func) const;
			CallbackResult<void> EnumSectionNames(CallbackFunction<String> func) const;
			CallbackResult<void> EnumSectionValues(const String& sectionName, CallbackFunction<String, String> func) const;

			size_t GetSectionCount() const;
			bool HasSection(const String& sectionName) const;
			bool ClearSection(const String& sectionName);
			bool RemoveSection(const String& sectionName);

			// INIDocument: Attributes
			INIDocumentValue QuerySectionValue(const String& sectionName, const String& keyName) const;
			CallbackResult<void> EnumAttributeNames(const String& sectionName, CallbackFunction<String> func, bool uniqueOnly = false) const;
			CallbackResult<void> EnumAttributeValues(const String& sectionName, const String& keyName, CallbackFunction<String> func) const;

			bool HasSectionAttribute(const String& sectionName, const String& keyName) const;
			bool RemoveSectionAttribute(const String& sectionName, const String& keyName);

			template<class T = String>
			requires(std::is_same_v<T, String>)
			std::optional<T> QuerySectionAttribute(const String& sectionName, const String& keyName) const
			{
				return IniDoGetValue(sectionName, keyName);
			}

			template<class T>
			requires(std::is_same_v<T, bool>)
			std::optional<T> QuerySectionAttribute(const String& sectionName, const String& keyName) const
			{
				if (auto value = IniDoGetValue(sectionName, keyName))
				{
					return XDocument_ConvertToBool(*value);
				}
				return {};
			}

			template<class T>
			requires(std::is_same_v<T, void*>)
			std::optional<T> QuerySectionAttribute(const String& sectionName, const String& keyName) const
			{
				if (auto value = IniDoGetValue(sectionName, keyName))
				{
					return XDocument_ConvertToPointer(*value);
				}
				return {};
			}

			template<class T>
			requires((std::is_integral_v<T> || std::is_enum_v<T>) && !std::is_same_v<T, bool>)
			std::optional<T> QuerySectionAttribute(const String& sectionName, const String& keyName) const
			{
				if (auto value = IniDoGetValue(sectionName, keyName))
				{
					return XDocument_ConvertToInt<T>(*value);
				}
				return {};
			}

			template<class T>
			requires(std::is_floating_point_v<T>)
			std::optional<T> QuerySectionAttribute(const String& sectionName, const String& keyName) const
			{
				if (auto value = IniDoGetValue(sectionName, keyName))
				{
					return XDocument_ConvertToFloat<T>(*value);
				}
				return {};
			}

			template<class T = String>
			T GetSectionAttribute(const String& sectionName, const String& keyName) const
			{
				return QuerySectionAttribute<T>(sectionName, keyName).value_or(T{});
			}

			bool SetSectionAttribute(const String& sectionName, const String& keyName, const String& value, WriteEmpty writeEmpty = WriteEmpty::Always, AsCDATA asCDATA = AsCDATA::Auto)
			{
				return IniDoSetValue(sectionName, keyName, value, {}, writeEmpty, asCDATA);
			}
			bool SetSectionAttribute(const String& sectionName, const String& keyName, bool value)
			{
				return IniDoSetValue(sectionName, keyName, XDocument_ConvertFromBool(value), {}, WriteEmpty::Always, AsCDATA::Never);
			}
			bool SetSectionAttribute(const String& sectionName, const String& keyName, void* value)
			{
				return IniDoSetValue(sectionName, keyName, XDocument_ConvertFromPointer(value), {}, WriteEmpty::Always, AsCDATA::Never);
			}

			template<class T>
			requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
			bool SetSectionAttribute(const String& sectionName, const String& keyName, T value)
			{
				return IniDoSetValue(sectionName, keyName, XDocument_ConvertFromInt<T>(value), {}, WriteEmpty::Always, AsCDATA::Never);
			}

			template<class T>
			requires(std::is_floating_point_v<T>)
			bool SetSectionAttribute(const String& sectionName, const String& keyName, T value, int precision = -1)
			{
				return IniDoSetValue(sectionName, keyName, XDocument_ConvertFromFloat<T>(value, precision), {}, WriteEmpty::Always, AsCDATA::Never);
			}

		public:
			INIDocument& operator=(const INIDocument&) = delete;
			INIDocument& operator=(INIDocument&& other) noexcept;
	};
}

namespace kxf
{
	template<>
	struct BinarySerializer<INIDocument> final
	{
		uint64_t Serialize(IOutputStream& stream, const INIDocument& value) const
		{
			return BinarySerializer<String>().Serialize(stream, value.SaveDocument());
		}
		uint64_t Deserialize(IInputStream& stream, INIDocument& value) const
		{
			String buffer;
			auto read = BinarySerializer<String>().Deserialize(stream, buffer);

			value.LoadDocument(buffer);
			return read;
		}
	};
}
