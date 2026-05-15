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
	class XMLDocumentNode;
	class XMLDocument;
	class XMLDocumentAttribute;

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
	class KXF_API XMLDocumentAttribute final: public IXDocumentNode,
											  public XDocument::RWValue<XMLDocumentAttribute>,
											  private XDocument::DefaultConverter<XMLDocumentAttribute>
	{
		friend class XMLDocumentNode;

		friend class ROValue;
		friend class RWValue;
		friend class DefaultConverter;

		private:
			XMLDocumentNode* m_Owner = nullptr;
			tinyxml2::XMLAttribute* m_Attribute = nullptr;

		private:
			// XDocument::RWValue
			std::optional<String> XDocument_QueryValue() const;
			bool XDocument_WriteValue(const String& value, AsCDATA asCDATA);

		public:
			XMLDocumentAttribute() = default;
		private:
			XMLDocumentAttribute(XMLDocumentNode& owner, tinyxml2::XMLAttribute* attribute)
				:m_Owner(&owner), m_Attribute(attribute)
			{
				if (!attribute)
				{
					m_Owner = nullptr;
				}
			}
			XMLDocumentAttribute(const XMLDocumentNode& owner, const tinyxml2::XMLAttribute* attribute)
				:m_Owner(const_cast<XMLDocumentNode*>(&owner)), m_Attribute(const_cast<tinyxml2::XMLAttribute*>(attribute))
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

			// XMLDocumentAttribute
			XMLDocumentNode GetNode() const;
			const XMLDocument& GetDocument() const;
			XMLDocument& GetDocument();

			XMLDocumentAttribute Next() const;
	};
}

namespace kxf
{
	class KXF_API XMLDocumentNode: public IXDocumentNode,
								   public XDocument::RWValue<XMLDocumentNode>,
								   public XDocument::RWAttribute<XMLDocumentNode>,
								   private XDocument::DefaultConverter<XMLDocumentNode>
	{
		friend class XMLDocument;
		friend class XMLDocumentAttribute;

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

			// XMLDocumentNode
			XMLDocumentNode QueryOrCreateElement(const String& xPath, bool allowCreate);

		public:
			XMLDocumentNode() = default;
		private:
			XMLDocumentNode(XMLDocument& document, tinyxml2::XMLNode* node)
				:m_Document(&document), m_Node(node)
			{
				if (!node)
				{
					m_Document = nullptr;
				}
			}
			XMLDocumentNode(XMLDocument& document, const tinyxml2::XMLNode* node)
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

			// XMLDocumentNode: Common
			const XMLDocument& GetDocument() const
			{
				return *m_Document;
			}
			XMLDocument& GetDocument()
			{
				return *m_Document;
			}

			bool SetName(const String& name);

			// XMLDocumentNode: Navigation
			XMLDocumentNode QueryElement(const String& xPath) const;
			XMLDocumentNode CreateElement(const String& xPath);
			XMLDocumentNode QueryElementByAttribute(const String& name, const String& value) const;
			XMLDocumentNode QueryElementByName(const String& name) const;

			XMLDocumentNode GetParent() const;
			XMLDocumentNode GetPreviousSibling() const;
			XMLDocumentNode GetPreviousSiblingElement(const String& name = {}) const;
			XMLDocumentNode GetNextSibling() const;
			XMLDocumentNode GetNextSiblingElement(const String& name = {}) const;
			XMLDocumentNode GetFirstChild() const;
			XMLDocumentNode GetFirstChildElement(const String& name = {}) const;
			XMLDocumentNode GetLastChild() const;
			XMLDocumentNode GetLastChildElement(const String& name = {}) const;

			// XMLNode: Children
			size_t GetChildrenCount() const;
			bool HasChildren() const;
			void ClearChildren();
			void ResetNode();

			CallbackResult<void> EnumChildren(CallbackFunction<XMLDocumentNode> func) const;
			CallbackResult<void> EnumChildElements(CallbackFunction<XMLDocumentNode> func, const String& name = {}) const;

			// XMLDocumentNode: Attributes
			size_t GetAttributeCount() const;
			bool HasAttributes() const;

			XMLDocumentAttribute GetAttributeObject(const String& name) const;
			CallbackResult<void> EnumAttributeNames(CallbackFunction<String> func) const;
			CallbackResult<void> EnumAttributes(CallbackFunction<XMLDocumentAttribute> func) const;

			bool HasAttribute(const String& name) const;
			bool RemoveAttribute(const String& name);
			bool RemoveAttribute(XMLDocumentAttribute& attribute);
			bool ClearAttributes();

			// XMLDocumentNode: Insertion
			bool Insert(XMLDocumentNode& node, InsertMode insertMode);
			bool InsertAfterChild(XMLDocumentNode& newNode, const XMLDocumentNode& afterThis = {});
			bool InsertFirstChild(XMLDocumentNode& newNode);
			bool InsertLastChild(XMLDocumentNode& newNode);

			XMLDocumentNode NewElement(const String& name, InsertMode insertMode = InsertMode::AsLastChild);
			XMLDocumentNode NewComment(const String& value, InsertMode insertMode = InsertMode::AsLastChild);
			XMLDocumentNode NewText(const String& value, InsertMode insertMode = InsertMode::AsLastChild);
			XMLDocumentNode NewDeclaration(const String& value, InsertMode insertMode = InsertMode::AsLastChild);
			XMLDocumentNode NewUnknown(const String& value, InsertMode insertMode = InsertMode::AsLastChild);

			// XMLDocumentNode: Properties
			bool IsCDATA() const;
			bool SetCDATA(bool value = true);

			NodeType GetType() const;
			bool IsElement() const;
			bool IsText() const;

			// XMLDocumentNode: Serialization
			bool SerializeSubtree(IOutputStream& stream, SerializationFormat format = SerializationFormat::Default) const;
			String SerializeSubtree(SerializationFormat format = SerializationFormat::Default) const;
			String SerializeSubtreeText(const String& separator = {}) const;
	};
}

namespace kxf
{
	class KXF_API XMLDocument final: public RTTI::DynamicImplementation<XMLDocument, IXDocument>, public XMLDocumentNode
	{
		kxf_RTTI_DeclareIID(XMLDocument, {0xdafcb16, 0xb15c, 0x4c8e, {0x90, 0xfa, 0x9f, 0x24, 0x3f, 0x13, 0x69, 0x12}});

		friend class XMLDocumentNode;

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
			XMLDocumentNode CreateNewElement(const String& name);
			XMLDocumentNode CreateNewComment(const String& value);
			XMLDocumentNode CreateNewText(const String& value);
			XMLDocumentNode CreateNewDeclaration(const String& value);
			XMLDocumentNode CreateNewUnknown(const String& value);

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
			void RemoveNode(XMLDocumentNode& node);

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
