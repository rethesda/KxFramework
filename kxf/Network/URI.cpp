#include "kxf-pch.h"
#include "URI.h"
#include "kxf/Core/ResourceID.h"
#include "kxf/FileSystem/FSPath.h"

#include <wx/uri.h>
#include <uriparser/Uri.h>
#include "kxf/Win32/UndefMacros.h"

namespace
{
	UriTextRangeW ToTextRange(const kxf::String& value) noexcept
	{
		UriTextRangeW range = {nullptr, nullptr};

		if (!value.IsEmpty())
		{
			range.first = value.wc_str();
			range.afterLast = value.wc_str() + value.length();
		}
		return range;
	}
	kxf::StringView FromTextRange(const UriTextRangeW& range) noexcept
	{
		return kxf::StringView(range.first, range.afterLast - range.first);
	}
	bool IsTextRangeEmpty(const UriTextRangeW& range) noexcept
	{
		return FromTextRange(range).empty();
	}

	constexpr UriBreakConversion MapLineBreakFormat(kxf::LineBreakFormat format) noexcept
	{
		using kxf::LineBreakFormat;

		switch (format)
		{
			case LineBreakFormat::LF:
			{
				return UriBreakConversion::URI_BR_TO_LF;
			}
			case LineBreakFormat::CRLF:
			{
				return UriBreakConversion::URI_BR_TO_CRLF;
			}
			case LineBreakFormat::CR:
			{
				return UriBreakConversion::URI_BR_TO_CR;
			}
		};
		return UriBreakConversion::URI_BR_DONT_TOUCH;
	}
	constexpr kxf::NetworkHostType MapHostType(wxURIHostType hostType) noexcept
	{
		using kxf::NetworkHostType;

		switch (hostType)
		{
			case wxURIHostType::wxURI_REGNAME:
			{
				return NetworkHostType::RegisteredName;
			}
			case wxURIHostType::wxURI_IPV4ADDRESS:
			{
				return NetworkHostType::IPv4;
			}
			case wxURIHostType::wxURI_IPV6ADDRESS:
			{
				return NetworkHostType::IPv6;
			}
			case wxURIHostType::wxURI_IPVFUTURE:
			{
				return NetworkHostType::IPvFuture;
			}
		};
		return NetworkHostType::None;
	}
}

namespace kxf
{
	class URIImpl final
	{
		private:
			UriUriW m_URIData = {};

		private:
			void ZeroBuffer() noexcept
			{
				std::memset(&m_URIData, 0, sizeof(m_URIData));
			}
			void Destroy() noexcept
			{
				if (m_URIData.owner)
				{
					::uriFreeUriMembersW(&m_URIData);
					m_URIData.owner = URI_FALSE;
				}
				ZeroBuffer();
			}

		public:
			URIImpl() noexcept
			{
				ZeroBuffer();
			}
			URIImpl(const URIImpl&) = delete;
			URIImpl(URIImpl&& other) noexcept
			{
				std::memcpy(&m_URIData, &other.m_URIData, sizeof(m_URIData));
				other.ZeroBuffer();
			}
			~URIImpl() noexcept
			{
				Destroy();
			}

		public:
			const UriUriW* Get() const noexcept
			{
				return &m_URIData;
			}
			UriUriW* Get() noexcept
			{
				return &m_URIData;
			}

			bool Create(const String& uri, bool makeOwner) noexcept
			{
				Destroy();

				if (!uri.IsEmpty())
				{
					auto result = ::uriParseSingleUriW(&m_URIData, uri.wc_str(), nullptr);
					if (result == URI_SUCCESS)
					{
						if (makeOwner)
						{
							result = ::uriMakeOwnerW(&m_URIData);
							return result == URI_SUCCESS || result == URI_TRUE;
						}
						return true;
					}
				}
				return false;
			}
			String BuildURI() const
			{
				int requiredSize = 0;
				if (::uriToStringCharsRequiredW(&m_URIData, &requiredSize) == URI_SUCCESS)
				{
					std::wstring buffer(requiredSize, 0);

					int written = 0;
					if (::uriToStringW(buffer.data(), &m_URIData, requiredSize + 1, &written) == URI_SUCCESS)
					{
						return buffer;
					}
				}
				return {};
			}

			size_t GetHash() const noexcept
			{
				auto Hash = [](const auto& value)
				{
					return std::hash<StringView>()(FromTextRange(value));
				};

				size_t hash = Hash(m_URIData.scheme);
				hash ^= Hash(m_URIData.userInfo);
				hash ^= Hash(m_URIData.hostText);
				hash ^= Hash(m_URIData.portText);

				auto pathHead = m_URIData.pathHead;
				while (pathHead)
				{
					hash ^= Hash(pathHead->text);
					pathHead = pathHead->next;
				}

				hash ^= Hash(m_URIData.query);
				hash ^= Hash(m_URIData.fragment);

				return hash;
			}
			bool IsSameAs(const URIImpl& other) const noexcept
			{
				return this == &other || ::uriEqualsUriW(&m_URIData, &other.m_URIData);
			}

			bool Resolve(const URIImpl& base, FlagSet<URIFlag> flags) noexcept
			{
				UriUriW result;
				if (::uriAddBaseUriW(&result, &m_URIData, &base.m_URIData) == URI_SUCCESS)
				{
					Destroy();
					m_URIData = result;

					return true;
				}
				return false;
			}
			bool MakeReference(const URIImpl& base, FlagSet<URIFlag> flags) noexcept
			{
				UriUriW result;
				if (::uriRemoveBaseUriW(&result, &m_URIData, &base.m_URIData, flags.Contains(URIFlag::DomainRootRelative)) == URI_SUCCESS)
				{
					Destroy();
					m_URIData = result;

					return true;
				}
				return false;
			}
			bool Normalize()
			{
				unsigned int requiresNormalization = 0;
				if (::uriNormalizeSyntaxMaskRequiredExW(&m_URIData, &requiresNormalization) == URI_SUCCESS)
				{
					return ::uriNormalizeSyntaxExW(&m_URIData, requiresNormalization) == URI_SUCCESS;
				}
				return false;
			}

		public:
			bool HasScheme() const noexcept
			{
				return !IsTextRangeEmpty(m_URIData.scheme);
			}
			String GetScheme() const
			{
				return FromTextRange(m_URIData.scheme);
			}

			NetworkHostType GetHostType() const noexcept
			{
				if (m_URIData.hostData.ip4)
				{
					return NetworkHostType::IPv4;
				}
				else if (m_URIData.hostData.ip6)
				{
					return NetworkHostType::IPv6;
				}
				else if (m_URIData.hostData.ipFuture.first && m_URIData.hostData.ipFuture.afterLast)
				{
					return NetworkHostType::IPvFuture;
				}
				else if (m_URIData.hostText.first && m_URIData.hostText.afterLast)
				{
					return NetworkHostType::RegisteredName;
				}
				return NetworkHostType::None;
			}
			bool HasServer() const noexcept
			{
				return !IsTextRangeEmpty(m_URIData.hostText);
			}
			String GetServer() const
			{
				return FromTextRange(m_URIData.hostText);
			}

			bool HasPort() const noexcept
			{
				return !IsTextRangeEmpty(m_URIData.portText);
			}
			String GetPort() const
			{
				return FromTextRange(m_URIData.portText);
			}

			bool HasPath() const
			{
				return m_URIData.pathHead && !IsTextRangeEmpty(m_URIData.pathHead->text);
			}
			String GetPath() const
			{
				String result;

				auto pathHead = m_URIData.pathHead;
				while (pathHead)
				{
					if (!result.IsEmpty())
					{
						result += '/';
					}
					result += FromTextRange(pathHead->text);

					pathHead = pathHead->next;
				}
				return result;
			}

			bool HasQuery() const noexcept
			{
				return !IsTextRangeEmpty(m_URIData.query);
			}
			String GetQuery() const
			{
				return FromTextRange(m_URIData.query);
			}

			bool HasFragment() const noexcept
			{
				return !IsTextRangeEmpty(m_URIData.fragment);
			}
			String GetFragment() const
			{
				return FromTextRange(m_URIData.fragment);
			}

			bool HasUserInfo() const noexcept
			{
				return !IsTextRangeEmpty(m_URIData.userInfo);
			}
			String GetUserInfo() const
			{
				return FromTextRange(m_URIData.userInfo);
			}
			String GetUser() const
			{
				auto userInfo = FromTextRange(m_URIData.userInfo);

				size_t pos = userInfo.find(':');
				if (pos != StringView::npos)
				{
					return userInfo.substr(0, pos - 1);
				}
				return {};
			}
			String GetPassword() const
			{
				auto userInfo = FromTextRange(m_URIData.userInfo);

				size_t pos = userInfo.find(':');
				if (pos != StringView::npos)
				{
					return userInfo.substr(pos + 1);
				}
				return {};
			}

		public:
			URIImpl& operator=(const URIImpl&) = delete;
			URIImpl& operator=(URIImpl&& other) noexcept
			{
				if (this != &other)
				{
					Destroy();

					std::memcpy(&m_URIData, &other.m_URIData, sizeof(m_URIData));
					other.ZeroBuffer();
				}
				return *this;
			}
	};
}

namespace kxf
{
	String URI::Escape(const String& source, FlagSet<URIFlag> flags)
	{
		auto sourceRange = ToTextRange(source);

		std::wstring buffer;
		buffer.resize(source.length() * (flags.Contains(URIFlag::NormalizeBreaks) ? 6 : 3));

		if (const wchar_t* outputTerminator = ::uriEscapeExW(sourceRange.first, sourceRange.afterLast, buffer.data(), flags.Contains(URIFlag::SpacePlus), flags.Contains(URIFlag::NormalizeBreaks)))
		{
			buffer.resize(outputTerminator - buffer.data());
			return buffer;
		}
		return {};
	}
	String URI::Unescape(const String& source, LineBreakFormat lineBreakFormat, FlagSet<URIFlag> flags)
	{
		auto buffer = source.ToUTF8();
		if (auto outputTerminator = ::uriUnescapeInPlaceExA(buffer.data(), flags.Contains(URIFlag::SpacePlus), MapLineBreakFormat(lineBreakFormat)))
		{
			buffer.resize(outputTerminator - buffer.data());
			return String::FromUTF8(buffer);
		}
		return {};
	}

	bool URI::IsNull() const noexcept
	{
		return !m_URI.IsConstructed();
	}
	void URI::Clear() noexcept
	{
		m_URI.Destroy();
	}

	bool URI::Create(const String& uri)
	{
		if (m_URI.ConstructAligned() && m_URI->Create(uri, true))
		{
			return true;
		}
		else
		{
			Clear();
			return false;
		}
	}
	bool URI::Create(const FSPath& path)
	{
		if (path)
		{
			String fullPath = path.GetFullPath();

			std::wstring buffer(8 + 3 * fullPath.length() + 1, 0);
			if (::uriWindowsFilenameToUriStringW(fullPath.wc_str(), buffer.data()) == URI_SUCCESS)
			{
				return Create(String(std::move(buffer)));
			}
		}
		return false;
	}
	bool URI::Create(const wxURI& uri)
	{
		return Create(uri.BuildURI());
	}

	bool URI::IsReference() const noexcept
	{
		return !HasScheme() || !HasServer();
	}
	URI& URI::Resolve(const URI& base, FlagSet<URIFlag> flags)
	{
		if (m_URI && base)
		{
			m_URI->Resolve(*base.m_URI, flags);
		}
		return *this;
	}
	URI& URI::MakeReference(const URI& base, FlagSet<URIFlag> flags)
	{
		if (m_URI && base)
		{
			m_URI->MakeReference(*base.m_URI, flags);
		}
		return *this;
	}
	URI& URI::Normalize() noexcept
	{
		if (m_URI)
		{
			m_URI->Normalize();
		}
		return *this;
	}

	String URI::BuildURI() const
	{
		return m_URI ? m_URI->BuildURI() : NullString;
	}
	String URI::BuildUnescapedURI(LineBreakFormat lineBreakFormat, FlagSet<URIFlag> flags) const
	{
		return m_URI ? Unescape(m_URI->BuildURI(), lineBreakFormat, flags) : NullString;
	}
	FSPath URI::ToFSPath() const
	{
		if (m_URI && m_URI->GetScheme() == kxfS("file"))
		{
			auto uri = m_URI->BuildURI();

			std::wstring buffer(uri.length() + 1, 0);
			if (::uriUriStringToWindowsFilenameW(uri.wc_str(), buffer.data()) == URI_SUCCESS)
			{
				return String(std::move(buffer));
			}
		}
		return {};
	}

	bool URI::HasScheme() const noexcept
	{
		return m_URI && m_URI->HasScheme();
	}
	String URI::GetScheme() const
	{
		return m_URI ? m_URI->GetScheme() : NullString;
	}

	NetworkHostType URI::GetHostType() const noexcept
	{
		return m_URI ? m_URI->GetHostType() : NetworkHostType::None;
	}
	bool URI::HasServer() const noexcept
	{
		return m_URI && m_URI->HasServer();
	}
	String URI::GetServer() const
	{
		return m_URI ? m_URI->GetServer() : NullString;
	}

	bool URI::HasPort() const noexcept
	{
		return m_URI && m_URI->HasPort();
	}
	std::optional<uint16_t> URI::GetPortInt() const
	{
		return GetPort().ParseInteger<uint16_t>();
	}
	String URI::GetPort() const
	{
		return m_URI ? m_URI->GetPort() : NullString;
	}

	bool URI::HasPath() const noexcept
	{
		return m_URI && m_URI->HasPath();
	}
	String URI::GetPath() const
	{
		return m_URI ? m_URI->GetPath() : NullString;
	}

	bool URI::HasQuery() const noexcept
	{
		return m_URI && m_URI->HasQuery();
	}
	String URI::GetQuery() const
	{
		return m_URI ? m_URI->GetQuery() : NullString;
	}

	bool URI::HasFragment() const noexcept
	{
		return m_URI && m_URI->HasFragment();
	}
	String URI::GetFragment() const
	{
		return m_URI ? m_URI->GetFragment() : NullString;
	}

	bool URI::HasUserInfo() const noexcept
	{
		return m_URI && m_URI->HasUserInfo();
	}
	String URI::GetUserInfo() const
	{
		return m_URI ? m_URI->GetUserInfo() : NullString;
	}
	String URI::GetUser() const
	{
		return m_URI ? m_URI->GetUser() : NullString;
	}
	String URI::GetPassword() const
	{
		return m_URI ? m_URI->GetPassword() : NullString;
	}

	URI& URI::operator=(const URI& other)
	{
		Create(other.BuildURI());
		return *this;
	}
	URI& URI::operator=(URI&& other) noexcept
	{
		if (this != &other)
		{
			Clear();

			if (other.m_URI)
			{
				m_URI.ConstructAligned(std::move(*other.m_URI));
				other.Clear();
			}
		}
		return *this;
	}

	bool URI::operator==(const URI& other) const noexcept
	{
		return this == &other || (!m_URI && !other.m_URI) || (m_URI && other.m_URI && m_URI->IsSameAs(*other.m_URI));
	}
	bool URI::operator==(const wxURI& other) const
	{
		return *this == URI(other);
	}

	URI::operator wxURI() const
	{
		return wxURI(BuildURI());
	}
}

namespace std
{
	size_t std::hash<kxf::URI>::operator()(const kxf::URI& uri) const noexcept
	{
		return uri.m_URI ? uri.m_URI->GetHash() : 0;
	}
}
