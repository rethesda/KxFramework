#include "kxf-pch.h"
#include "XMLDocument.h"
#include "kxf/IO/IStream.h"
#include "kxf/Network/URI.h"
#include "kxf/Utility/SoftwareLicenseDB.h"
#include "TinyXML2.h"

namespace
{
	constexpr char g_Copyright[] = "Copyright© Lee Thomason";
	constexpr char g_DefaultDeclaredEncoding[] = "utf-8";
}

namespace kxf
{
	// IObject
	RTTI::QueryInfo XMLDocument::DoQueryInterface(const IID& iid) noexcept
	{
		if (iid.IsOfType<ILibraryInfo>())
		{
			class XMLDocumentLibraryInfo final: public ILibraryInfo
			{
				public:
					// ILibraryInfo
					String GetName() const override
					{
						return "TinyXML2";
					}
					Version GetVersion() const override
					{
						return {TIXML2_MAJOR_VERSION, TIXML2_MINOR_VERSION, TIXML2_PATCH_VERSION};
					}
					URI GetHomePage() const override
					{
						return "https://github.com/leethomason/tinyxml2";
					}
					uint32_t GetAPILevel() const override
					{
						return TIXML2_MAJOR_VERSION * 1000 + TIXML2_MINOR_VERSION * 100 + TIXML2_PATCH_VERSION * 10;
					}

					String GetLicense() const override
					{
						return SoftwareLicenseDB::Get().GetText(SoftwareLicenseType::ZLib, g_Copyright);
					}
					String GetLicenseName() const override
					{
						return SoftwareLicenseDB::Get().GetName(SoftwareLicenseType::ZLib);
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

	// XMLDocument
	void XMLDocument::ReplaceDeclaration()
	{
		if (m_Impl->FirstChild() && m_Impl->FirstChild()->ToDeclaration())
		{
			m_Impl->DeleteNode(m_Impl->FirstChild());
		}

		String declaration = Format(R"(xml version="1.0" encoding="{}")", g_DefaultDeclaredEncoding);
		m_Impl->InsertFirstChild(m_Impl->NewDeclaration(declaration.utf8_str()));
	}

	void XMLDocument::Init()
	{
		m_Impl = std::make_unique<tinyxml2::XMLDocument>();
		m_Impl->SetBOM(false);
	}
	bool XMLDocument::DoLoad(const char* xml, size_t length)
	{
		DoUnload();
		m_Impl->Parse(xml, length);

		return !m_Impl->Error();
	}
	void XMLDocument::DoUnload()
	{
		m_Impl->Clear();
	}

	XMLNode XMLDocument::CreateElement(const String& name)
	{
		return XMLNode(*this, m_Impl->NewElement(name.utf8_str()));
	}
	XMLNode XMLDocument::CreateComment(const String& value)
	{
		return XMLNode(*this, m_Impl->NewComment(value.utf8_str()));
	}
	XMLNode XMLDocument::CreateText(const String& value)
	{
		return XMLNode(*this, m_Impl->NewText(value.utf8_str()));
	}
	XMLNode XMLDocument::CreateDeclaration(const String& value)
	{
		if (!value.IsEmpty())
		{
			return XMLNode(*this, m_Impl->NewDeclaration(value.utf8_str()));
		}
		else
		{
			return XMLNode(*this, m_Impl->NewDeclaration());
		}
	}
	XMLNode XMLDocument::CreateUnknown(const String& value)
	{
		return XMLNode(*this, m_Impl->NewUnknown(value.utf8_str()));
	}

	XMLDocument::XMLDocument()
		:XMLNode(*this, m_Impl.get())
	{
		Init();
	}
	XMLDocument::XMLDocument(XMLDocument&&) noexcept = default;
	XMLDocument::~XMLDocument() = default;

	// IXDocument
	bool XMLDocument::IsNull() const
	{
		return m_Impl->Error() || !m_Impl->FirstChild();
	}
	String XMLDocument::GetDocumentMeta() const
	{
		if (auto node = m_Impl->FirstChild())
		{
			if (auto declaration = node->ToDeclaration())
			{
				return String::FromUTF8(declaration->Value());
			}
		}
		return {};
	}

	bool XMLDocument::LoadDocument(IInputStream& stream)
	{
		std::vector<std::byte> buffer;
		buffer.resize(stream.GetSize().ToBytes());
		stream.ReadAll(buffer.data(), buffer.size());
		buffer.resize(stream.LastRead().ToBytes());

		return DoLoad(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	}
	bool XMLDocument::SaveDocument(IOutputStream& stream) const
	{
		return SerializeSubtree(stream);
	}

	// XMLDocument
	bool XMLDocument::LoadDocument(const String& xml)
	{
		auto utf8 = xml.utf8_str();
		return DoLoad(utf8.data(), utf8.length());
	}
	String XMLDocument::SaveDocument() const
	{
		return SerializeSubtree();
	}

	void XMLDocument::ClearDocument()
	{
		DoUnload();
	}
	void XMLDocument::RemoveNode(XMLNode& node)
	{
		if (node)
		{
			m_Impl->DeleteNode(node.m_Node);
		}
	}

	XMLDocument& XMLDocument::operator=(XMLDocument&&) noexcept = default;
}
