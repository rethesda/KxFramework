#include "kxf-pch.h"
#include "XMLDocument.h"
#include "TinyXML2.h"

namespace kxf
{
	// XDocument::RWValue
	std::optional<String> XMLDocumentAttribute::XDocument_QueryValue() const
	{
		if (m_Attribute)
		{
			return String::FromUTF8(m_Attribute->Value());
		}
		return {};
	}
	bool XMLDocumentAttribute::XDocument_WriteValue(const String& value, AsCDATA asCDATA)
	{
		if (m_Attribute)
		{
			m_Attribute->SetAttribute(value.utf8_str());
			return true;
		}
		return false;
	}

	// IXDocumentNode
	String XMLDocumentAttribute::GetXPath() const
	{
		if (m_Owner)
		{
			auto xPath = m_Owner->GetXPath();
			auto name = GetName();
			if (!xPath.IsEmpty() && !name.IsEmpty())
			{
				// Backslash for the attribute name
				return Format(kxfS("{}\\{}"), xPath, name);
			}
			return xPath;
		}
		return {};
	}
	String XMLDocumentAttribute::GetName() const
	{
		if (m_Attribute)
		{
			return String::FromUTF8(m_Attribute->Name());
		}
		return {};
	}

	size_t XMLDocumentAttribute::GetIndexWithinParent() const
	{
		if (m_Attribute && m_Owner && m_Owner->m_Node)
		{
			if (auto element = m_Owner->m_Node->ToElement())
			{
				size_t index = 0;
				for (auto attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
				{
					if (attribute == m_Attribute)
					{
						return index;
					}
					index++;
				}
			}
		}
		return npos;
	}
	size_t XMLDocumentAttribute::GetRelativeIndexWithinParent() const
	{
		return GetIndexWithinParent();
	}

	// XMLDocumentAttribute
	XMLDocumentNode XMLDocumentAttribute::GetNode() const
	{
		if (m_Owner)
		{
			return *m_Owner;
		}
		return {};
	}
	const XMLDocument& XMLDocumentAttribute::GetDocument() const
	{
		return m_Owner->GetDocument();
	}
	XMLDocument& XMLDocumentAttribute::GetDocument()
	{
		return m_Owner->GetDocument();
	}

	XMLDocumentAttribute XMLDocumentAttribute::Next() const
	{
		if (m_Attribute && m_Owner)
		{
			if (auto attribute = m_Attribute->Next())
			{
				return XMLDocumentAttribute(*m_Owner, attribute);
			}
		}
		return {};
	}
}
