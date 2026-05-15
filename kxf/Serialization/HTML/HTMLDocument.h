#pragma once
#include "../Common.h"
#include "../XDocument.h"

namespace kxf::HTML
{
	enum class NodeType;
	enum class TagType;
}

namespace kxf
{
	class HTMLDocument;

	class IInputStream;
	class IOutputStream;
}

namespace kxf
{
	class KXF_API HTMLDocumentAttribute final: public IXDocumentNode,
											   public XDocument::ROValue<HTMLDocumentAttribute>,
											   private XDocument::DefaultConverter<HTMLDocumentAttribute>
	{
		friend class HTMLDocument;
		friend class HTMLDocumentNode;

		friend class ROValue;
		friend class DefaultConverter;

		private:
			HTMLDocumentNode* m_Owner = nullptr;
			const void* m_Attribute = nullptr;

		private:
			// XDocument::ROValue
			std::optional<String> XDocument_QueryValue() const;

		public:
			HTMLDocumentAttribute() = default;
		private:
			HTMLDocumentAttribute(HTMLDocumentNode& owner, void* attribute)
				:m_Owner(&owner), m_Attribute(attribute)
			{
				if (!attribute)
				{
					m_Owner = nullptr;
				}
			}
			HTMLDocumentAttribute(const HTMLDocumentNode& owner, const void* attribute)
				:m_Owner(const_cast<HTMLDocumentNode*>(&owner)), m_Attribute(const_cast<void*>(attribute))
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

			// HTMLDocumentAttribute
			HTMLDocumentNode GetNode() const;
			const HTMLDocument& GetDocument() const;
			HTMLDocument& GetDocument();

			HTMLDocumentAttribute Next() const;
	};
}

namespace kxf
{
	class KXF_API HTMLDocumentNode: public IXDocumentNode,
									public XDocument::ROValue<HTMLDocumentNode>,
									public XDocument::ROAttribute<HTMLDocumentNode>,
									private XDocument::DefaultConverter<HTMLDocumentNode>
	{
		friend class HTMLDocument;
		friend class HTMLDocumentAttribute;

		friend class ROValue;
		friend class ROAttribute;
		friend class DefaultConverter;

		public:
			using NodeType = HTML::NodeType;
			using TagType = HTML::TagType;

		private:
			HTMLDocument* m_Document = nullptr;
			void* m_Node = nullptr;

		private:
			// XDocument::ROValue
			std::optional<String> XDocument_QueryValue() const;

			// XDocument::ROAttribute
			std::optional<String> XDocument_QueryAttribute(const String& name) const;

		public:
			HTMLDocumentNode() = default;
		protected:
			HTMLDocumentNode(HTMLDocument& document, void* node)
				:m_Document(&document), m_Node(node)
			{
				if (!node)
				{
					m_Document = nullptr;
				}
			}
			HTMLDocumentNode(const HTMLDocument& document, const void* node)
				:m_Document(const_cast<HTMLDocument*>(&document)), m_Node(const_cast<void*>(node))
			{
				if (!node)
				{
					m_Document = nullptr;
				}
			}

		public:
			// IXDocumentNode
			bool IsNull() const override;
			String GetXPath() const override;

			String GetName() const override;
			size_t GetIndexWithinParent() const override;
			size_t GetRelativeIndexWithinParent() const override;

			// HTMLDocumentNode: Common
			const HTMLDocument& GetDocument() const
			{
				return *m_Document;
			}
			HTMLDocument& GetDocument()
			{
				return *m_Document;
			}
			
			// HTMLDocumentNode: Navigation
			HTMLDocumentNode QueryElement(const String& xPath) const;
			HTMLDocumentNode QueryElementByAttribute(const String& name, const String& value) const;
			HTMLDocumentNode QueryElementByID(const String& id) const
			{
				return QueryElementByAttribute("id", id);
			}
			HTMLDocumentNode QueryElementByClass(const String & className) const
			{
				return QueryElementByAttribute("class", className);
			}
			HTMLDocumentNode QueryElementByName(TagType tagType) const;
			HTMLDocumentNode QueryElementByName(const String& tagName) const;
		
			HTMLDocumentNode GetParent() const;
			HTMLDocumentNode GetPreviousSibling() const;
			HTMLDocumentNode GetPreviousSiblingElement(const String& name) const;
			HTMLDocumentNode GetNextSibling() const;
			HTMLDocumentNode GetNextSiblingElement(const String& name) const;
			HTMLDocumentNode GetFirstChild() const;
			HTMLDocumentNode GetFirstChildElement(const String& name) const;
			HTMLDocumentNode GetLastChild() const;
			HTMLDocumentNode GetLastChildElement(const String& name) const;

			// HTMLDocumentNode: Children
			size_t GetChildrenCount() const;
			CallbackResult<void> EnumChildren(CallbackFunction<HTMLDocumentNode> func) const;

			// HTMLDocumentNode: Attributes
			size_t GetAttributeCount() const;
			bool HasAttribute(const String& name) const;

			HTMLDocumentAttribute GetAttributeObject(const String& name) const;
			CallbackResult<void> EnumAttributeNames(CallbackFunction<String> func) const;
			CallbackResult<void> EnumAttributes(CallbackFunction<HTMLDocumentAttribute> func) const;

			// HTMLDocumentNode: Properties
			NodeType GetType() const;
			TagType GetTagType() const;
			bool IsElement() const;
			bool IsText() const;

			// HTMLDocumentNode: Serialization
			bool SerializeSubtree(IOutputStream& stream) const;
			String SerializeSubtree() const;
			String SerializeSubtreeText(const String& separator = {}) const;
	};
}

namespace kxf
{
	class KXF_API HTMLDocument final: public RTTI::DynamicImplementation<HTMLDocument, IXDocument>, public HTMLDocumentNode
	{
		kxf_RTTI_DeclareIID(HTMLDocument, {0xda03b100, 0x8c33, 0x4dc2, {0x83, 0xfd, 0xea, 0xc2, 0xe5, 0x9a, 0x94, 0x57}});

		class ImplOptions;

		friend class HTMLDocumentNode;
		friend class HTMLDocumentAttribute;

		private:
			std::string m_Buffer;
			std::unique_ptr<ImplOptions> m_ParserOptions;
			void* m_ParserOutput = nullptr;

		private:
			bool DoLoad();
			void DoUnload();

		public:
			HTMLDocument();
			HTMLDocument(const String& html)
				:HTMLDocument()
			{
				LoadDocument(html);
			}
			HTMLDocument(const HTMLDocument&) = delete;
			HTMLDocument(HTMLDocument&& other) noexcept
				:HTMLDocument()
			{
				*this = std::move(other);
			}
			~HTMLDocument();

		public:
			// IXDocument
			bool IsNull() const override;
			String GetDocumentMeta() const override;

			bool LoadDocument(IInputStream& stream) override;
			bool SaveDocument(IOutputStream& stream) const override;

			// HTMLDocument
			bool LoadDocument(const String& html);
			String SaveDocument() const;

		public:
			HTMLDocument& operator=(const HTMLDocument&) = delete;
			HTMLDocument& operator=(HTMLDocument&& other) noexcept;
	};
}

namespace kxf::HTML
{
	enum class NodeType
	{
		None = -1,

		Document = 0,
		Element = 1,
		Text = 2,
		CData = 3,
		Comment = 4,
		NodeTemplate = 5,
		NodeWhitespace = 6
	};
	enum class TagType
	{
		HTML,
		HEAD,
		TITLE,
		BASE,
		LINK,
		META,
		STYLE,
		SCRIPT,
		NOSCRIPT,
		TEMPLATE,
		BODY,
		ARTICLE,
		SECTION,
		NAV,
		ASIDE,
		H1,
		H2,
		H3,
		H4,
		H5,
		H6,
		HGROUP,
		HEADER,
		FOOTER,
		ADDRESS,
		P,
		HR,
		PRE,
		BLOCKQUOTE,
		OL,
		UL,
		LI,
		DL,
		DT,
		DD,
		FIGURE,
		FIGCAPTION,
		MAIN,
		DIV,
		A,
		EM,
		STRONG,
		SMALL,
		S,
		CITE,
		Q,
		DFN,
		ABBR,
		DATA,
		TIME,
		CODE,
		VAR,
		SAMP,
		KBD,
		SUB,
		SUP,
		I,
		B,
		U,
		MARK,
		RUBY,
		RT,
		RP,
		BDI,
		BDO,
		SPAN,
		BR,
		WBR,
		INS,
		DEL,
		IMAGE,
		IMG,
		IFRAME,
		EMBED,
		OBJECT,
		PARAM,
		VIDEO,
		AUDIO,
		SOURCE,
		TRACK,
		CANVAS,
		MAP,
		AREA,
		MATH,
		MI,
		MO,
		MN,
		MS,
		MTEXT,
		MGLYPH,
		MALIGNMARK,
		ANNOTATION_XML,
		SVG,
		FOREIGNOBJECT,
		DESC,
		TABLE,
		CAPTION,
		COLGROUP,
		COL,
		TBODY,
		THEAD,
		TFOOT,
		TR,
		TD,
		TH,
		FORM,
		FIELDSET,
		LEGEND,
		LABEL,
		INPUT,
		BUTTON,
		SELECT,
		DATALIST,
		OPTGROUP,
		OPTION,
		TEXTAREA,
		KEYGEN,
		OUTPUT,
		PROGRESS,
		METER,
		DETAILS,
		SUMMARY,
		MENU,
		MENUITEM,
		APPLET,
		ACRONYM,
		BGSOUND,
		DIR,
		FRAME,
		FRAMESET,
		NOFRAMES,
		ISINDEX,
		LISTING,
		XMP,
		NEXTID,
		NOEMBED,
		PLAINTEXT,
		RB,
		STRIKE,
		BASEFONT,
		BIG,
		BLINK,
		CENTER,
		FONT,
		MARQUEE,
		MULTICOL,
		NOBR,
		SPACER,
		TT,
		RTC,

		UNKNOWN,
		LAST,
	};
}

namespace kxf
{
	template<>
	struct BinarySerializer<HTMLDocument> final
	{
		uint64_t Serialize(IOutputStream& stream, const HTMLDocument& value) const
		{
			return BinarySerializer<String>().Serialize(stream, value.SaveDocument());
		}
		uint64_t Deserialize(IInputStream& stream, HTMLDocument& value) const
		{
			String buffer;
			auto read = BinarySerializer<String>().Deserialize(stream, buffer);

			value.LoadDocument(buffer);
			return read;
		}
	};
}
