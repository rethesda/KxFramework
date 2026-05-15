#include "kxf-pch.h"
#include "HTMLDocument.h"
#include "kxf/IO/IStream.h"
#include "kxf/IO/StreamReaderWriter.h"
#include "kxf/Utility/Common.h"

#pragma warning(disable: 4005) // macro redefinition
#include "gumbo.h"

namespace
{
	const GumboNode* CastGumboNode(const void* node) noexcept
	{
		return reinterpret_cast<const GumboNode*>(node);
	}
	const GumboAttribute* CastGumboAttribute(const void* node) noexcept
	{
		return reinterpret_cast<const GumboAttribute*>(node);
	}
}

namespace kxf::HTML::Private
{
	std::string gumbo_ex_cleantext(const GumboNode* node, std::string_view separator);
	std::string gumbo_ex_serialize(const GumboNode* node);

	String GetTagName(GumboTag tagType)
	{
		return gumbo_normalized_tagname(tagType);
	}
	String GetTagName(const GumboNode* node)
	{
		switch (node->type)
		{
			case GUMBO_NODE_DOCUMENT:
			{
				return GetTagName(GumboTag::GUMBO_TAG_HTML);
			}
			case GUMBO_NODE_ELEMENT:
			{
				GumboStringPiece stringPiece = node->v.element.original_tag;
				if (stringPiece.data && stringPiece.length != 0)
				{
					gumbo_tag_from_original_text(&stringPiece);
					return String::FromUTF8({stringPiece.data, stringPiece.length}).MakeLower();
				}
				return GetTagName(node->v.element.tag);
			}
		};
		return GetTagName(GumboTag::GUMBO_TAG_UNKNOWN);
	}
	bool IsFullNode(const GumboNode* node)
	{
		return node->type == GumboNodeType::GUMBO_NODE_DOCUMENT || node->type == GumboNodeType::GUMBO_NODE_ELEMENT;
	}
	
	const GumboNode* GetNodeAt(const GumboVector* nodes, size_t i)
	{
		if (i < nodes->length)
		{
			return CastGumboNode(nodes->data[i]);
		}
		return nullptr;
	}
	const GumboAttribute* GetAttribute(const GumboNode* node, const String& name)
	{
		if (node->type == GUMBO_NODE_ELEMENT)
		{
			return gumbo_get_attribute(&node->v.element.attributes, name.utf8_str());
		}
		return nullptr;
	}
	const GumboAttribute** GetAttributes(const GumboNode* node, size_t* count = nullptr)
	{
		if (node->type == GUMBO_NODE_ELEMENT)
		{
			Utility::SetIfNotNull(count, node->v.element.attributes.length);
			return const_cast<const GumboAttribute**>(reinterpret_cast<GumboAttribute**>(node->v.element.attributes.data));
		}
		else
		{
			Utility::SetIfNotNull(count, 0);
			return nullptr;
		}
	}
	size_t GetAttributeCount(const GumboNode* node)
	{
		if (node->type == GUMBO_NODE_ELEMENT)
		{
			return node->v.element.attributes.length;
		}
		return 0;
	}
	
	const GumboVector* GetChildren(const GumboNode* node)
	{
		if (node->type == GUMBO_NODE_DOCUMENT)
		{
			return &node->v.document.children;
		}
		else if (node->type == GUMBO_NODE_ELEMENT)
		{
			return &node->v.element.children;
		}
		return nullptr;
	}
	size_t GetChildrenCount(const GumboNode* node)
	{
		if (auto children = GetChildren(node))
		{
			return children->length;
		}
		return 0;
	}
	
	const GumboNode* GetElementByAttribute(const GumboNode* node, const String& name, const String& value)
	{
		auto attributes = GetAttribute(node, name);
		if (attributes && attributes->value == value)
		{
			return node;
		}
		else
		{
			size_t count = GetChildrenCount(node);
			for (size_t i = 0; i < count; i++)
			{
				if (auto children = GetChildren(node))
				{
					if (auto foundNode = GetElementByAttribute(static_cast<GumboNode*>(children->data[i]), name, value))
					{
						return foundNode;
					}
				}
			}
		}

		return nullptr;
	}
	const GumboNode* GetElementByTag(const GumboNode* node, const String& desiredTagName)
	{
		String tagName = GetTagName(node);
		if (tagName.IsSameAs(desiredTagName, StringActionFlag::IgnoreCase))
		{
			return node;
		}
		else
		{
			size_t count = GetChildrenCount(node);
			for (size_t i = 0; i < count; i++)
			{
				if (auto children = GetChildren(node))
				{
					if (auto foundNode = GetElementByTag(static_cast<GumboNode*>(children->data[i]), desiredTagName))
					{
						return foundNode;
					}
				}
			}
		}

		return nullptr;
	}
};

namespace kxf
{
	// XDocument::ROValue
	std::optional<String> HTMLDocumentAttribute::XDocument_QueryValue() const
	{
		if (auto attribute = CastGumboAttribute(m_Attribute))
		{
			return String::FromUTF8(attribute->value);
		}
		return {};
	}

	// IXDocumentNode
	String HTMLDocumentAttribute::GetXPath() const
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
	String HTMLDocumentAttribute::GetName() const
	{
		if (auto attribute = CastGumboAttribute(m_Attribute))
		{
			return String::FromUTF8(attribute->name);
		}
		return {};
	}

	size_t HTMLDocumentAttribute::GetIndexWithinParent() const
	{
		if (m_Attribute && m_Owner)
		{
			size_t attributeCount = 0;
			if (auto attributes = HTML::Private::GetAttributes(CastGumboNode(m_Owner->m_Node), &attributeCount))
			{
				for (size_t i = 0; i < attributeCount; i++)
				{
					if (attributes[i] == m_Attribute)
					{
						return i;
					}
				}
			}
		}
		return npos;
	}
	size_t HTMLDocumentAttribute::GetRelativeIndexWithinParent() const
	{
		return GetIndexWithinParent();
	}

	// HTMLDocumentAttribute
	HTMLDocumentNode HTMLDocumentAttribute::GetNode() const
	{
		if (m_Owner)
		{
			return *m_Owner;
		}
		return {};
	}
	const HTMLDocument& HTMLDocumentAttribute::GetDocument() const
	{
		return m_Owner->GetDocument();
	}
	HTMLDocument& HTMLDocumentAttribute::GetDocument()
	{
		return m_Owner->GetDocument();
	}

	HTMLDocumentAttribute HTMLDocumentAttribute::Next() const
	{
		if (m_Attribute && m_Owner)
		{
			size_t attributeCount = 0;
			if (auto attributes = HTML::Private::GetAttributes(CastGumboNode(m_Owner->m_Node), &attributeCount))
			{
				for (size_t i = 0; i < attributeCount; i++)
				{
					if (attributes[i] == m_Attribute)
					{
						if (i + 1 < attributeCount)
						{
							return HTMLDocumentAttribute(*m_Owner, attributes[i + 1]);
						}
						break;
					}
				}
			}
		}
		return {};
	}
}

namespace kxf
{
	// XDocument::ROValue
	std::optional<String> HTMLDocumentNode::XDocument_QueryValue() const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			if (HTML::Private::IsFullNode(node))
			{
				auto children = HTML::Private::GetChildren(node);
				if (children && children->length == 1)
				{
					return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, 0)).XDocument_QueryValue();
				}
			}
			else
			{
				return CastGumboNode(m_Node)->v.text.text;
			}
		}
		return {};
	}

	// XDocument::ROAttribute
	std::optional<String> HTMLDocumentNode::XDocument_QueryAttribute(const String& name) const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			auto attribute = HTML::Private::GetAttribute(node, name);
			if (attribute && attribute->value)
			{
				return attribute->value;
			}
		}
		return {};
	}

	// IXDocument
	bool HTMLDocumentNode::IsNull() const
	{
		return !m_Node || !m_Document;
	}
	String HTMLDocumentNode::GetXPath() const
	{
		return IXDocumentNode::BacktrackXPath(*m_Document, *this);
	}

	String HTMLDocumentNode::GetName() const
	{
		if (m_Node && HTML::Private::IsFullNode(CastGumboNode(m_Node)))
		{
			return HTML::Private::GetTagName(CastGumboNode(m_Node));
		}
		return {};
	}
	size_t HTMLDocumentNode::GetIndexWithinParent() const
	{
		return CastGumboNode(m_Node)->index_within_parent;
	}
	size_t HTMLDocumentNode::GetRelativeIndexWithinParent() const
	{
		return CastGumboNode(m_Node)->index_within_parent;
	}

	// HTMLDocumentNode: Navigation
	HTMLDocumentNode HTMLDocumentNode::QueryElement(const String& xPath) const
	{
		if (m_Document && m_Node)
		{
			HTMLDocumentNode currentNode = *this;
			HTMLDocumentNode previousNode = currentNode;

			UniChar indexSeparator;
			UniChar xPathSeparator = m_Document->GetXPathSeparator(&indexSeparator);
			size_t itemCount = xPath.SplitBySeparator(xPathSeparator.GetAs<XChar>(), [&](StringView name)
			{
				// Save previous element
				if (!currentNode)
				{
					return false;
				}
				previousNode = currentNode;

				// Extract index from name and remove it from path, zero-based
				// point/x -> 0, point/x:1 -> 1, point/y:0 -> 0, point/z:-7 -> 0
				auto [elementName, index] = XDocument::ExtractIndexFromElementName(name, indexSeparator.GetAs<XChar>());

				// Get level 0
				currentNode = previousNode.GetFirstChildElement(elementName);
				if (!currentNode)
				{
					return false;
				}

				// We need to go down by 'index' more elements
				for (int level = 1; level <= index; level++)
				{
					// Get next level
					currentNode = currentNode.GetNextSiblingElement(elementName);
					if (!currentNode)
					{
						return false;
					}
				}
				return true;
			});

			if (currentNode && itemCount != 0)
			{
				return currentNode;
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::QueryElementByAttribute(const String& name, const String& value) const
	{
		if (m_Document && m_Node)
		{
			return HTMLDocumentNode(*m_Document, HTML::Private::GetElementByAttribute(CastGumboNode(m_Node), name, value));
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::QueryElementByName(TagType tagType) const
	{
		if (m_Document && m_Node)
		{
			return HTMLDocumentNode(*m_Document, HTML::Private::GetElementByTag(CastGumboNode(m_Node), HTML::Private::GetTagName(static_cast<GumboTag>(tagType))));
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::QueryElementByName(const String& tagName) const
	{
		if (m_Document && m_Node)
		{
			return HTMLDocumentNode(*m_Document, HTML::Private::GetElementByTag(CastGumboNode(m_Node), tagName.ToLower()));
		}
		return {};
	}

	HTMLDocumentNode HTMLDocumentNode::GetParent() const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			return HTMLDocumentNode(*m_Document, node->parent);
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetPreviousSibling() const
	{
		if (auto node = CastGumboNode(m_Node); node && node->parent && node->index_within_parent != npos && node->index_within_parent > 0)
		{
			if (auto children = HTML::Private::GetChildren(node->parent))
			{
				return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, node->index_within_parent - 1));
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetPreviousSiblingElement(const String& name) const
	{
		if (auto node = CastGumboNode(m_Node); node && node->parent && node->index_within_parent != npos && node->index_within_parent > 0)
		{
			if (auto children = HTML::Private::GetChildren(node->parent))
			{
				size_t index = node->index_within_parent - 1;
				while (true)
				{
					auto child = HTML::Private::GetNodeAt(children, index);
					if (!child)
					{
						break;
					}
					if (HTML::Private::IsFullNode(child) && (name.IsEmpty() || HTML::Private::GetTagName(child).IsSameAs(name, StringActionFlag::IgnoreCase)))
					{
						return HTMLDocumentNode(*m_Document, child);
					}

					if (index == 0)
					{
						break;
					}
					index--;
				}
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetNextSibling() const
	{
		if (auto node = CastGumboNode(m_Node); node && node->parent && node->index_within_parent != npos)
		{
			auto childCount = HTML::Private::GetChildrenCount(node->parent);
			if (node->index_within_parent + 1 < childCount)
			{
				if (auto children = HTML::Private::GetChildren(node->parent))
				{
					return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, node->index_within_parent + 1));
				}
			}

			if (auto children = HTML::Private::GetChildren(node->parent))
			{
				return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, node->index_within_parent - 1));
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetNextSiblingElement(const String& name) const
	{
		if (auto node = CastGumboNode(m_Node); node && node->parent && node->index_within_parent != npos)
		{
			if (auto children = HTML::Private::GetChildren(node->parent))
			{
				for (size_t i = node->index_within_parent + 1; i < children->length; i++)
				{
					auto child = HTML::Private::GetNodeAt(children, i);
					if (!child)
					{
						break;
					}
					if (HTML::Private::IsFullNode(child) && (name.IsEmpty() || HTML::Private::GetTagName(child).IsSameAs(name, StringActionFlag::IgnoreCase)))
					{
						return HTMLDocumentNode(*m_Document, child);
					}
				}
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetFirstChild() const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			if (auto children = HTML::Private::GetChildren(node); children && children->length != 0)
			{
				return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, 0));
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetFirstChildElement(const String& name) const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			if (auto children = HTML::Private::GetChildren(node))
			{
				for (size_t i = 0; i < children->length; i++)
				{
					auto child = HTML::Private::GetNodeAt(children, i);
					if (!child)
					{
						break;
					}
					if (HTML::Private::IsFullNode(child) && (name.IsEmpty() || HTML::Private::GetTagName(child).IsSameAs(name, StringActionFlag::IgnoreCase)))
					{
						return HTMLDocumentNode(*m_Document, child);
					}
				}
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetLastChild() const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			if (auto children = HTML::Private::GetChildren(node); children && children->length != 0)
			{
				return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, children->length - 1));
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetLastChildElement(const String& name) const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			if (auto children = HTML::Private::GetChildren(node); children && children->length != 0)
			{
				size_t index = children->length - 1;
				while (true)
				{
					auto child = HTML::Private::GetNodeAt(children, index);
					if (!child)
					{
						break;
					}
					if (HTML::Private::IsFullNode(child) && (name.IsEmpty() || HTML::Private::GetTagName(child).IsSameAs(name, StringActionFlag::IgnoreCase)))
					{
						return HTMLDocumentNode(*m_Document, child);
					}

					if (index == 0)
					{
						break;
					}
					index--;
				}
			}
		}
		return {};
	}

	// HTMLDocumentNode: Children
	size_t HTMLDocumentNode::GetChildrenCount() const
	{
		if (m_Node)
		{
			return HTML::Private::GetChildrenCount(CastGumboNode(m_Node));
		}
		return 0;
	}
	CallbackResult<void> HTMLDocumentNode::EnumChildren(CallbackFunction<HTMLDocumentNode> func) const
	{
		if (m_Node)
		{
			if (auto children = HTML::Private::GetChildren(CastGumboNode(m_Node)))
			{
				for (size_t i = 0; i < children->length; i++)
				{
					if (func.Invoke(HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, i))).ShouldTerminate())
					{
						break;
					}
				}
				return func.Finalize();
			}
		}
		return {};
	}

	// HTMLDocumentNode: Attributes
	size_t HTMLDocumentNode::GetAttributeCount() const
	{
		if (m_Node)
		{
			return HTML::Private::GetAttributeCount(CastGumboNode(m_Node));
		}
		return 0;
	}
	bool HTMLDocumentNode::HasAttribute(const String& name) const
	{
		if (m_Node)
		{
			return HTML::Private::GetAttribute(CastGumboNode(m_Node), name);
		}
		return false;
	}

	HTMLDocumentAttribute HTMLDocumentNode::GetAttributeObject(const String& name) const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			return HTMLDocumentAttribute(*this, HTML::Private::GetAttribute(node, name));
		}
		return {};
	}
	CallbackResult<void> HTMLDocumentNode::EnumAttributeNames(CallbackFunction<String> func) const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			size_t attributeCount = 0;
			if (auto attributes = HTML::Private::GetAttributes(node, &attributeCount))
			{
				for (size_t i = 0; i < attributeCount; i++)
				{
					if (func.Invoke(String::FromUTF8(attributes[i]->name)).ShouldTerminate())
					{
						break;
					}
				}
				return func.Finalize();
			}
		}
		return {};
	}
	CallbackResult<void> HTMLDocumentNode::EnumAttributes(CallbackFunction<HTMLDocumentAttribute> func) const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			size_t attributeCount = 0;
			if (auto attributes = HTML::Private::GetAttributes(node, &attributeCount))
			{
				for (size_t i = 0; i < attributeCount; i++)
				{
					if (func.Invoke(HTMLDocumentAttribute(*this, attributes[i])).ShouldTerminate())
					{
						break;
					}
				}
				return func.Finalize();
			}
		}
		return {};
	}

	// HTMLDocumentNode: Properties
	HTML::NodeType HTMLDocumentNode::GetType() const
	{
		if (m_Node)
		{
			return static_cast<NodeType>(CastGumboNode(m_Node)->type);
		}
		return NodeType::None;
	}
	HTML::TagType HTMLDocumentNode::GetTagType() const
	{
		if (m_Node)
		{
			switch (CastGumboNode(m_Node)->type)
			{
				case GUMBO_NODE_DOCUMENT:
				{
					return TagType::HTML;
				}
				case GUMBO_NODE_ELEMENT:
				{
					return static_cast<TagType>(CastGumboNode(m_Node)->v.element.tag);
				}
			};
		}
		return TagType::UNKNOWN;
	}
	bool HTMLDocumentNode::IsElement() const
	{
		return GetType() == NodeType::Element;
	}
	bool HTMLDocumentNode::IsText() const
	{
		return GetType() == NodeType::Text;
	}

	// HTMLDocumentNode: Serialization
	bool HTMLDocumentNode::SerializeSubtree(IOutputStream& stream) const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			if (HTML::Private::IsFullNode(CastGumboNode(m_Node)))
			{
				auto buffer = HTML::Private::gumbo_ex_serialize(const_cast<GumboNode*>(CastGumboNode(m_Node)));
				return stream.WriteAll(buffer.data(), buffer.size());
			}
			else
			{
				auto buffer = XDocument_QueryValue().value_or(String());

				IO::OutputStreamWriter writer(stream);
				return writer.WriteStringUTF8(buffer);
			}
		}
		return {};
	}
	String HTMLDocumentNode::SerializeSubtree() const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			if (HTML::Private::IsFullNode(node))
			{
				return String::FromUTF8(HTML::Private::gumbo_ex_serialize(node));
			}
			else
			{
				return XDocument_QueryValue().value_or(String());
			}
		}
		return {};
	}
	String HTMLDocumentNode::SerializeSubtreeText(const String& separator) const
	{
		if (auto node = CastGumboNode(m_Node))
		{
			if (node->type == GUMBO_NODE_TEXT)
			{
				return String::FromUTF8(node->v.text.text);
			}
			else
			{
				auto sep = separator.utf8_str();
				return String::FromUTF8(HTML::Private::gumbo_ex_cleantext(node, sep));
			}
		}
		return {};
	}
}
