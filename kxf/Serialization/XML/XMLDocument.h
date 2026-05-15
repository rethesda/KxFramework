#pragma once
#include "../Common.h"
#include "../XDocument.h"
#include "../BinarySerializer.h"
#include "kxf/Core/ILibraryInfo.h"

namespace tinyxml2
{
	class XMLNode;
	class XMLDocument;
	class XMLAttribute;
}

namespace kxf
{
	class XMLNode;
	class XMLDocument;
	class XMLAttribute;

	class IInputStream;
	class IOutputStream;
}

namespace kxf::XML
{
	enum class NodeType
	{
		None = -1,

		Document,
		Element,
		Text,
		Declaration,
		Comment,
		Unknown
	};
	enum class SerializationFormat
	{
		Default = 0,
		HTML5
	};
	enum class InsertMode
	{
		AfterChild,
		AsFirstChild,
		AsLastChild
	};
}

namespace kxf
{
	class KXF_API XMLAttribute final: public IXDocumentNode,
		public XDocument::RWValue<XMLAttribute>,
		private XDocument::DefaultConverter<XMLAttribute>
	{
		friend class XMLNode;

		friend class ROValue;
		friend class RWValue;
		friend class DefaultConverter;

		private:
			XMLNode* m_Owner = nullptr;
			tinyxml2::XMLAttribute* m_Attribute = nullptr;

		private:
			// XDocument::RWValue
			std::optional<String> XDocument_QueryValue() const;
			bool XDocument_WriteValue(const String& value, AsCDATA asCDATA);

		public:
			XMLAttribute() = default;
		private:
			XMLAttribute(XMLNode& owner, tinyxml2::XMLAttribute* attribute)
				:m_Owner(&owner), m_Attribute(attribute)
			{
				if (!attribute)
				{
					m_Owner = nullptr;
				}
			}
			XMLAttribute(const XMLNode& owner, const tinyxml2::XMLAttribute* attribute)
				:m_Owner(const_cast<XMLNode*>(&owner)), m_Attribute(const_cast<tinyxml2::XMLAttribute*>(attribute))
			{
				if (!attribute)
				{
					m_Owner = nullptr;
				}
			}

		public:
			// IXDocumentNode
			bool IsNull() const override
			{
				return !m_Owner || !m_Attribute;
			}
			String GetXPath() const override;

			String GetName() const override;
			size_t GetIndexWithinParent() const override;
			size_t GetRelativeIndexWithinParent() const override;

			// XMLAttribute
			XMLNode GetNode() const;
			XMLDocument& GetDocument() const;

			XMLAttribute Next() const;
	};
}

namespace kxf
{
	class KXF_API XMLNode: public IXDocumentNode,
		public XDocument::RWValue<XMLNode>,
		public XDocument::RWAttribute<XMLNode>,
		private XDocument::DefaultConverter<XMLNode>
	{
		friend class XMLDocument;
		friend class XMLAttribute;

		friend class ROValue;
		friend class RWValue;
		friend class ROAttribute;
		friend class RWAttribute;
		friend class DefaultConverter;

		public:
			using NodeType = XML::NodeType;
			using InsertMode = XML::InsertMode;
			using SerializationFormat = XML::SerializationFormat;

		private:
			XMLDocument* m_Document = nullptr;
			tinyxml2::XMLNode* m_Node = nullptr;

		private:
			// XDocument::RWValue
			std::optional<String> XDocument_QueryValue() const;
			bool XDocument_WriteValue(const String& value, AsCDATA asCDATA);

			// XDocument::RWAttribute
			std::optional<String> XDocument_QueryAttribute(const String& name) const;
			bool XDocument_WriteAttribute(const String& name, const String& value, AsCDATA asCDATA);

			// XMLNode
			XMLNode ConstructOrQueryElement(const String& xPath, bool allowCreate);

		public:
			XMLNode() = default;
		private:
			XMLNode(XMLDocument& document, tinyxml2::XMLNode* node)
				:m_Document(&document), m_Node(node)
			{
				if (!node)
				{
					m_Document = nullptr;
				}
			}
			XMLNode(XMLDocument& document, const tinyxml2::XMLNode* node)
				:m_Document(&document), m_Node(const_cast<tinyxml2::XMLNode*>(node))
			{
				if (!node)
				{
					m_Document = nullptr;
				}
			}

		public:
			// IXDocumentNode
			bool IsNull() const override
			{
				return !m_Node || !m_Document;
			}
			String GetXPath() const override;

			String GetName() const override;
			size_t GetIndexWithinParent() const override;
			size_t GetRelativeIndexWithinParent() const override;

			// XMLNode: Common
			XMLDocument& GetDocument()
			{
				return *m_Document;
			}
			const XMLDocument& GetDocument() const
			{
				return *m_Document;
			}

			bool SetName(const String& name);

			// XMLNode: Navigation
			XMLNode QueryElement(const String& xPath) const;
			XMLNode ConstructElement(const String& xPath);
			XMLNode QueryElementByAttribute(const String& name, const String& value) const;
			XMLNode QueryElementByName(const String& name) const;

			XMLNode GetParent() const;
			XMLNode GetPreviousSibling() const;
			XMLNode GetPreviousSiblingElement(const String& name = {}) const;
			XMLNode GetNextSibling() const;
			XMLNode GetNextSiblingElement(const String& name = {}) const;
			XMLNode GetFirstChild() const;
			XMLNode GetFirstChildElement(const String& name = {}) const;
			XMLNode GetLastChild() const;
			XMLNode GetLastChildElement(const String& name = {}) const;

			// XMLNode: Children
			size_t GetChildrenCount() const;
			bool HasChildren() const;
			void ClearChildren();
			void ResetNode();

			CallbackResult<void> EnumChildren(CallbackFunction<XMLNode> func) const;
			CallbackResult<void> EnumChildElements(CallbackFunction<XMLNode> func, const String& name = {}) const;

			// XMLNode: Attributes
			size_t GetAttributeCount() const;
			bool HasAttributes() const;

			XMLAttribute GetAttributeObject(const String& name) const;
			CallbackResult<void> EnumAttributeNames(CallbackFunction<String> func) const;
			CallbackResult<void> EnumAttributes(CallbackFunction<XMLAttribute> func) const;

			bool HasAttribute(const String& name) const;
			bool RemoveAttribute(const String& name);
			bool RemoveAttribute(XMLAttribute& attribute);
			bool ClearAttributes();

			// XMLNode: Insertion
			bool Insert(XMLNode& node, InsertMode insertMode);
			bool InsertAfterChild(XMLNode& newNode, const XMLNode& afterThis = {});
			bool InsertFirstChild(XMLNode& newNode);
			bool InsertLastChild(XMLNode& newNode);

			XMLNode NewElement(const String& name, InsertMode insertMode = InsertMode::AsLastChild);
			XMLNode NewComment(const String& value, InsertMode insertMode = InsertMode::AsLastChild);
			XMLNode NewText(const String& value, InsertMode insertMode = InsertMode::AsLastChild);
			XMLNode NewDeclaration(const String& value, InsertMode insertMode = InsertMode::AsLastChild);
			XMLNode NewUnknown(const String& value, InsertMode insertMode = InsertMode::AsLastChild);

			// XMLNode: Properties
			bool IsCDATA() const;
			bool SetCDATA(bool value = true);

			NodeType GetType() const;
			bool IsElement() const;
			bool IsText() const;

			// XMLNode: Serialization
			bool SerializeSubtree(IOutputStream& stream, SerializationFormat format = SerializationFormat::Default) const;
			String SerializeSubtree(SerializationFormat format = SerializationFormat::Default) const;
			String SerializeSubtreeText(const String& separator = {}) const;
	};
}

namespace kxf
{
	class KXF_API XMLDocument final: public RTTI::DynamicImplementation<XMLDocument, IXDocument>, public XMLNode
	{
		kxf_RTTI_DeclareIID(XMLDocument, {0xdafcb16, 0xb15c, 0x4c8e, {0x90, 0xfa, 0x9f, 0x24, 0x3f, 0x13, 0x69, 0x12}});

		friend class XMLNode;

		private:
			std::unique_ptr<tinyxml2::XMLDocument> m_Impl;

		private:
			// IObject
			RTTI::QueryInfo DoQueryInterface(const IID& iid) noexcept override;

			// XMLDocument
			void ReplaceDeclaration();
			
			bool DoLoad(const char* xml, size_t length);
			void DoUnload();

		private:
			XMLNode CreateElement(const String& name);
			XMLNode CreateComment(const String& value);
			XMLNode CreateText(const String& value);
			XMLNode CreateDeclaration(const String& value);
			XMLNode CreateUnknown(const String& value);

		public:
			XMLDocument();
			XMLDocument(const String& xml)
				:XMLDocument()
			{
				LoadDocument(xml);
			}
			XMLDocument(const XMLDocument&) = delete;
			XMLDocument(XMLDocument&&) noexcept;
			~XMLDocument();

		public:
			// IXDocument
			bool IsNull() const override;
			String GetDocumentMeta() const override;

			bool LoadDocument(IInputStream& stream) override;
			bool SaveDocument(IOutputStream& stream) const override;

			// XMLDocument
			bool LoadDocument(const String& xml);
			String SaveDocument() const;

			void ClearDocument();
			void RemoveNode(XMLNode& node);

		public:
			XMLDocument& operator=(const XMLDocument&) = delete;
			XMLDocument& operator=(XMLDocument&&) noexcept;
	};
}

namespace kxf
{
	template<>
	struct BinarySerializer<XMLDocument> final
	{
		uint64_t Serialize(IOutputStream& stream, const XMLDocument& value) const
		{
			return BinarySerializer<String>().Serialize(stream, value.SaveDocument());
		}
		uint64_t Deserialize(IInputStream& stream, XMLDocument& value) const
		{
			String buffer;
			auto read = BinarySerializer<String>().Deserialize(stream, buffer);

			value.LoadDocument(buffer);
			return read;
		}
	};
}
