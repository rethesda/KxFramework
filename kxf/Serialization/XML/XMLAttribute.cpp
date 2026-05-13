#include "kxf-pch.h"
#include "XMLDocument.h"
#include "TinyXML2.h"

namespace kxf
{
	// XDocument::RWValue
	std::optional<String> XMLAttribute::XDocument_QueryValue() const
	{
		if (m_Attribute)
		{
			return String::FromUTF8(m_Attribute->Value());
		}
		return {};
	}
	bool XMLAttribute::XDocument_WriteValue(const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		if (m_Attribute)
		{
			if (value.IsEmpty() && writeEmpty == WriteEmpty::Never)
			{
				return false;
			}

			m_Attribute->SetAttribute(value.utf8_str());
			return true;
		}
		return false;
	}

	// IXDocumentNode
	String XMLAttribute::GetXPath() const
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
	String XMLAttribute::GetName() const
	{
		if (m_Attribute)
		{
			return String::FromUTF8(m_Attribute->Name());
		}
		return {};
	}

	size_t XMLAttribute::GetIndexWithinParent() const
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
	size_t XMLAttribute::GetRelativeIndexWithinParent() const
	{
		return GetIndexWithinParent();
	}

	// XMLAttribute
	XMLNode XMLAttribute::GetNode() const
	{
		if (m_Owner)
		{
			return *m_Owner;
		}
		return {};
	}
	XMLDocument& XMLAttribute::GetDocument() const
	{
		return m_Owner->GetDocument();
	}

	XMLAttribute XMLAttribute::Next() const
	{
		if (m_Attribute && m_Owner)
		{
			if (auto attribute = m_Attribute->Next())
			{
				return XMLAttribute(*m_Owner, attribute);
			}
		}
		return {};
	}
}
