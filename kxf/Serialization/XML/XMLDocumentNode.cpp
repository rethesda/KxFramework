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

	void CloneSubTree(tinyxml2::XMLDocument& subTree, const tinyxml2::XMLNode& node)
	{
		subTree.InsertFirstChild(node.DeepClone(&subTree));
	}
}

namespace kxf
{
	// XDocument::RWValue
	std::optional<String> XMLDocumentNode::XDocument_QueryValue() const
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
					return String::FromUTF8(text);
				}
			}
			else if (auto value = m_Node->Value())
			{
				return String::FromUTF8(value);
			}
		}
		return {};
	}
	bool XMLDocumentNode::XDocument_WriteValue(const String& value, AsCDATA asCDATA)
	{
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
	std::optional<String> XMLDocumentNode::XDocument_QueryAttribute(const String& name) const
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
	bool XMLDocumentNode::XDocument_WriteAttribute(const String& name, const String& value, AsCDATA asCDATA)
	{
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

	XMLDocumentNode XMLDocumentNode::QueryOrCreateElement(const String& xPath, bool allowCreate)
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
				auto [elementName, index] = XDocument::ExtractIndexFromElementName(name, indexSeparator.GetAs<XChar>());
				auto elementNameUTF8 = EncodingConverter_UTF8.ToMultiByte(elementName);

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
				return XMLDocumentNode(*m_Document, currentNode);
			}
		}
		return {};
	}

	// IXDocumentNode
	String XMLDocumentNode::GetXPath() const
	{
		if (m_Document && m_Node)
		{
			return XDocument::BacktrackXPath(*m_Document, *this);
		}
		return {};
	}

	String XMLDocumentNode::GetName() const
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
	size_t XMLDocumentNode::GetIndexWithinParent() const
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
	size_t XMLDocumentNode::GetRelativeIndexWithinParent() const
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

	// XMLDocumentNode: Common
	bool XMLDocumentNode::SetName(const String& name)
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

	// XMLDocumentNode: Navigation
	XMLDocumentNode XMLDocumentNode::QueryElement(const String& xPath) const
	{
		return const_cast<XMLDocumentNode&>(*this).QueryOrCreateElement(xPath, false);
	}
	XMLDocumentNode XMLDocumentNode::CreateElement(const String& xPath)
	{
		return const_cast<XMLDocumentNode&>(*this).QueryOrCreateElement(xPath, true);
	}
	XMLDocumentNode XMLDocumentNode::QueryElementByAttribute(const String& name, const String& value) const
	{
		if (m_Node)
		{
			if (GetAttribute(name) == value)
			{
				return XMLDocumentNode(*m_Document, m_Node);
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
	XMLDocumentNode XMLDocumentNode::QueryElementByName(const String& name) const
	{
		if (m_Node)
		{
			if (GetName() == name)
			{
				return XMLDocumentNode(*m_Document, m_Node);
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

	XMLDocumentNode XMLDocumentNode::GetParent() const
	{
		if (m_Node)
		{
			return XMLDocumentNode(*m_Document, m_Node->Parent());
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::GetPreviousSibling() const
	{
		if (m_Node)
		{
			return XMLDocumentNode(*m_Document, m_Node->PreviousSibling());
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::GetPreviousSiblingElement(const String& name) const
	{
		if (m_Node)
		{
			if (name.IsEmpty())
			{
				return XMLDocumentNode(*m_Document, m_Node->PreviousSiblingElement());
			}
			else
			{
				return XMLDocumentNode(*m_Document, m_Node->PreviousSiblingElement(name.utf8_str()));
			}
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::GetNextSibling() const
	{
		if (m_Node)
		{
			return XMLDocumentNode(*m_Document, m_Node->NextSibling());
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::GetNextSiblingElement(const String& name) const
	{
		if (m_Node)
		{
			if (name.IsEmpty())
			{
				return XMLDocumentNode(*m_Document, m_Node->NextSiblingElement());
			}
			else
			{
				return XMLDocumentNode(*m_Document, m_Node->NextSiblingElement(name.utf8_str()));
			}
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::GetFirstChild() const
	{
		if (m_Node)
		{
			return XMLDocumentNode(*m_Document, m_Node->FirstChild());
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::GetFirstChildElement(const String& name) const
	{
		if (m_Node)
		{
			if (name.IsEmpty())
			{
				return XMLDocumentNode(*m_Document, m_Node->FirstChildElement());
			}
			else
			{
				return XMLDocumentNode(*m_Document, m_Node->FirstChildElement(name.utf8_str()));
			}
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::GetLastChild() const
	{
		if (m_Node)
		{
			return XMLDocumentNode(*m_Document, m_Node->LastChild());
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::GetLastChildElement(const String& name) const
	{
		if (m_Node)
		{
			if (name.IsEmpty())
			{
				return XMLDocumentNode(*m_Document, m_Node->LastChildElement());
			}
			else
			{
				return XMLDocumentNode(*m_Document, m_Node->LastChildElement(name.utf8_str()));
			}
		}
		return {};
	}

	// XMLDocumentNode: Children
	size_t XMLDocumentNode::GetChildrenCount() const
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
	bool XMLDocumentNode::HasChildren() const
	{
		if (m_Node)
		{
			return !m_Node->NoChildren();
		}
		return false;
	}
	void XMLDocumentNode::ClearChildren()
	{
		if (m_Node)
		{
			m_Node->DeleteChildren();
		}
	}
	void XMLDocumentNode::ResetNode()
	{
		ClearChildren();
		ClearAttributes();
	}

	CallbackResult<void> XMLDocumentNode::EnumChildren(CallbackFunction<XMLDocumentNode> func) const
	{
		if (m_Node)
		{
			for (auto child = m_Node->FirstChild(); child; child = child->NextSibling())
			{
				if (func.Invoke(XMLDocumentNode(*m_Document, child)).ShouldTerminate())
				{
					break;
				}
			}

			return func.Finalize();
		}
		return {};
	}
	CallbackResult<void> XMLDocumentNode::EnumChildElements(CallbackFunction<XMLDocumentNode> func, const String& name) const
	{
		if (m_Node)
		{
			auto nameUTF8 = name.utf8_str();
			for (auto child = m_Node->FirstChildElement(nameUTF8.data_if_not_empty()); child; child = child->NextSiblingElement(nameUTF8.data_if_not_empty()))
			{
				if (func.Invoke(XMLDocumentNode(*m_Document, child)).ShouldTerminate())
				{
					break;
				}
			}

			return func.Finalize();
		}
		return {};
	}

	// XMLDocumentNode: Attributes
	size_t XMLDocumentNode::GetAttributeCount() const
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
	bool XMLDocumentNode::HasAttributes() const
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

	XMLDocumentAttribute XMLDocumentNode::GetAttributeObject(const String& name) const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				return XMLDocumentAttribute(*m_Document, element->FindAttribute(name.utf8_str()));
			}
		}
		return {};
	}
	CallbackResult<void> XMLDocumentNode::EnumAttributeNames(CallbackFunction<String> func) const
	{
		return EnumAttributes([&](XMLDocumentAttribute attribute)
		{
			return func.Invoke(attribute.GetName()).GetLastCommand();
		});
	}
	CallbackResult<void> XMLDocumentNode::EnumAttributes(CallbackFunction<XMLDocumentAttribute> func) const
	{
		if (m_Node)
		{
			if (auto element = m_Node->ToElement())
			{
				for (auto attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
				{
					if (func.Invoke(XMLDocumentAttribute(*this, attribute)).ShouldTerminate())
					{
						break;
					}
				}
				return func.Finalize();
			}
		}
		return {};
	}

	bool XMLDocumentNode::HasAttribute(const String& name) const
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
	bool XMLDocumentNode::RemoveAttribute(const String& name)
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
	bool XMLDocumentNode::RemoveAttribute(XMLDocumentAttribute& attribute)
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
	bool XMLDocumentNode::ClearAttributes()
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
	bool XMLDocumentNode::Insert(XMLDocumentNode& node, InsertMode insertMode)
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
	bool XMLDocumentNode::InsertAfterChild(XMLDocumentNode& newNode, const XMLDocumentNode& afterThis)
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
	bool XMLDocumentNode::InsertFirstChild(XMLDocumentNode& newNode)
	{
		if (m_Node && newNode)
		{
			return m_Node->InsertFirstChild(newNode.m_Node);
		}
		return false;
	}
	bool XMLDocumentNode::InsertLastChild(XMLDocumentNode& newNode)
	{
		if (m_Node && newNode)
		{
			return m_Node->InsertEndChild(newNode.m_Node);
		}
		return false;
	}

	XMLDocumentNode XMLDocumentNode::NewElement(const String& name, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLDocumentNode node = m_Document->CreateNewElement(name);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::NewComment(const String& value, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLDocumentNode node = m_Document->CreateNewComment(value);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::NewText(const String& value, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLDocumentNode node = m_Document->CreateNewText(value);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::NewDeclaration(const String& value, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLDocumentNode node = m_Document->CreateNewDeclaration(value);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}
	XMLDocumentNode XMLDocumentNode::NewUnknown(const String& value, InsertMode insertMode)
	{
		if (m_Document)
		{
			XMLDocumentNode node = m_Document->CreateNewUnknown(value);
			if (node && Insert(node, insertMode))
			{
				return node;
			}
		}
		return {};
	}

	// XMLDocumentNode: Properties
	bool XMLDocumentNode::IsCDATA() const
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
	bool XMLDocumentNode::SetCDATA(bool value)
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

	XML::NodeType XMLDocumentNode::GetType() const
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
	bool XMLDocumentNode::IsElement() const
	{
		if (m_Node)
		{
			return m_Node->ToElement();
		}
		return false;
	}
	bool XMLDocumentNode::IsText() const
	{
		if (m_Node)
		{
			return m_Node->ToText();
		}
		return false;
	}

	// XMLDocumentNode: Serialization
	bool XMLDocumentNode::SerializeSubtree(IOutputStream& stream, SerializationFormat format) const
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
				CloneSubTree(subTree, *m_Node);

				return Serialize(subTree);
			}
		}
		return false;
	}
	String XMLDocumentNode::SerializeSubtree(SerializationFormat format) const
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
				CloneSubTree(subTree, *m_Node);

				return Serialize(subTree);
			}
		}
		return {};
	}
	String XMLDocumentNode::SerializeSubtreeText(const String& separator) const
	{
		if (m_Node)
		{
			return CleanText(*m_Node, StringViewOf(separator));
		}
		return {};
	}
}
