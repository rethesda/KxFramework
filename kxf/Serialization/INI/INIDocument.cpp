#include "kxf-pch.h"
#include "INIDocument.h"
#include "kxf/Core/ILibraryInfo.h"
#include "kxf/IO/MemoryStream.h"
#include "kxf/Serialization/StringTokenizer.h"
#include "kxf/Utility/SoftwareLicenseDB.h"

namespace SimpleINI
{
	#include "SimpleIni.h"
}
namespace
{
	constexpr char g_Copyright[] = "Copyright© 2006-2012 Brodie Thiesfield";
	constexpr int g_VersionMajor = 4;
	constexpr int g_VersionMinor = 19;
	constexpr int g_VersionPatch = 0;

	constexpr char g_EscapeCharacters[] = "\"=";
}

namespace kxf
{
	class INIDocumentImpl final: public SimpleINI::CSimpleIniA
	{
		public:
			using SimpleINI::CSimpleIniA::CSimpleIniA;

		public:
			static INIDocumentSection ToSection(const INIDocument& document, const Entry& entry)
			{
				return INIDocumentSection(const_cast<INIDocument&>(document), String::FromUTF8(entry.pItem));
			}

			CallbackResult<void> ForEach(TNamesDepend& items, CallbackFunction<Entry&> func, SortOrder order = SortOrder::None) const
			{
				if (order != SortOrder::None)
				{
					items.sort(INIDocumentImpl::Entry::LoadOrder());
				}

				if (order == SortOrder::Ascending || order == SortOrder::None)
				{
					for (auto it = items.begin(); it != items.end(); ++it)
					{
						if (func.Invoke(*it).ShouldTerminate())
						{
							break;
						}
					}
				}
				else if (order == SortOrder::Descending)
				{
					for (auto it = items.rbegin(); it != items.rend(); ++it)
					{
						if (func.Invoke(*it).ShouldTerminate())
						{
							break;
						}
					}
				}
				return func.Finalize();
			}
			CallbackResult<void> ForEachSection(CallbackFunction<Entry&> func, SortOrder order = SortOrder::None) const
			{
				TNamesDepend items;
				GetAllSections(items);

				return ForEach(items, std::move(func), order);
			}
			CallbackResult<void> ForEachKey(CallbackFunction<Entry&> func, const String& sectionName, SortOrder order = SortOrder::None) const
			{
				TNamesDepend items;
				if (GetAllKeys(sectionName.utf8_str(), items))
				{
					return ForEach(items, std::move(func), order);
				}
				return {};
			}
			CallbackResult<void> ForEachValue(CallbackFunction<Entry&> func, const String& sectionName, const String& keyName, SortOrder order = SortOrder::None) const
			{
				TNamesDepend items;
				if (GetAllValues(sectionName.utf8_str(), keyName.utf8_str(), items))
				{
					return ForEach(items, std::move(func), order);
				}
				return {};
			}
			CallbackResult<void> ForEachSectionItem(CallbackFunction<const Entry&, const char*> func, const String& sectionName) const
			{
				if (auto sectionData = GetSection(sectionName.utf8_str()))
				{
					for (auto& [name, value]: *sectionData)
					{
						if (func.Invoke(name, value).ShouldTerminate())
						{
							break;
						}
					}
					return func.Finalize();
				}
				return {};
			}
	};
}

namespace kxf
{
	// XDocument::RWValue
	std::optional<String> INIDocumentValue::XDocument_QueryValue() const
	{
		if (!m_ExtendedSyntaxEnabled)
		{
			return m_Ref->IniDoGetValue(m_SectionName, m_KeyName);
		}
		return {};
	}
	bool INIDocumentValue::XDocument_WriteValue(const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		if (!m_ExtendedSyntaxEnabled)
		{
			return m_Ref->IniDoSetValue(m_SectionName, m_KeyName, value, {}, writeEmpty, asCDATA);
		}
		return false;
	}

	// XDocument::RWAttribute
	std::optional<String> INIDocumentValue::XDocument_QueryAttribute(const String& name) const
	{
		if (m_ExtendedSyntaxEnabled)
		{
			if (auto it = m_Attributes.find(name); it != m_Attributes.end())
			{
				return it->second;
			}
		}
		return {};
	}
	bool INIDocumentValue::XDocument_WriteAttribute(const String& name, const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		if (m_ExtendedSyntaxEnabled)
		{
			if (value.IsEmpty() && writeEmpty == WriteEmpty::Never)
			{
				return false;
			}
			else if (name.IsEmptyOrWhitespace())
			{
				return false;
			}
			else
			{
				String name2 = name;
				name2.TrimBoth();

				m_Attributes.insert_or_assign(name2, value);
				return true;
			}
		}
		return false;
	}

	// INIDocumentValue
	void INIDocumentValue::ReadAttributes()
	{
		if (auto source = m_Ref->IniDoGetValue(m_SectionName, m_KeyName); source && !source->IsEmpty())
		{
			if (source->StartsWith('(') && source->EndsWith(')'))
			{
				StringTokenizer tokenizer(source->view());

				// Skip the opening brace and any whitespace
				tokenizer.Advance();
				tokenizer.SkipWhitespace();

				// Search for the next pair
				while (tokenizer.ScanUntil([&](UniChar c)
				{
					if (c == '=' && tokenizer.PeekPrevious() != '\\')
					{
						return CallbackCommand::Terminate;
					}
					return CallbackCommand::Continue;
				}))
				{
					auto nameToken = tokenizer.GetCurrentToken();

					// We're at the '=', if next is quotation mark, assume we're at the value
					if (tokenizer.PeekNext() == '\"')
					{
						// Skip both the equals sing and opening quote
						tokenizer.Advance(2);

						// Extract the value, but ignore any C-style escaped quotation marks
						if (tokenizer.ScanUntil([&](UniChar c)
						{
							if (c == '\"' && tokenizer.PeekPrevious() != '\\')
							{
								return CallbackCommand::Terminate;
							}
							return CallbackCommand::Continue;
						}))
						{
							String name = nameToken;
							name.TrimBoth();
							name.UnescapeCString();

							String value = tokenizer.GetCurrentToken();
							value.UnescapeCString();

							m_Attributes.insert_or_assign(std::move(name), std::move(value));

							// Skip the closing quote and any trailing spaces
							tokenizer.Advance();

							// Allow to separate the pairs with comma
							if (tokenizer.PeekCurrent() == ',')
							{
								tokenizer.Advance();
							}
							tokenizer.SkipWhitespace();
						}
					}
				}
			}
		}
	}
	void INIDocumentValue::WriteAttributes()
	{
		if (!m_Attributes.empty())
		{
			String result = kxfSV("(");
			for (const auto& [name, value]: m_Attributes)
			{
				auto Format = [&result](const String& name, const String& value)
				{
					result.Format(kxfSV("{}=\"{}\", "), name, value);
				};

				if (name.ContainsAnyOfCharacters(g_EscapeCharacters) || value.ContainsAnyOfCharacters(g_EscapeCharacters))
				{
					Format(String(name).EscapeCStringChars(g_EscapeCharacters), String(value).EscapeCStringChars(g_EscapeCharacters));
				}
				else
				{
					Format(name, value);
				}
			}

			result.TrimRight();
			result.TrimRight(',');
			result += kxfSV(");");

			m_Ref->IniDoSetValue(m_SectionName, m_KeyName, result, {}, WriteEmpty::Always, AsCDATA::Never);
		}
		else
		{
			m_Ref->IniDoSetValue(m_SectionName, m_KeyName, {}, {}, WriteEmpty::Always, AsCDATA::Never);
		}
	}

	INIDocumentValue::INIDocumentValue(INIDocument& document, String sectionName, String keyName)
		:m_Ref(&document), m_SectionName(std::move(sectionName)), m_KeyName(std::move(keyName))
	{
		m_ExtendedSyntaxEnabled = document.GetOptions().Contains(INIDocumentOption::ExtendedSytnax);
		if (m_ExtendedSyntaxEnabled)
		{
			ReadAttributes();
		}
	}

	// XDocumentNode
	String INIDocumentValue::GetXPath() const
	{
		if (m_Ref)
		{
			auto section = m_Ref->QuerySection(m_SectionName);

			UniChar indexSep;
			UniChar separator = m_Ref->GetXPathSeparator(&indexSep);

			if (m_Ref->GetOptions().Contains(INIDocumentOption::MultiKey))
			{
				auto index = GetRelativeIndexWithinParent();
				if (index != 0 && index != npos)
				{
					return String().Format("{}{}{}{}{}", section ? section.GetXPath() : m_SectionName, separator.GetAs<XChar>(), m_KeyName, indexSep.GetAs<XChar>(), index);
				}
			}
			return String().Format("{}{}{}", section ? section.GetXPath() : m_SectionName, separator.GetAs<XChar>(), m_KeyName);
		}
		return {};
	}
	size_t INIDocumentValue::GetIndexWithinParent() const
	{
		if (m_Ref)
		{
			size_t index = npos;
			m_Ref->IniDoGetValue(m_SectionName, m_KeyName, nullptr, &index);

			return index;
		}
		return npos;
	}
	size_t INIDocumentValue::GetRelativeIndexWithinParent() const
	{
		return npos;
	}

	// INIDocumentValue
	INIDocumentSection INIDocumentValue::GetSection() const
	{
		if (m_Ref)
		{
			return m_Ref->QuerySection(m_SectionName);
		}
		return {};
	}

	String INIDocumentValue::GetComment() const
	{
		if (m_Ref)
		{
			String comment;
			m_Ref->IniDoGetValue(m_SectionName, m_KeyName, &comment);

			return comment;
		}
		return {};
	}
	bool INIDocumentValue::SetComment(const String& comment)
	{
		if (m_Ref)
		{
			return m_Ref->IniDoSetValue(m_SectionName, m_KeyName, {}, comment, WriteEmpty::Always, AsCDATA::Never);
		}
		return false;
	}

	size_t INIDocumentValue::GetAttributeCount() const
	{
		if (m_ExtendedSyntaxEnabled)
		{
			return m_Attributes.size();
		}
		return 0;
	}
	CallbackResult<void> INIDocumentValue::EnumAttributeNames(CallbackFunction<String> func) const
	{
		if (m_ExtendedSyntaxEnabled)
		{

		}
		return {};
	}
	CallbackResult<void> INIDocumentValue::EnumAttributeValues(const String& name, CallbackFunction<String> func) const
	{
		if (m_ExtendedSyntaxEnabled)
		{

		}
		return {};
	}

	bool INIDocumentValue::HasAttribute(const String& name) const
	{
		if (m_ExtendedSyntaxEnabled)
		{
			return m_Attributes.contains(name);
		}
		return false;
	}
	bool INIDocumentValue::RemoveAttribute(const String& name)
	{
		if (m_ExtendedSyntaxEnabled)
		{
			return m_Attributes.erase(name) != 0;
		}
		return false;
	}
	bool INIDocumentValue::Remove()
	{
		if (m_Ref)
		{
			return m_Ref->RemoveSectionAttribute(m_SectionName, m_KeyName);
		}
		return false;
	}
	bool INIDocumentValue::Clear()
	{
		if (m_Ref)
		{
			return m_Ref->RemoveSectionAttribute(m_SectionName, m_KeyName);
		}
		return false;
	}

	void INIDocumentValue::CommitAttributes()
	{
		if (m_ExtendedSyntaxEnabled)
		{
			WriteAttributes();
		}
	}
}

namespace kxf
{
	// XDocument::RWValue
	std::optional<String> INIDocumentSection::XDocument_QueryValue() const
	{
		return m_Ref->IniDoGetValue(m_SectionName, {});
	}
	bool INIDocumentSection::XDocument_WriteValue(const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		return m_Ref->IniDoSetValue(m_SectionName, {}, value, {}, writeEmpty, asCDATA);
	}

	// XDocument::RWAttribute
	std::optional<String> INIDocumentSection::XDocument_QueryAttribute(const String& name) const
	{
		return m_Ref->IniDoGetValue(m_SectionName, name);
	}
	bool INIDocumentSection::XDocument_WriteAttribute(const String& name, const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		return m_Ref->IniDoSetValue(m_SectionName, name, value, {}, writeEmpty, asCDATA);
	}

	// XDocumentNode
	String INIDocumentSection::GetXPath() const
	{
		if (m_Ref)
		{
			return m_SectionName;
		}
		return {};
	}
	size_t INIDocumentSection::GetIndexWithinParent() const
	{
		if (m_Ref)
		{
			size_t index = npos;
			m_Ref->m_Document->ForEachSection([&](const INIDocumentImpl::Entry& entry)
			{
				if (m_SectionName == entry.pItem)
				{
					index = entry.nOrder != 0 ? entry.nOrder - 1 : 0;
					return CallbackCommand::Terminate;
				}
				return CallbackCommand::Continue;
			});
			return index;
		}
		return npos;
	}
	size_t INIDocumentSection::GetRelativeIndexWithinParent() const
	{
		return GetIndexWithinParent();
	}

	// INIDocumentSection
	bool INIDocumentSection::DoesExist() const
	{
		if (m_Ref)
		{
			m_Ref->HasSection(m_SectionName);
		}
		return false;
	}

	String INIDocumentSection::GetComment() const
	{
		if (m_Ref)
		{
			String comment;
			m_Ref->IniDoGetValue(m_SectionName, {}, &comment);

			return comment;
		}
		return {};
	}
	bool INIDocumentSection::SetComment(const String& comment)
	{
		if (m_Ref)
		{
			return m_Ref->IniDoSetValue(m_SectionName, {}, {}, comment, WriteEmpty::Always, AsCDATA::Never);
		}
		return false;
	}

	size_t INIDocumentSection::GetAttributeCount() const
	{
		if (m_Ref && m_Ref->m_Document)
		{
			auto count = m_Ref->m_Document->GetSectionSize(m_SectionName.utf8_str());
			return count >= 0 ? count : 0;
		}
		return 0;
	}
	CallbackResult<void> INIDocumentSection::EnumAttributeNames(CallbackFunction<String> func, bool uniqueOnly) const
	{
		if (m_Ref)
		{
			return m_Ref->EnumAttributeNames(m_SectionName, std::move(func), uniqueOnly);
		}
		return {};
	}
	CallbackResult<void> INIDocumentSection::EnumAttributeValues(const String& name, CallbackFunction<String> func) const
	{
		if (m_Ref)
		{
			return m_Ref->EnumAttributeValues(m_SectionName, name, std::move(func));
		}
		return {};
	}
	INIDocumentValue INIDocumentSection::QueryAttributeValue(const String& name) const
	{
		if (m_Ref)
		{
			return m_Ref->QuerySectionValue(m_SectionName, name);
		}
		return {};
	}

	bool INIDocumentSection::HasAttribute(const String& name) const
	{
		if (m_Ref)
		{
			return m_Ref->HasSectionAttribute(m_SectionName, name);
		}
		return false;
	}
	bool INIDocumentSection::RemoveAttribute(const String& name)
	{
		if (m_Ref)
		{
			return m_Ref->RemoveSectionAttribute(m_SectionName, name);
		}
		return false;
	}
	bool INIDocumentSection::Remove()
	{
		if (m_Ref)
		{
			return m_Ref->RemoveSection(m_SectionName);
		}
		return false;
	}
	bool INIDocumentSection::Clear()
	{
		if (m_Ref)
		{
			return m_Ref->ClearSection(m_SectionName);
		}
		return false;
	}
}

namespace kxf
{
	// IObject
	RTTI::QueryInfo INIDocument::DoQueryInterface(const IID& iid) noexcept
	{
		if (iid.IsOfType<ILibraryInfo>())
		{
			class INIDocumentLibraryInfo final: public ILibraryInfo
			{
				public:
					// ILibraryInfo
					String GetName() const override
					{
						return "SimpleINI";
					}
					Version GetVersion() const override
					{
						return {g_VersionMajor, g_VersionMinor, g_VersionPatch};
					}
					uint32_t GetAPILevel() const override
					{
						return g_VersionMajor * 1000 + g_VersionMinor * 100 + g_VersionPatch * 10;
					}
					URI GetHomePage() const override
					{
						return "https://github.com/brofield/simpleini";
					}

					String GetLicense() const override
					{
						return SoftwareLicenseDB::Get().GetText(SoftwareLicenseType::MIT, g_Copyright);
					}
					String GetLicenseName() const override
					{
						return SoftwareLicenseDB::Get().GetName(SoftwareLicenseType::MIT);
					}
					String GetCopyright() const override
					{
						return g_Copyright;
					}
			};

			static INIDocumentLibraryInfo libraryInfo;
			return static_cast<ILibraryInfo&>(libraryInfo);
		}
		return IObject::DoQueryInterface(iid);
	}

	// XDocument::RWAttribute
	std::optional<String> INIDocument::XDocument_QueryAttribute(const String& name) const
	{
		return IniDoGetValue({}, name);
	}
	bool INIDocument::XDocument_WriteAttribute(const String& name, const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		return IniDoSetValue({}, name, value, {}, writeEmpty, asCDATA);
	}

	// INIDocument
	void INIDocument::Init()
	{
		m_Document = std::make_unique<INIDocumentImpl>();
		m_Document->SetAllowKeyOnly(false);
		m_Document->SetUnicode(true);
		m_Document->SetMultiLine(false);
		SetOptions(m_Options);
	}
	bool INIDocument::DoLoad(const char* ini, size_t length)
	{
		if (!m_Document)
		{
			Init();
		}
		return m_Document->LoadData(ini, length) == SimpleINI::SI_OK;
	}
	void INIDocument::DoUnload()
	{
		if (m_Document)
		{
			m_Document->Reset();
		}
	}

	std::optional<String> INIDocument::IniDoGetValue(const String& sectionName, const String& keyName, String* comment, size_t* order) const
	{
		if (m_Document && !m_Document->IsEmpty())
		{
			const auto options = GetOptions();
			if (options.Contains(INIDocumentOption::InlineComments) && StartsWithInlineComment(keyName))
			{
				return {};
			}

			String keyName2 = keyName;
			keyName2.TrimBoth();
			if (options.Contains(INIDocumentOption::Quotes))
			{
				RemoveQuotes(keyName2);
			}

			std::optional<String> value;
			if (comment || order)
			{
				// Single key only for now
				m_Document->ForEachValue([&](const INIDocumentImpl::Entry& entry)
				{
					value = String::FromUTF8(entry.pItem);
					Utility::SetIfNotNull(comment, String::FromUTF8(entry.pComment));
					Utility::SetIfNotNull(order, entry.nOrder != 0 ? entry.nOrder - 1 : 0);

					return CallbackCommand::Terminate;
				}, sectionName, keyName2);
			}
			else
			{
				if (const char* ptr = m_Document->GetValue(sectionName.utf8_str(), (!keyName2.IsEmpty() ? keyName2.utf8_str() : nullptr), nullptr))
				{
					value = String::FromUTF8(ptr);
				}
			}

			if (value && !value->IsEmpty())
			{
				if (options.Contains(INIDocumentOption::InlineComments))
				{
					RemoveInlineComments(*value, comment);
				}
				if (options.Contains(INIDocumentOption::Quotes))
				{
					RemoveQuotes(*value);
				}
			}
			return value;
		}
		return {};
	}
	bool INIDocument::IniDoSetValue(const String& sectionName, const String& keyName, const String& value, const String& comment, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		if (writeEmpty == WriteEmpty::Never && value.IsEmpty())
		{
			return false;
		}
		else
		{
			if (!m_Document)
			{
				Init();
			}

			String keyName2 = keyName;
			keyName2.TrimBoth();

			auto status = m_Document->SetValue(sectionName.utf8_str(),
											   (!keyName2.IsEmpty() ? keyName2.utf8_str() : nullptr),
											   (!value.IsEmpty() ? value.utf8_str() : nullptr),
											   (!comment.IsEmpty() ? comment.utf8_str() : nullptr),
											   true);
			return status == SimpleINI::SI_UPDATED || status == SimpleINI::SI_INSERTED;
		}
	}

	bool INIDocument::RemoveQuotes(String& value) const
	{
		constexpr auto quote1 = '\"';
		constexpr auto quote2 = '\'';

		size_t count = 0;
		auto itL = value.begin();
		auto itR = value.rbegin();
		for (; itL != value.end() && itR != value.rend(); ++itL, ++itR)
		{
			if ((*itL == quote1 && *itR == quote1) || (*itL == quote2 && *itR == quote2))
			{
				count++;
			}
			else
			{
				break;
			}
		}

		if (count != 0)
		{
			value.Remove(0, count);
			value.RemoveRight(count);

			return true;
		}
		return false;
	}
	bool INIDocument::RemoveInlineComments(String& value, String* comment) const
	{
		size_t length = 1;
		size_t index = value.ReverseFind(';');
		if (index == npos)
		{
			index = value.ReverseFind('#');
			length = 1;
		}
		if (index == npos)
		{
			index = value.ReverseFind(kxfS("//"));
			length = 2;
		}

		if (index != npos)
		{
			if (comment)
			{
				*comment = value.SubMid(index + length);
			}
			value.Truncate(index);
			value.TrimRight();

			return true;
		}
		return false;
	}
	bool INIDocument::StartsWithInlineComment(const String& value) const
	{
		if (!value.IsEmpty())
		{
			auto c = value.front();
			if (c == ';' || c == '#')
			{
				return true;
			}
			else if (value.length() >= 2 && c == '/' && value[1] == '/')
			{
				return true;
			}
		}
		return false;
	}
	std::optional<String> INIDocument::ProcessItem(const char* rawData, FlagSet<INIDocumentOption> options) const
	{
		auto item = String::FromUTF8(rawData);
		if (options.Contains(INIDocumentOption::InlineComments) && StartsWithInlineComment(item))
		{
			return {};
		}
		if (options.Contains(INIDocumentOption::Quotes))
		{
			RemoveQuotes(item);
		}

		return item;
	}

	INIDocument::INIDocument()
	{
		Init();
	}
	INIDocument::INIDocument(INIDocument&& other) noexcept
	{
		*this = std::move(*this);
	}
	INIDocument::~INIDocument() = default;

	// IXDocument
	bool INIDocument::IsNull() const
	{
		return !m_Document || m_Document->IsEmpty();
	}
	String INIDocument::GetMeta() const
	{
		return {};
	}

	bool INIDocument::LoadDocument(IInputStream& stream)
	{
		DoUnload();

		MemoryOutputStream memoryStream;
		memoryStream.Write(stream);
		auto& buffer = memoryStream.GetStreamBuffer();

		return DoLoad(reinterpret_cast<const char*>(buffer.GetBufferStart()), buffer.GetBufferSize());
	}
	bool INIDocument::SaveDocument(IOutputStream& stream) const
	{
		if (m_Document)
		{
			std::string buffer;
			m_Document->Save(buffer, false);

			return stream.WriteAll(buffer.data(), buffer.size());
		}
		return false;
	}

	// INIDocument
	bool INIDocument::LoadDocument(const String& ini)
	{
		DoUnload();

		if (!ini.IsEmpty())
		{
			auto utf8 = ini.ToUTF8();
			return DoLoad(utf8.data(), utf8.size());
		}
		return false;
	}
	bool INIDocument::LoadDocument(std::span<const char8_t> utf8Data)
	{
		DoUnload();

		if (!utf8Data.empty())
		{
			return DoLoad(reinterpret_cast<const char*>(utf8Data.data()), utf8Data.size_bytes());
		}
		return false;
	}
	String INIDocument::SaveDocument() const
	{
		if (m_Document)
		{
			std::string buffer;
			m_Document->Save(buffer, false);

			return String::FromUTF8(buffer);
		}
		return {};
	}

	INIDocument INIDocument::Clone() const
	{
		if (m_Document)
		{
			std::string buffer;
			m_Document->Save(buffer, false);

			INIDocument document;
			document.DoLoad(buffer.data(), buffer.size());
			return document;
		}
		return {};
	}
	void INIDocument::Clear()
	{
		DoUnload();
	}

	FlagSet<INIDocumentOption> INIDocument::GetOptions() const
	{
		if (m_Document)
		{
			FlagSet<INIDocumentOption> options = m_Options;
			options.Add(INIDocumentOption::IgnoreCase);
			options.Mod(INIDocumentOption::Spaces, m_Document->UsingSpaces());
			options.Mod(INIDocumentOption::Quotes, m_Document->UsingQuotes());
			options.Mod(INIDocumentOption::MultiKey, m_Document->IsMultiKey());

			return options;
		}
		return {};
	}
	void INIDocument::SetOptions(FlagSet<INIDocumentOption> options)
	{
		if (m_Document)
		{
			m_Options = options;
			m_Document->SetSpaces(options.Contains(INIDocumentOption::Spaces));
			m_Document->SetQuotes(options.Contains(INIDocumentOption::Quotes));
			m_Document->SetMultiKey(options.Contains(INIDocumentOption::MultiKey));
		}
	}

	kxf::String INIDocument::GetComment() const
	{
		String comment;
		IniDoGetValue({}, {}, &comment);

		return comment;
	}
	bool INIDocument::SetComment(String comment)
	{
		return IniDoSetValue({}, {}, {}, comment, WriteEmpty::Never, AsCDATA::Never);
	}

	// INIDocument: Sections
	INIDocumentSection INIDocument::QuerySection(const String& sectionName) const
	{
		if (m_Document)
		{
			return INIDocumentSection(const_cast<INIDocument&>(*this), sectionName);
		}
		return {};
	}
	CallbackResult<void> INIDocument::EnumSections(CallbackFunction<INIDocumentSection> func) const
	{
		if (m_Document)
		{
			return m_Document->ForEachSection([&](const INIDocumentImpl::Entry& entry)
			{
				return func.Invoke(INIDocumentImpl::ToSection(*this, entry)).GetLastCommand();
			}, SortOrder::Ascending);
		}
		return {};
	}
	CallbackResult<void> INIDocument::EnumSectionNames(CallbackFunction<String> func) const
	{
		if (m_Document)
		{
			return m_Document->ForEachSection([&, options = GetOptions()](const INIDocumentImpl::Entry& entry)
			{
				return func.Invoke(String::FromUTF8(entry.pItem)).GetLastCommand();
			}, SortOrder::Ascending);
		}
		return {};
	}
	CallbackResult<void> INIDocument::EnumSectionValues(const String& sectionName, CallbackFunction<String, String> func) const
	{
		if (m_Document)
		{
			return m_Document->ForEachSectionItem([&, options = GetOptions()](const INIDocumentImpl::Entry& nameEntry, const char* rawValue)
			{
				auto keyName = ProcessItem(nameEntry.pItem, options);
				auto value = ProcessItem(rawValue, options);

				if (keyName && value)
				{
					return func.Invoke(*std::move(keyName), *std::move(value)).GetLastCommand();
				}
				return CallbackCommand::Discard;
			}, sectionName);
		}
		return {};
	}

	size_t INIDocument::GetSectionCount() const
	{
		if (m_Document && !m_Document->IsEmpty())
		{
			INIDocumentImpl::TNamesDepend sections;
			m_Document->GetAllSections(sections);

			return sections.size();
		}
		return 0;
	}
	bool INIDocument::HasSection(const String& sectionName) const
	{
		if (m_Document)
		{
			return m_Document->GetSection(sectionName.utf8_str()) != nullptr;
		}
		return false;
	}
	bool INIDocument::ClearSection(const String& sectionName)
	{
		if (m_Document)
		{
			return m_Document->Delete(sectionName.utf8_str(), nullptr, false);
		}
		return false;
	}
	bool INIDocument::RemoveSection(const String& sectionName)
	{
		if (m_Document)
		{
			return m_Document->Delete(sectionName.utf8_str(), nullptr, true);
		}
		return false;
	}

	// INIDocument: Attributes
	INIDocumentValue INIDocument::QuerySectionValue(const String& sectionName, const String& keyName) const
	{
		if (m_Document)
		{
			return INIDocumentValue(const_cast<INIDocument&>(*this), sectionName, keyName);
		}
		return {};
	}
	CallbackResult<void> INIDocument::EnumAttributeNames(const String& sectionName, CallbackFunction<String> func, bool uniqueOnly) const
	{
		if (m_Document)
		{
			if (uniqueOnly)
			{
				return m_Document->ForEachKey([&, options = GetOptions()](const INIDocumentImpl::Entry& entry)
				{
					if (auto keyName = ProcessItem(entry.pItem, options))
					{
						return func.Invoke(*std::move(keyName)).GetLastCommand();
					}
					return CallbackCommand::Discard;
				}, sectionName, SortOrder::Ascending);
			}
			else
			{
				return m_Document->ForEachSectionItem([&, options = GetOptions()](const INIDocumentImpl::Entry& nameEntry, const char* rawValue)
				{
					if (auto keyName = ProcessItem(nameEntry.pItem, options))
					{
						return func.Invoke(*std::move(keyName)).GetLastCommand();
					}
					return CallbackCommand::Discard;
				}, sectionName);
			}
		}
		return {};
	}
	CallbackResult<void> INIDocument::EnumAttributeValues(const String& sectionName, const String& keyName, CallbackFunction<String> func) const
	{
		if (m_Document)
		{
			return m_Document->ForEachValue([&, options = GetOptions()](const INIDocumentImpl::Entry& entry)
			{
				if (auto value = ProcessItem(entry.pItem, options))
				{
					return func.Invoke(*std::move(value)).GetLastCommand();
				}
				return CallbackCommand::Discard;
			}, sectionName, keyName, SortOrder::Ascending);
		}
		return {};
	}

	bool INIDocument::HasSectionAttribute(const String& sectionName, const String& keyName) const
	{
		if (m_Document)
		{
			return m_Document->GetValue(sectionName.utf8_str(), keyName.utf8_str()) != nullptr;
		}
		return false;
	}
	bool INIDocument::RemoveSectionAttribute(const String& sectionName, const String& keyName)
	{
		if (m_Document)
		{
			return m_Document->Delete(sectionName.utf8_str(), keyName.utf8_str(), false);
		}
		return false;
	}

	INIDocument& INIDocument::operator=(INIDocument&& other) noexcept
	{
		m_Document = std::move(other.m_Document);
		m_Options = std::move(other.m_Options);

		return *this;
	}
}
