#include "kxf-pch.h"
#include "CURLWebResponse.h"
#include "CURLWebRequest.h"
#include "LibCURLUtility.h"
#include "LibCURL.h"
#include "kxf/IO/IStream.h"

#include "kxf/System/HandlePtr.h"
#include "kxf/Utility/Container.h"

namespace kxf
{
	CURL::Private::RequestHandle& CURLWebResponse::GetRequestHandle() const noexcept
	{
		return m_Request.m_Handle;
	}

	URI CURLWebResponse::GetURI() const
	{
		if (m_Request.m_FollowLocation == WebRequestOption2::Enabled)
		{
			return GetRequestHandle().GetOptionString(CURLINFO_EFFECTIVE_URL).value_or(NullString);
		}
		else
		{
			return GetRequestHandle().GetOptionString(CURLINFO_REDIRECT_URL).value_or(NullString);
		}
	}
	String CURLWebResponse::GetMethod() const
	{
		return GetRequestHandle().GetOptionString(CURLINFO_EFFECTIVE_METHOD).value_or(NullString);
	}
	String CURLWebResponse::GetPrimaryIP() const
	{
		return GetRequestHandle().GetOptionString(CURLINFO_PRIMARY_IP).value_or(NullString);
	}
	std::optional<uint16_t> CURLWebResponse::GetPrimaryPort() const
	{
		if (auto port = GetRequestHandle().GetOptionUInt32(CURLINFO_PRIMARY_PORT))
		{
			return static_cast<uint16_t>(*port);
		}
		return {};
	}
	WebRequestProtocol CURLWebResponse::GetProtocol() const
	{
		if (auto scheme = GetRequestHandle().GetOptionString(CURLINFO_SCHEME); scheme)
		{
			return CURL::Private::MapProtocol(*scheme);
		}
		return WebRequestProtocol::None;
	}
	WebRequestHTTPVersion CURLWebResponse::GetHTTPVersion() const
	{
		switch (GetRequestHandle().GetOptionUInt32(CURLINFO_HTTP_VERSION).value_or(0))
		{
			case CURL_HTTP_VERSION_1_0:
			{
				return WebRequestHTTPVersion::Version1_0;
			}
			case CURL_HTTP_VERSION_1_1:
			{
				return WebRequestHTTPVersion::Version1_1;
			}
			case CURL_HTTP_VERSION_2_0:
			{
				return WebRequestHTTPVersion::Version2;
			}
			case CURL_HTTP_VERSION_3:
			{
				return WebRequestHTTPVersion::Version3;
			}
		};
		return WebRequestHTTPVersion::None;
	}
	DataSize CURLWebResponse::GetContentLength() const
	{
		if (auto value = GetRequestHandle().GetOptionUInt64(CURLINFO_CONTENT_LENGTH_DOWNLOAD_T))
		{
			return DataSize::FromBytes(*value);
		}
		return {};
	}
	String CURLWebResponse::GetContentType() const
	{
		return GetRequestHandle().GetOptionString(CURLINFO_CONTENT_TYPE).value_or(NullString);
	}
	std::optional<int> CURLWebResponse::GetResponseCode() const
	{
		return GetRequestHandle().GetOptionInt32(CURLINFO_RESPONSE_CODE);
	}

	String CURLWebResponse::GetHeader(const String& name) const
	{
		auto it = Utility::Container::FindIf(m_Request.m_ResponseHeaders, [&](const WebRequestHeader& header)
		{
			return header.GetName() == name;
		});
		if (it != m_Request.m_ResponseHeaders.end())
		{
			return it->GetValue();
		}
		return {};
	}
	CallbackResult<void> CURLWebResponse::EnumHeaders(CallbackFunction<WebRequestHeader> func) const
	{
		for (const auto& item: m_Request.m_ResponseHeaders)
		{
			if (func.Invoke(item).ShouldTerminate())
			{
				break;
			}
		}
		return func.Finalize();
	}
	CallbackResult<void> CURLWebResponse::EnumCookies(CallbackFunction<String> func) const
	{
		if (auto cookesList = static_cast<curl_slist*>(GetRequestHandle().GetOptionPtr(CURLINFO_COOKIELIST).value_or(nullptr)))
		{
			auto handle = make_handle_ptr<::curl_slist_free_all>(cookesList);
			for (auto item = cookesList; item; item = item->next)
			{
				if (func.Invoke(String::FromUTF8(item->data)).ShouldTerminate())
				{
					break;
				}
			}
			return func.Finalize();
		}
		return {};
	}

	std::shared_ptr<IInputStream> CURLWebResponse::GetStream() const
	{
		if (m_Request.m_ReceiveStream)
		{
			switch (m_Request.m_ReceiveStorage)
			{
				case WebRequestStorage::Memory:
				case WebRequestStorage::Stream:
				case WebRequestStorage::FileSystem:
				{
					if (auto readableStream = m_Request.m_ReceiveStream->QueryInterface<IReadableOutputStream>())
					{
						return readableStream->CreateInputStream();
					}
					
					break;
				}
			};
		}
		return nullptr;
	}
}
