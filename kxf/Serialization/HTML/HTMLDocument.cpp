#include "kxf-pch.h"
#include "HTMLDocument.h"

#pragma warning(disable: 4005) // macro redefinition
#include "gumbo.h"
#include "Private/error.h"

namespace
{
	GumboOutput* CastOutput(void* output) noexcept
	{
		return reinterpret_cast<GumboOutput*>(output);
	}
}

namespace kxf
{
	class HTMLDocument::ImplOptions final: public GumboOptions
	{
		public:
			ImplOptions(const GumboOptions& options, HTMLDocument& ref) noexcept
				:GumboOptions(options)
			{
				userdata = &ref;
			}
	};
}

namespace kxf
{
	// HTMLDocument
	void HTMLDocument::Init()
	{
		m_ParserOptions = std::make_unique<ImplOptions>(kGumboDefaultOptions, *this);
	}
	void HTMLDocument::DoLoad()
	{
		m_ParserOutput = gumbo_parse_with_options(m_ParserOptions.get(), m_Buffer.data(), m_Buffer.size());
		m_Node = CastOutput(m_ParserOutput)->document;
	}
	void HTMLDocument::DoUnload()
	{
		if (m_ParserOutput)
		{
			gumbo_destroy_output(m_ParserOptions.get(), CastOutput(m_ParserOutput));
			m_ParserOutput = nullptr;
		}
		m_Node = nullptr;
		m_Buffer.clear();
	}
	void HTMLDocument::Destroy()
	{
		DoUnload();
	}

	// HTMLNode
	const void* HTMLDocument::GetNode() const
	{
		return CastOutput(m_ParserOutput)->document;
	}

	// HTMLDocument
	HTMLDocument::HTMLDocument()
		:HTMLNode(*this, nullptr)
	{
		Init();
	}
	HTMLDocument::HTMLDocument(const String& html)
		:HTMLDocument()
	{
		LoadDocument(html);
	}
	HTMLDocument::HTMLDocument(HTMLDocument&& other) noexcept
		:HTMLDocument()
	{
		*this = std::move(other);
	}
	HTMLDocument::~HTMLDocument()
	{
		Destroy();
	}

	// IXDocument
	bool HTMLDocument::IsNull() const
	{
		if (!m_Buffer.empty() && m_ParserOutput)
		{
			if (auto errors = reinterpret_cast<GumboError**>(CastOutput(m_ParserOutput)->errors.data))
			{
				for (size_t i = 0; i < CastOutput(m_ParserOutput)->errors.length; i++)
				{
					if (errors[i]->type == GUMBO_ERR_PARSER)
					{
						return true;
					}
				}
			}
			return false;
		}
		return true;
	}
	String HTMLDocument::GetDocumentMeta() const
	{
		return {};
	}

	bool HTMLDocument::LoadDocument(IInputStream& stream)
	{
		DoUnload();

		m_Buffer.resize(stream.GetSize().ToBytes());
		stream.Read(m_Buffer.data(), m_Buffer.size());
		m_Buffer.resize(stream.LastRead().ToBytes());

		DoLoad();
		return !IsNull();
	}
	bool HTMLDocument::SaveDocument(IOutputStream& stream) const
	{
		auto utf8 = GetHTML().ToUTF8();
		return stream.WriteAll(utf8.data(), utf8.length());
	}

	// HTMLDocument
	bool HTMLDocument::LoadDocument(const String& html)
	{
		DoUnload();

		if (!html.IsEmpty())
		{
			m_Buffer = html.ToUTF8();
			DoLoad();
		}
		return !IsNull();
	}

	HTMLDocument& HTMLDocument::operator=(HTMLDocument&& other) noexcept
	{
		static_cast<HTMLNode&>(*this) = std::move(static_cast<HTMLNode&>(other));
		m_Buffer = std::move(other.m_Buffer);
		m_ParserOptions = std::move(other.m_ParserOptions);
		m_ParserOutput = std::exchange(other.m_ParserOutput, nullptr);

		return *this;
	}
}
