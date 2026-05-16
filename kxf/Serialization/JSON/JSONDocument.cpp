#include "kxf-pch.h"
#include "JSONDocument.h"
#include "kxf/Network/URI.h"
#include "kxf/IO/IStream.h"
#include "kxf/IO/StreamReaderWriter.h"
#include "kxf/Core/ILibraryInfo.h"
#include "kxf/Utility/SoftwareLicenseDB.h"

namespace
{
	constexpr char g_Copyright[] = "Copyright© 2013-2025 Niels Lohmann";
}

namespace kxf
{
	// IObject
	RTTI::QueryInfo JSONDocument::DoQueryInterface(const IID& iid) noexcept
	{
		if (iid.IsOfType<ILibraryInfo>())
		{
			class XMLDocumentLibraryInfo final: public ILibraryInfo
			{
				public:
				// ILibraryInfo
				String GetName() const override
				{
					return "JSON for Modern C++";
				}
				Version GetVersion() const override
				{
					return {NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH};
				}
				URI GetHomePage() const override
				{
					return "https://github.com/nlohmann/json";
				}
				uint32_t GetAPILevel() const override
				{
					return NLOHMANN_JSON_VERSION_MAJOR * 1000 + NLOHMANN_JSON_VERSION_MINOR * 100 + NLOHMANN_JSON_VERSION_PATCH * 10;
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

			static XMLDocumentLibraryInfo libraryInfo;
			return static_cast<ILibraryInfo&>(libraryInfo);
		}
		return DynamicImplementation::DoQueryInterface(iid);
	}

	// IXDocument
	bool JSONDocument::IsNull() const
	{
		return m_Impl.empty();
	}
	String JSONDocument::GetDocumentMeta() const
	{
		return {};
	}

	bool JSONDocument::LoadDocument(IInputStream& stream)
	{
		if (auto size = stream.GetSize())
		{
			try
			{
				IO::InputStreamReader reader(stream);

				m_Impl = nlohmann::json::parse(reader.ReadStdString(size.ToBytes()), nullptr, false);
				return !m_Impl.empty();
			}
			catch (...)
			{
				m_Impl.clear();
			}
		}
		else
		{
			m_Impl.clear();
		}
		return false;
	}
	bool JSONDocument::SaveDocument(IOutputStream& stream) const
	{
		try
		{
			std::string string = m_Impl.dump(1, '\t');
			return stream.WriteAll(string.data(), string.length());
		}
		catch (...)
		{
			return false;
		}
	}

	// JSONDocument
	bool JSONDocument::LoadDocument(const String& json)
	{
		try
		{
			m_Impl = nlohmann::json::parse(json.ToUTF8(), nullptr, false);
			return !m_Impl.empty();
		}
		catch (...)
		{
			m_Impl.clear();
			return false;
		}
	}
	String JSONDocument::SaveDocument() const
	{
		try
		{
			return String::FromUTF8(m_Impl.dump(1, '\t'));
		}
		catch (...)
		{
			return {};
		}
	}

	void JSONDocument::ClearDocument()
	{
		m_Impl.clear();
	}
}
