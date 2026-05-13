#include "kxf-pch.h"
#include "XMLDocument.h"
#include "kxf/IO/IStream.h"
#include "kxf/Core/IEncodingConverter.h"
#include "TinyXML2.h"

namespace
{
	class XMLPrinterDefault: public tinyxml2::XMLPrinter
	{
		public:
			XMLPrinterDefault(FILE* file = nullptr, bool compact = false, int depth = 0)
				:XMLPrinter(file, compact, depth)
			{
			}

		protected:
			// tinyxml2::XMLPrinter
			void PrintSpace(int depth) override
			{
				for (int i = 0; i < depth; i++)
				{
					Write("\t");
				}
			}
	};

	class XMLPrinterHTML5: public XMLPrinterDefault
	{
		private:
			bool m_IsCompactMode = false;

		private:
			bool IsVoidElement(const char* name) const
			{
				// Complete list of all HTML5 "void elements": http://dev.w3.org/html5/markup/syntax.html
				constexpr const char* voidElementNames[] =
				{
					"area",
					"base",
					"br",
					"col",
					"command",
					"embed",
					"hr",
					"img",
					"input",
					"keygen",
					"link",
					"meta",
					"param",
					"source",
					"track",
					"wbr"
				};

				auto it = std::ranges::find_if(voidElementNames, [&](const char* item)
				{
					return _stricmp(item, name) == 0;
				});
				return it != std::end(voidElementNames);
			}

		public:
			XMLPrinterHTML5(FILE* file = nullptr, bool compact = false, int depth = 0)
				:XMLPrinterDefault(file, compact, depth), m_IsCompactMode(compact)
			{
			}

		protected:
			// tinyxml2::XMLPrinter
			void CloseElement(bool compactMode = false) override
			{
				if (_elementJustOpened && !IsVoidElement(_stack.PeekTop()))
				{
					SealElementIfJustOpened();
				}
				XMLPrinterDefault::CloseElement(m_IsCompactMode);
			}
	};
}
namespace
{
	bool ContainsForbiddenCharactersForValueData(const kxf::String& value)
	{
		for (kxf::XChar c: value)
		{
			if (c == '&' || c == '<' || c == '>' || c == '\"' || c == '\'')
			{
				return true;
			}
		}
		return false;
	}
	std::pair<kxf::StringView, int> ExtractIndexFromElementName(kxf::StringView elementName, kxf::XChar xPathSeparator)
	{
		int index = 0;

		size_t indexStart = elementName.find(xPathSeparator);
		if (indexStart != kxf::String::npos)
		{
			if (auto value = kxf::String(elementName.substr(indexStart + 1)).ParseInteger<int>(10))
			{
				index = std::clamp(*value, 0, std::numeric_limits<int>::max());
				elementName = elementName.substr(0, indexStart);
			}
		}
		return {elementName, index};
	}

	kxf::String CleanText(const tinyxml2::XMLNode& node, kxf::StringView separator = {})
	{
		using namespace kxf;

		String content;
		if (node.ToElement())
		{
			for (auto child = node.FirstChild(); child; child = child->NextSibling())
			{
				String text = CleanText(*child);
				if (!separator.empty() && child != &node && !text.empty())
				{
					content += separator;
				}
				content += text;
			}
		}
		else if (!node.ToDocument())
		{
			content = String::FromUTF8(node.Value());
		}
		return content;
	}

	template<std::derived_from<tinyxml2::XMLPrinter> TPrinter>
	kxf::String PrintDocumentUsing(const tinyxml2::XMLDocument& document)
	{
		TPrinter buffer;
		document.Print(&buffer);

		return kxf::String::FromUTF8({buffer.CStr(), buffer.CStrSize() - 1});
	}
}

namespace kxf
{
	// XDocument::RWValue
	std::optional<String> XMLNode::XDocument_QueryValue() const
	{
		if (m_Node)
		{
			if (auto text = m_Node->ToText())
			{
				return String::FromUTF8(text->Value());
			}
			else if (auto element = m_Node->ToElement())
			{
				if (auto text = element->GetText())
				{
					String::FromUTF8(text);
				}
			}
			else if (auto value = m_Node->Value())
			{
				return String::FromUTF8(value);
			}
		}
		return {};
	}
	bool XMLNode::XDocument_WriteValue(const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		if (value.IsEmpty() && writeEmpty == WriteEmpty::Never)
		{
			return false;
		}

		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				tinyxml2::XMLText* textNode = nullptr;
				if (auto firstChild = element->FirstChild())
				{
					textNode = firstChild->ToText();
					if (textNode)
					{
						textNode->SetValue(value.utf8_str());
					}
				}
				if (!textNode)
				{
					textNode = m_Document->m_Impl->NewText(value.utf8_str());
					m_Node->DeleteChildren();
					m_Node->InsertFirstChild(textNode);
				}

				switch (asCDATA)
				{
					case AsCDATA::Always:
					{
						textNode->SetCData(true);
						break;
					}
					case AsCDATA::Never:
					{
						textNode->SetCData(false);
						break;
					}
					default:
					{
						textNode->SetCData(ContainsForbiddenCharactersForValueData(value));
						break;
					}
				};
				return true;
			}
			else if (m_Node->ToText() || m_Node->ToComment() || m_Node->ToDeclaration() || m_Node->ToUnknown())
			{
				m_Node->SetValue(value.utf8_str());
				return true;
			}
		}
		return false;
	}

	// XDocument::RWAttribute
	std::optional<String> XMLNode::XDocument_QueryAttribute(const String& name) const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				if (auto value = element->Attribute(name.utf8_str()))
				{
					return String::FromUTF8(value);
				}
			}
		}
		return {};
	}
	bool XMLNode::XDocument_WriteAttribute(const String& name, const String& value, WriteEmpty writeEmpty, AsCDATA asCDATA)
	{
		if (value.IsEmpty() && writeEmpty == WriteEmpty::Never)
		{
			return false;
		}

		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				element->SetAttribute(name.utf8_str(), value.utf8_str());
				return true;
			}
		}
		return false;
	}

	XMLNode XMLNode::ConstructOrQueryElement(const String& xPath, bool allowCreate)
	{
		if (m_Document && m_Node)
		{
			tinyxml2::XMLDocument& document = *m_Document->m_Impl;
			tinyxml2::XMLNode* currentNode = m_Node;
			tinyxml2::XMLNode* previousNode = currentNode;

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
				auto [elementName, index] = ExtractIndexFromElementName(name, indexSeparator.GetAs<XChar>());
				auto elementNameUTF8 = EncodingConverter_UTF8.ToMultiByte(name);

				// Get level 0
				currentNode = previousNode->FirstChildElement(elementNameUTF8.data());
				if (!currentNode)
				{
					if (allowCreate)
					{
						currentNode = document.NewElement(elementNameUTF8.data());
						previousNode->InsertEndChild(currentNode);
					}
					else
					{
						return false;
					}
				}

				// We need to go down by 'index' more elements
				for (int level = 1; level <= index; level++)
				{
					// Get next level
					currentNode = currentNode->NextSiblingElement(elementNameUTF8.data());
					if (!currentNode)
					{
						if (allowCreate)
						{
							currentNode = document.NewElement(elementNameUTF8.data());
							previousNode->InsertAfterChild(previousNode, currentNode);
						}
						else
						{
							return false;
						}
					}
				}
				return true;
			});

			if (currentNode && itemCount != 0)
			{
				return XMLNode(*m_Document, currentNode);
			}
		}
		return {};
	}

	// IXDocumentNode
	String XMLNode::GetXPath() const
	{
		return IXDocumentNode::BacktrackXPath(*m_Document, *this);
	}

	String XMLNode::GetName() const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				return String::FromUTF8(element->Name());
			}
		}
		return {};
	}
	size_t XMLNode::GetIndexWithinParent() const
	{
		if (m_Node)
		{
			if (auto parent = m_Node->Parent())
			{
				size_t index = 0;
				for (auto child = parent->FirstChild(); child; child = child->NextSibling())
				{
					if (child == m_Node)
					{
						break;
					}
					index++;
				}
				return index;
			}
			else
			{
				return 0;
			}
		}
		return npos;
	}
	size_t XMLNode::GetRelativeIndexWithinParent() const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				if (auto parent = m_Node->Parent())
				{
					auto name = element->Name();

					size_t index = 0;
					for (auto childElement = parent->FirstChildElement(name); childElement; childElement = childElement->NextSiblingElement(name))
					{
						if (childElement == element)
						{
							break;
						}
						index++;
					}
					return index;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return GetIndexWithinParent();
			}
		}
		return npos;
	}

	// XMLNode: Common
	bool XMLNode::SetName(const String& name)
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				element->SetName(name.utf8_str());
				return true;
			}
		}
		return false;
	}

	// XMLNode: Navigation
	XMLNode XMLNode::QueryElement(const String& xPath) const
	{
		return const_cast<XMLNode&>(*this).ConstructOrQueryElement(xPath, false);
	}
	XMLNode XMLNode::ConstructElement(const String& xPath)
	{
		return const_cast<XMLNode&>(*this).ConstructOrQueryElement(xPath, true);
	}
	XMLNode XMLNode::QueryElementByAttribute(const String& name, const String& value) const
	{
		if (m_Node)
		{
			if (GetAttribute(name) == value)
			{
				return XMLNode(*m_Document, m_Node);
			}
			else
			{
				for (auto child = GetFirstChildElement(); child; child = child.GetNextSiblingElement())
				{
					if (auto foundElement = child.QueryElementByAttribute(name, value))
					{
						return foundElement;
					}
				}
			}
		}
		return {};
	}
	XMLNode XMLNode::QueryElementByName(const String& name) const
	{
		if (m_Node)
		{
			if (GetName() == name)
			{
				return XMLNode(*m_Document, m_Node);
			}
			else
			{
				for (auto child = GetFirstChildElement(); child; child = child.GetNextSiblingElement())
				{
					if (auto foundElement = child.QueryElementByName(name))
					{
						return foundElement;
					}
				}
			}
		}
		return {};
	}

	XMLNode XMLNode::GetParent() const
	{
		if (m_Node)
		{
			return XMLNode(*m_Document, m_Node->Parent());
		}
		return {};
	}
	XMLNode XMLNode::GetPreviousSibling() const
	{
		if (m_Node)
		{
			return XMLNode(*m_Document, m_Node->PreviousSibling());
		}
		return {};
	}
	XMLNode XMLNode::GetPreviousSiblingElement(const String& name) const
	{
		if (m_Node)
		{
			if (name.IsEmpty())
			{
				return XMLNode(*m_Document, m_Node->PreviousSiblingElement());
			}
			else
			{
				return XMLNode(*m_Document, m_Node->PreviousSiblingElement(name.utf8_str()));
			}
		}
		return {};
	}
	XMLNode XMLNode::GetNextSibling() const
	{
		if (m_Node)
		{
			return XMLNode(*m_Document, m_Node->NextSibling());
		}
		return {};
	}
	XMLNode XMLNode::GetNextSiblingElement(const String& name) const
	{
		if (m_Node)
		{
			if (name.IsEmpty())
			{
				return XMLNode(*m_Document, m_Node->NextSiblingElement());
			}
			else
			{
				return XMLNode(*m_Document, m_Node->NextSiblingElement(name.utf8_str()));
			}
		}
		return {};
	}
	XMLNode XMLNode::GetFirstChild() const
	{
		if (m_Node)
		{
			return XMLNode(*m_Document, m_Node->FirstChild());
		}
		return {};
	}
	XMLNode XMLNode::GetFirstChildElement(const String& name) const
	{
		if (m_Node)
		{
			if (name.IsEmpty())
			{
				return XMLNode(*m_Document, m_Node->FirstChildElement());
			}
			else
			{
				return XMLNode(*m_Document, m_Node->FirstChildElement(name.utf8_str()));
			}
		}
		return {};
	}
	XMLNode XMLNode::GetLastChild() const
	{
		if (m_Node)
		{
			return XMLNode(*m_Document, m_Node->LastChild());
		}
		return {};
	}
	XMLNode XMLNode::GetLastChildElement(const String& name) const
	{
		if (m_Node)
		{
			if (name.IsEmpty())
			{
				return XMLNode(*m_Document, m_Node->LastChildElement());
			}
			else
			{
				return XMLNode(*m_Document, m_Node->LastChildElement(name.utf8_str()));
			}
		}
		return {};
	}

	// XMLNode: Children
	size_t XMLNode::GetChildrenCount() const
	{
		if (m_Node)
		{
			size_t count = 0;
			for (auto child = m_Node->FirstChild(); child ; child = child->NextSibling())
			{
				count++;
			}
			return count;
		}
		return 0;
	}
	bool XMLNode::HasChildren() const
	{
		if (m_Node)
		{
			return !m_Node->NoChildren();
		}
		return false;
	}
	void XMLNode::ClearChildren()
	{
		if (m_Node)
		{
			m_Node->DeleteChildren();
		}
	}
	void XMLNode::ResetNode()
	{
		ClearChildren();
		ClearAttributes();
	}

	CallbackResult<void> XMLNode::EnumChildren(CallbackFunction<XMLNode> func) const
	{
		if (m_Node)
		{
			for (auto child = m_Node->FirstChild(); child; child = child->NextSibling())
			{
				if (func.Invoke(XMLNode(*m_Document, child)).ShouldTerminate())
				{
					break;
				}
			}

			return func.Finalize();
		}
		return {};
	}
	CallbackResult<void> XMLNode::EnumChildElements(CallbackFunction<XMLNode> func, const String& name) const
	{
		if (m_Node)
		{
			auto nameUTF8 = name.utf8_str();
			for (auto child = m_Node->FirstChildElement(nameUTF8.data_if_not_empty()); child; child = child->NextSiblingElement(nameUTF8.data_if_not_empty()))
			{
				if (func.Invoke(XMLNode(*m_Document, child)).ShouldTerminate())
				{
					break;
				}
			}

			return func.Finalize();
		}
		return {};
	}

	// XMLNode: Attributes
	size_t XMLNode::GetAttributeCount() const
	{
		if (m_Node)
		{
			size_t count = 0;
			if (auto element = m_Node->ToElement())
			{
				for (auto attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
				{
					count++;
				}
			}
			return count;
		}
		return 0;
	}
	bool XMLNode::HasAttributes() const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				return element->FirstAttribute();
			}
		}
		return false;
	}

	XMLAttribute XMLNode::GetAttributeObject(const String& name) const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				return XMLAttribute(*m_Document, element->FindAttribute(name.utf8_str()));
			}
		}
		return {};
	}
	CallbackResult<void> XMLNode::EnumAttributeNames(CallbackFunction<String> func) const
	{
		return EnumAttributes([&](XMLAttribute attribute)
		{
			return func.Invoke(attribute.GetName()).GetLastCommand();
		});
	}
	CallbackResult<void> XMLNode::EnumAttributes(CallbackFunction<XMLAttribute> func) const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				for (auto attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
				{
					if (func.Invoke(XMLAttribute(*this, attribute)).ShouldTerminate())
					{
						break;
					}
				}
				return func.Finalize();
			}
		}
		return {};
	}

	bool XMLNode::HasAttribute(const String& name) const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				return element->Attribute(name.utf8_str());
			}
		}
		return false;
	}
	bool XMLNode::RemoveAttribute(const String& name)
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				element->DeleteAttribute(name.utf8_str());
				return true;
			}
		}
		return false;
	}
	bool XMLNode::RemoveAttribute(XMLAttribute& attribute)
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				element->DeleteAttribute(attribute.m_Attribute->Name());
				return true;
			}
		}
		return false;
	}
	bool XMLNode::ClearAttributes()
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				for (auto attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
				{
					element->DeleteAttribute(attribute->Name());
				}
				return true;
			}
		}
		return false;
	}

	// Insertion
	bool XMLNode::Insert(XMLNode& node, InsertMode insertMode)
	{
		switch (insertMode)
		{
			case InsertMode::AfterChild:
			{
				return InsertAfterChild(node);
			}
			case InsertMode::AsFirstChild:
			{
				return InsertFirstChild(node);
			}
			case InsertMode::AsLastChild:
			{
				return InsertLastChild(node);
			}
		};
		return false;
	}
	bool XMLNode::InsertAfterChild(XMLNode& newNode, const XMLNode& afterThis)
	{
		if (m_Node && newNode)
		{
			if (afterThis)
			{
				return m_Node->InsertAfterChild(afterThis.m_Node, newNode.m_Node);
			}
			else
			{
				return m_Node->InsertAfterChild(m_Node, newNode.m_Node);
			}
		}
		return false;
	}
	bool XMLNode::InsertFirstChild(XMLNode& newNode)
	{
		if (m_Node && newNode)
		{
			return m_Node->InsertFirstChild(newNode.m_Node);
		}
		return false;
	}
	bool XMLNode::InsertLastChild(XMLNode& newNode)
	{
		if (m_Node && newNode)
		{
			return m_Node->InsertEndChild(newNode.m_Node);
		}
		return false;
	}

	XMLNode XMLNode::NewElement(const String& name, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLNode node = m_Document->CreateElement(name);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}
	XMLNode XMLNode::NewComment(const String& value, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLNode node = m_Document->CreateComment(value);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}
	XMLNode XMLNode::NewText(const String& value, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLNode node = m_Document->CreateText(value);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}
	XMLNode XMLNode::NewDeclaration(const String& value, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLNode node = m_Document->CreateDeclaration(value);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}
	XMLNode XMLNode::NewUnknown(const String& value, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLNode node = m_Document->CreateUnknown(value);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}

	// XMLNode: Properties
	bool XMLNode::IsCDATA() const
	{
		if (m_Node)
		{
			if (auto text = m_Node->ToText())
			{
				return text->CData();
			}
		}
		return false;
	}
	bool XMLNode::SetCDATA(bool value)
	{
		if (m_Node)
		{
			if (auto text = m_Node->ToText())
			{
				text->SetCData(value);
				return true;
			}
		}
		return false;
	}

	XML::NodeType XMLNode::GetType() const
	{
		if (m_Node)
		{
			if (m_Node->ToDocument())
			{
				return NodeType::Document;
			}
			else if (m_Node->ToElement())
			{
				return NodeType::Element;
			}
			else if (m_Node->ToText())
			{
				return NodeType::Text;
			}
			else if (m_Node->ToComment())
			{
				return NodeType::Comment;
			}
			else if (m_Node->ToDeclaration())
			{
				return NodeType::Declaration;
			}
			else if (m_Node->ToUnknown())
			{
				return NodeType::Unknown;
			}
		}
		return NodeType::None;
	}
	bool XMLNode::IsElement() const
	{
		if (m_Node)
		{
			return m_Node->ToElement();
		}
		return false;
	}
	bool XMLNode::IsText() const
	{
		if (m_Node)
		{
			return m_Node->ToText();
		}
		return false;
	}

	// XMLNode: Serialization
	bool XMLNode::SerializeSubtree(IOutputStream& stream, SerializationFormat format) const
	{
		if (m_Node)
		{
			auto Serialize = [&](const tinyxml2::XMLDocument& document)
			{
				if (format == SerializationFormat::HTML5)
				{
					XMLPrinterHTML5 buffer;
					document.Print(&buffer);

					return stream.WriteAll(buffer.CStr(), buffer.CStrSize() - 1);
				}
				else
				{
					XMLPrinterDefault buffer;
					document.Print(&buffer);

					return stream.WriteAll(buffer.CStr(), buffer.CStrSize() - 1);
				}
			};

			if (auto document = m_Node->ToDocument())
			{
				return Serialize(*document);
			}
			else
			{
				tinyxml2::XMLDocument subTree;
				m_Node->ShallowClone(&subTree);

				return Serialize(subTree);
			}
		}
		return false;
	}
	String XMLNode::SerializeSubtree(SerializationFormat format) const
	{
		if (m_Node)
		{
			auto Serialize = [&](const tinyxml2::XMLDocument& document)
			{
				if (format == SerializationFormat::HTML5)
				{
					return PrintDocumentUsing<XMLPrinterHTML5>(document);
				}
				else
				{
					return PrintDocumentUsing<XMLPrinterDefault>(document);
				}
			};

			if (auto document = m_Node->ToDocument())
			{
				return Serialize(*document);
			}
			else
			{
				tinyxml2::XMLDocument subTree;
				m_Node->ShallowClone(&subTree);

				return Serialize(subTree);
			}
		}
		return {};
	}
	String XMLNode::SerializeSubtreeText(const String& separator) const
	{
		if (m_Node)
		{
			return CleanText(*m_Node, StringViewOf(separator));
		}
		return {};
	}
}
