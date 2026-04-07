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
	// XDocument::RWValue
	std::optional<String> HTMLNode::XDocument_QueryValue() const
	{
		auto node = GetNode();
		if (node && HTML::Private::IsFullNode(CastGumboNode(node)))
		{
			auto children = HTML::Private::GetChildren(CastGumboNode(node));
			if (children && children->length == 1)
			{
				return HTMLNode(*m_Document, HTML::Private::GetNodeAt(children, 0)).XDocument_QueryValue();
			}
		}
		else if (node)
		{
			return CastGumboNode(node)->v.text.text;
		}
		return {};
	}
	bool HTMLNode::XDocument_WriteValue(const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		return false;
	}

	// XDocument::RWAttribute
	std::optional<String> HTMLNode::XDocument_QueryAttribute(const String& name) const
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
	bool HTMLNode::XDocument_WriteAttribute(const String& name, const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		return false;
	}

	// IXDocument
	bool HTMLNode::IsNull() const
	{
		return !m_Node || !m_Document;
	}
	String HTMLNode::GetXPath() const
	{
		return IXDocumentNode::BacktrackXPath(*m_Document, *this);
	}

	String HTMLNode::GetName() const
	{
		auto node = GetNode();
		if (node && HTML::Private::IsFullNode(CastGumboNode(node)))
		{
			return HTML::Private::GetTagName(CastGumboNode(node));
		}
		return {};
	}
	size_t HTMLNode::GetIndexWithinParent() const
	{
		return CastGumboNode(m_Node)->index_within_parent;
	}
	size_t HTMLNode::GetRelativeIndexWithinParent() const
	{
		return CastGumboNode(m_Node)->index_within_parent;
	}

	// HTMLNode

	bool HTMLNode::IsFullNode() const
	{
		return HTML::Private::IsFullNode(CastGumboNode(GetNode()));
	}
	HTML::NodeType HTMLNode::GetType() const
	{
		if (auto node = GetNode())
		{
			return static_cast<NodeType>(CastGumboNode(node)->type);
		}
		return NodeType::None;
	}
	HTML::TagType HTMLNode::GetTagType() const
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
	String HTMLNode::GetValueText() const
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
	String HTMLNode::GetHTML() const
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

	// HTMLNode: Children

	size_t HTMLNode::GetChildrenCount() const
	{
		return HTML::Private::GetChildrenCount(CastGumboNode(GetNode()));
	}
	CallbackResult<void> HTMLNode::EnumChildren(CallbackFunction<HTMLNode> func) const
	{
		if (auto node = GetNode())
		{
			if (auto children = HTML::Private::GetChildren(CastGumboNode(node)))
			{
				for (size_t i = 0; i < children->length; i++)
				{
					if (func.Invoke(HTMLNode(*m_Document, HTML::Private::GetNodeAt(children, i))).ShouldTerminate())
					{
						break;
					}
				}
				return func.Finalize();
			}
		}
		return {};
	}

	// HTMLNode: Attributes
	size_t HTMLNode::GetAttributeCount() const
	{
		return HTML::Private::GetAttributesCount(CastGumboNode(GetNode()));
	}
	bool HTMLNode::HasAttribute(const String& name) const
	{
		if (auto node = GetNode())
		{
			return HTML::Private::GetAttribute(CastGumboNode(node), name);
		}
		return false;
	}
	CallbackResult<void> HTMLNode::EnumAttributeNames(CallbackFunction<String> func) const
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

	// HTMLNode: Navigation
	HTMLNode HTMLNode::QueryElement(const String& XPath) const
	{
		return {};
	}

	HTMLNode HTMLNode::GetElementByAttribute(const String& name, const String& value) const
	{
		if (m_Document)
		{
			return HTMLNode(*m_Document, HTML::Private::GetElementByAttribute(CastGumboNode(GetNode()), name, value));
		}
		return {};
	}
	HTMLNode HTMLNode::GetElementByTag(TagType tagType) const
	{
		if (m_Document)
		{
			return HTMLNode(*m_Document, HTML::Private::GetElementByTag(CastGumboNode(GetNode()), HTML::Private::GetTagName(static_cast<GumboTag>(tagType))));
		}
		return {};
	}
	HTMLNode HTMLNode::GetElementByTag(const String& tagName) const
	{
		if (m_Document)
		{
			return HTMLNode(*m_Document, HTML::Private::GetElementByTag(CastGumboNode(GetNode()), tagName.ToLower()));
		}
		return {};
	}

	HTMLNode HTMLNode::GetParent() const
	{
		if (auto node = GetNode())
		{
			return HTMLNode(*m_Document, HTML::Private::GetParent(CastGumboNode(node)));
		}
		return {};
	}
	HTMLNode HTMLNode::GetPreviousSibling() const
	{
		auto node = GetNode();
		if (node && CastGumboNode(node)->parent && CastGumboNode(node)->index_within_parent != -1 && CastGumboNode(node)->index_within_parent > 0)
		{
			if (const GumboVector* children = HTML::Private::GetChildren(CastGumboNode(node)->parent))
			{
				return HTMLNode(*m_Document, HTML::Private::GetNodeAt(children, CastGumboNode(node)->index_within_parent - 1));
			}
		}
		return {};
	}
	HTMLNode HTMLNode::GetNextSibling() const
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
					return HTMLNode(*m_Document, sibling);
				}
			}
		}
		return {};
	}
	HTMLNode HTMLNode::GetFirstChild() const
	{
		if (auto node = GetNode())
		{
			auto children = HTML::Private::GetChildren(CastGumboNode(node));
			if (children && children->length != 0)
			{
				return HTMLNode(*m_Document, HTML::Private::GetNodeAt(children, 0));
			}
		}
		return {};
	}
	HTMLNode HTMLNode::GetLastChild() const
	{
		if (auto node = GetNode())
		{
			auto children = HTML::Private::GetChildren(CastGumboNode(node));
			if (children && children->length != 0)
			{
				return HTMLNode(*m_Document, HTML::Private::GetNodeAt(children, children->length - 1));
			}
		}
		return {};
	}
}
