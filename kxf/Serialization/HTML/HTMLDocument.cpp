#include "kxf-pch.h"
#include "HTMLDocument.h"
#include "kxf/IO/IStream.h"

#pragma warning(disable: 4005) // macro redefinition
#include "gumbo.h"
#include "Private/error.h"

namespace
{
	constexpr GumboOutput* CastOutput(void* output) noexcept
	{
		return static_cast<GumboOutput*>(output);
	}

	bool AnyError(const GumboOutput* output)
	{
		return output->errors.length != 0;
	}
	bool AnyErrorOfType(const GumboOutput* output, GumboErrorType errorType)
	{
		if (auto errors = reinterpret_cast<GumboError**>(output->errors.data))
		{
			for (size_t i = 0; i < output->errors.length; i++)
			{
				if (errors[i]->type == errorType)
				{
					return true;
				}
			}
		}
		return false;
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
				tab_stop = 4;
			}
	};
}

namespace kxf
{
	// HTMLDocument
	bool HTMLDocument::DoLoad()
	{
		// Make sure it's allocated on the heap so it won't cause any issues if the object is moved
		m_Buffer.reserve(64);

		m_ParserOutput = gumbo_parse_with_options(m_ParserOptions.get(), m_Buffer.data(), m_Buffer.size());
		if (m_ParserOutput)
		{
			// HTMLDocumentNode
			m_Node = CastOutput(m_ParserOutput)->document;
			m_Document = this;

			return !AnyErrorOfType(CastOutput(m_ParserOutput), GUMBO_ERR_PARSER);
		}
		return false;
	}
	void HTMLDocument::DoUnload()
	{
		if (m_ParserOutput)
		{
			gumbo_destroy_output(m_ParserOptions.get(), CastOutput(m_ParserOutput));
			m_ParserOutput = nullptr;
		}
		m_Buffer.clear();

		// HTMLDocumentNode
		m_Node = nullptr;
		m_Document = nullptr;
	}

	// HTMLDocument
	HTMLDocument::HTMLDocument()
	{
		m_ParserOptions = std::make_unique<ImplOptions>(kGumboDefaultOptions, *this);
	}
	HTMLDocument::~HTMLDocument()
	{
		DoUnload();
	}

	// IXDocument
	bool HTMLDocument::IsNull() const
	{
		if (!m_Buffer.empty() && m_ParserOutput)
		{
			return AnyErrorOfType(CastOutput(m_ParserOutput), GUMBO_ERR_PARSER);
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
		return DoLoad();
	}
	bool HTMLDocument::SaveDocument(IOutputStream& stream) const
	{
		return SerializeSubtree(stream);
	}

	// HTMLDocument
	bool HTMLDocument::LoadDocument(const String& html)
	{
		DoUnload();

		m_Buffer = html.ToUTF8();
		return DoLoad();
	}
	String HTMLDocument::SaveDocument() const
	{
		return SerializeSubtree();
	}

	HTMLDocument& HTMLDocument::operator=(HTMLDocument&& other) noexcept
	{
		// HTMLDocumentNode
		m_Node = std::exchange(other.m_Node, nullptr);
		m_Document = std::exchange(other.m_Document, nullptr);

		// HTMLDocument
		m_Buffer = std::move(other.m_Buffer);
		m_ParserOptions = std::move(other.m_ParserOptions);
		m_ParserOutput = std::exchange(other.m_ParserOutput, nullptr);

		return *this;
	}
}
