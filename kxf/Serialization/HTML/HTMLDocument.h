#pragma once
#include "../Common.h"
#include "../XDocument.h"
#include "kxf/Core/Version.h"
#include "kxf/IO/IStream.h"

namespace kxf::HTML
{
	enum class NodeType;
	enum class TagType;
}

namespace kxf
{
	class HTMLDocument;
}

namespace kxf
{
	class KXF_API HTMLDocumentNode: public IXDocumentNode,
									public XDocument::ROValue<HTMLDocumentNode>,
									public XDocument::ROAttribute<HTMLDocumentNode>,
									private XDocument::DefaultConverter<HTMLDocumentNode>
	{
		friend class ROValue;
		friend class ROAttribute;
		friend class DefaultConverter;

		public:
			using NodeType = HTML::NodeType;
			using TagType = HTML::TagType;

		protected:
			const HTMLDocument* m_Document = nullptr;
			const void* m_Node = nullptr;

		protected:
			// XDocument::ROValue
			std::optional<String> XDocument_QueryValue() const;

			// XDocument::ROAttribute
			std::optional<String> XDocument_QueryAttribute(const String& name) const;

			// HTMLDocumentNode
			virtual const void* GetNode() const
			{
				return m_Node;
			}

		public:
			HTMLDocumentNode() = default;
			HTMLDocumentNode(const HTMLDocumentNode&) = default;
			HTMLDocumentNode(HTMLDocumentNode&& other) noexcept
			{
				*this = std::move(other);
			}
		protected:
			HTMLDocumentNode(const HTMLDocument& document, const void* node)
				:m_Document(&document), m_Node(node)
			{
			}

		public:
			// IXDocumentNode
			bool IsNull() const override;
			String GetXPath() const override;

			String GetName() const override;
			size_t GetIndexWithinParent() const override;
			size_t GetRelativeIndexWithinParent() const override;

			// HTMLDocumentNode
			const HTMLDocument& GetDocument() const
			{
				return *m_Document;
			}

			bool IsFullNode() const;
			NodeType GetType() const;
			TagType GetTagType() const;
			String GetValueText() const;
			virtual String GetHTML() const;

			// HTMLDocumentNode: Children
			size_t GetChildrenCount() const;
			CallbackResult<void> EnumChildren(CallbackFunction<HTMLDocumentNode> func) const;

			// HTMLDocumentNode: Attributes
			size_t GetAttributeCount() const;
			bool HasAttribute(const String& name) const;
			CallbackResult<void> EnumAttributeNames(CallbackFunction<String> func) const;
			
			// HTMLDocumentNode: Navigation
			HTMLDocumentNode QueryElement(const String& XPath) const;
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
			HTMLDocumentNode GetNextSibling() const;
			HTMLDocumentNode GetFirstChild() const;
			HTMLDocumentNode GetLastChild() const;

		public:
			HTMLDocumentNode& operator=(const HTMLDocumentNode&) = delete;
			HTMLDocumentNode& operator=(HTMLDocumentNode&& other) noexcept
			{
				m_Document = std::exchange(other.m_Document, nullptr);
				m_Node = std::exchange(other.m_Node, nullptr);

				return *this;
			}
	};
}

namespace kxf
{
	class KXF_API HTMLDocument final: public RTTI::DynamicImplementation<HTMLDocument, IXDocument>, public HTMLDocumentNode
	{
		kxf_RTTI_DeclareIID(HTMLDocument, {0xda03b100, 0x8c33, 0x4dc2, {0x83, 0xfd, 0xea, 0xc2, 0xe5, 0x9a, 0x94, 0x57}});

		class ImplOptions;

		private:
			std::string m_Buffer;
			std::unique_ptr<ImplOptions> m_ParserOptions;
			void* m_ParserOutput = nullptr;

		private:
			void Init();
			void DoLoad();
			void DoUnload();
			void Destroy();

			// HTMLDocumentNode
			const void* GetNode() const override;

		public:
			HTMLDocument();
			HTMLDocument(const String& html);
			HTMLDocument(const HTMLDocument&) = delete;
			HTMLDocument(HTMLDocument&& other) noexcept;
			~HTMLDocument();

		public:
			// IXDocument
			bool IsNull() const override;
			String GetDocumentMeta() const override;

			bool LoadDocument(IInputStream& stream) override;
			bool SaveDocument(IOutputStream& stream) const override;

			// HTMLDocument
			bool LoadDocument(const String& html);
			String SaveDocument() const
			{
				return GetHTML();
			}

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

		Document = 0, // GumboNodeType::GUMBO_NODE_DOCUMENT,
		Element = 1, // GumboNodeType::GUMBO_NODE_ELEMENT,
		Text = 2, // GumboNodeType::GUMBO_NODE_TEXT,
		CData = 3, // GumboNodeType::GUMBO_NODE_CDATA,
		Comment = 4, // GumboNodeType::GUMBO_NODE_COMMENT,
		NodeTemplate = 5, // GumboNodeType::GUMBO_NODE_TEMPLATE,
		NodeWhitespace = 6, // GumboNodeType::GUMBO_NODE_WHITESPACE,
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
