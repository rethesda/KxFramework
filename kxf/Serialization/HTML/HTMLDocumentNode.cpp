#include "kxf-pch.h"
#include "HTMLDocument.h"
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
	std::string gumbo_ex_cleantext(GumboNode* node);
	std::string gumbo_ex_serialize(GumboNode* node);

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
			auto utf8 = name.ToUTF8();
			return gumbo_get_attribute(&node->v.element.attributes, utf8.data());
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
	size_t GetAttributesCount(const GumboNode* node)
	{
		if (node->type == GUMBO_NODE_ELEMENT)
		{
			return node->v.element.attributes.length;
		}
		return 0;
	}
	
	const GumboVector* GetChildren(const GumboNode* node)
	{
		const GumboVector* children = nullptr;
		if (node->type == GUMBO_NODE_DOCUMENT)
		{
			children = &node->v.document.children;
		}
		else if (node->type == GUMBO_NODE_ELEMENT)
		{
			children = &node->v.element.children;
		}
		return children;
	}
	size_t GetChildrenCount(const GumboNode* node)
	{
		const GumboVector* children = GetChildren(node);
		if (children)
		{
			return children->length;
		}
		return 0;
	}
	
	const GumboNode* GetElementByAttribute(const GumboNode* node, const String& name, const String& value)
	{
		const GumboAttribute* attributes = GetAttribute(node, name);
		if (attributes && attributes->value == value)
		{
			return node;
		}
		else
		{
			size_t count = GetChildrenCount(node);
			for (size_t i = 0; i < count; i++)
			{
				const GumboVector* children = GetChildren(node);
				if (children)
				{
					const GumboNode* foundNode = GetElementByAttribute((GumboNode*)children->data[i], name, value);
					if (foundNode)
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
				const GumboVector* children = GetChildren(node);
				if (children)
				{
					const GumboNode* foundNode = GetElementByTag((GumboNode*)children->data[i], desiredTagName);
					if (foundNode)
					{
						return foundNode;
					}
				}
			}
		}

		return nullptr;
	}
	const GumboNode* GetParent(const GumboNode* node)
	{
		return node->parent;
	}
};

namespace kxf
{
	// XDocument::ROValue
	std::optional<String> HTMLDocumentNode::XDocument_QueryValue() const
	{
		auto node = GetNode();
		if (node && HTML::Private::IsFullNode(CastGumboNode(node)))
		{
			auto children = HTML::Private::GetChildren(CastGumboNode(node));
			if (children && children->length == 1)
			{
				return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, 0)).XDocument_QueryValue();
			}
		}
		else if (node)
		{
			return CastGumboNode(node)->v.text.text;
		}
		return {};
	}

	// XDocument::ROAttribute
	std::optional<String> HTMLDocumentNode::XDocument_QueryAttribute(const String& name) const
	{
		if (auto node = GetNode())
		{
			auto attribute = HTML::Private::GetAttribute(CastGumboNode(node), name);
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
		auto node = GetNode();
		if (node && HTML::Private::IsFullNode(CastGumboNode(node)))
		{
			return HTML::Private::GetTagName(CastGumboNode(node));
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

	// HTMLDocumentNode
	bool HTMLDocumentNode::IsFullNode() const
	{
		return HTML::Private::IsFullNode(CastGumboNode(GetNode()));
	}
	HTML::NodeType HTMLDocumentNode::GetType() const
	{
		if (auto node = GetNode())
		{
			return static_cast<NodeType>(CastGumboNode(node)->type);
		}
		return NodeType::None;
	}
	HTML::TagType HTMLDocumentNode::GetTagType() const
	{
		if (auto node = GetNode())
		{
			switch (CastGumboNode(node)->type)
			{
				case GUMBO_NODE_DOCUMENT:
				{
					return TagType::HTML;
				}
				case GUMBO_NODE_ELEMENT:
				{
					return static_cast<TagType>(CastGumboNode(node)->v.element.tag);
				}
			};
		}
		return TagType::UNKNOWN;
	}
	String HTMLDocumentNode::GetValueText() const
	{
		auto node = GetNode();
		if (node)
		{
			if (CastGumboNode(node)->type == GUMBO_NODE_TEXT)
			{
				return String::FromUTF8(CastGumboNode(node)->v.text.text);
			}
			else
			{
				return String::FromUTF8(HTML::Private::gumbo_ex_cleantext(const_cast<GumboNode*>(CastGumboNode(node))));
			}
		}
		return {};
	}
	String HTMLDocumentNode::GetHTML() const
	{
		if (auto node = GetNode())
		{
			if (HTML::Private::IsFullNode(CastGumboNode(node)))
			{
				return String::FromUTF8(HTML::Private::gumbo_ex_serialize(const_cast<GumboNode*>(CastGumboNode(node))));
			}
			else
			{
				return XDocument_QueryValue().value_or(String());
			}
		}
		return {};
	}

	// HTMLDocumentNode: Children

	size_t HTMLDocumentNode::GetChildrenCount() const
	{
		return HTML::Private::GetChildrenCount(CastGumboNode(GetNode()));
	}
	CallbackResult<void> HTMLDocumentNode::EnumChildren(CallbackFunction<HTMLDocumentNode> func) const
	{
		if (auto node = GetNode())
		{
			if (auto children = HTML::Private::GetChildren(CastGumboNode(node)))
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
		return HTML::Private::GetAttributesCount(CastGumboNode(GetNode()));
	}
	bool HTMLDocumentNode::HasAttribute(const String& name) const
	{
		if (auto node = GetNode())
		{
			return HTML::Private::GetAttribute(CastGumboNode(node), name);
		}
		return false;
	}
	CallbackResult<void> HTMLDocumentNode::EnumAttributeNames(CallbackFunction<String> func) const
	{
		if (auto node = GetNode())
		{
			size_t attributeCount = 0;
			if (const GumboAttribute** attributes = HTML::Private::GetAttributes(CastGumboNode(node), &attributeCount))
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

	// HTMLDocumentNode: Navigation
	HTMLDocumentNode HTMLDocumentNode::QueryElement(const String& XPath) const
	{
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::QueryElementByAttribute(const String& name, const String& value) const
	{
		if (m_Document)
		{
			return HTMLDocumentNode(*m_Document, HTML::Private::GetElementByAttribute(CastGumboNode(GetNode()), name, value));
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::QueryElementByName(TagType tagType) const
	{
		if (m_Document)
		{
			return HTMLDocumentNode(*m_Document, HTML::Private::GetElementByTag(CastGumboNode(GetNode()), HTML::Private::GetTagName(static_cast<GumboTag>(tagType))));
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::QueryElementByName(const String& tagName) const
	{
		if (m_Document)
		{
			return HTMLDocumentNode(*m_Document, HTML::Private::GetElementByTag(CastGumboNode(GetNode()), tagName.ToLower()));
		}
		return {};
	}

	HTMLDocumentNode HTMLDocumentNode::GetParent() const
	{
		if (auto node = GetNode())
		{
			return HTMLDocumentNode(*m_Document, HTML::Private::GetParent(CastGumboNode(node)));
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetPreviousSibling() const
	{
		auto node = GetNode();
		if (node && CastGumboNode(node)->parent && CastGumboNode(node)->index_within_parent != -1 && CastGumboNode(node)->index_within_parent > 0)
		{
			if (const GumboVector* children = HTML::Private::GetChildren(CastGumboNode(node)->parent))
			{
				return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, CastGumboNode(node)->index_within_parent - 1));
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetNextSibling() const
	{
		auto node = GetNode();
		if (node && CastGumboNode(node)->parent)
		{
			const size_t max = HTML::Private::GetChildrenCount(CastGumboNode(node)->parent);
			if (CastGumboNode(node)->index_within_parent != -1 && CastGumboNode(node)->index_within_parent < max)
			{
				if (const GumboVector* children = HTML::Private::GetChildren(CastGumboNode(node)->parent))
				{
					const GumboNode* sibling = HTML::Private::GetNodeAt(children, CastGumboNode(node)->index_within_parent + 1);
					return HTMLDocumentNode(*m_Document, sibling);
				}
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetFirstChild() const
	{
		if (auto node = GetNode())
		{
			auto children = HTML::Private::GetChildren(CastGumboNode(node));
			if (children && children->length != 0)
			{
				return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, 0));
			}
		}
		return {};
	}
	HTMLDocumentNode HTMLDocumentNode::GetLastChild() const
	{
		if (auto node = GetNode())
		{
			auto children = HTML::Private::GetChildren(CastGumboNode(node));
			if (children && children->length != 0)
			{
				return HTMLDocumentNode(*m_Document, HTML::Private::GetNodeAt(children, children->length - 1));
			}
		}
		return {};
	}
}
